/*
 * Copyright 2012 Canonical Ltd.
 * Copyright 2022 Robert Tari
 *
 * Authors: Lars Uebernickel <lars.uebernickel@canonical.com>
 *          Robert Tari <robert@tari.in>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cups/cups.h>
#include "dbus-names.h"
#include <glib/gi18n-lib.h>
#include <gio/gio.h>
#include "indicator-printers-service.h"
#include "cups-notifier.h"
#include "indicator-printer-state-notifier.h"
#include "spawn-printer-settings.h"

#define NOTIFY_LEASE_DURATION (24 * 60 * 60)

static guint m_nSignal = 0;

enum
{
    SECTION_HEADER = (1<<0),
    SECTION_PRINTERS = (1<<2)
};

enum
{
    PROFILE_PHONE,
    PROFILE_DESKTOP,
    N_PROFILES
};

static const char *const lMenuNames[N_PROFILES] =
{
    "phone",
    "desktop"
};

struct ProfileMenuInfo
{
    GMenu *pMenu;
    GMenu *pSubmenu;
    guint nExportId;
};

struct _IndicatorPrintersServicePrivate
{
    GCancellable *pCancellable;
    IndicatorPrinterStateNotifier *pStateNotifier;
    CupsNotifier *pCupsNotifier;
    guint nOwnId;
    guint nActionsId;
    GDBusConnection *pConnection;
    gboolean bMenusBuilt;
    int nSubscriptionId;
    struct ProfileMenuInfo lMenus[N_PROFILES];
    GSimpleActionGroup *pActionGroup;
    GSimpleAction *pHeaderAction;
    GSimpleAction *pPrinterAction;
    GMenu *pPrintersSection;
    gboolean bVisible;
};

typedef IndicatorPrintersServicePrivate priv_t;

G_DEFINE_TYPE_WITH_PRIVATE (IndicatorPrintersService, indicator_printers_service, G_TYPE_OBJECT)

static void rebuildNow (IndicatorPrintersService *self, guint nSections);

static void unexport (IndicatorPrintersService *self)
{
    if (self->pPrivate->nSubscriptionId > 0)
    {
        ipp_t *pRequest = ippNewRequest (IPP_CANCEL_SUBSCRIPTION);
        ippAddString (pRequest, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, "/");
        ippAddInteger (pRequest, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "notify-subscription-id", self->pPrivate->nSubscriptionId);
        ipp_t *pResponse = cupsDoRequest (CUPS_HTTP_DEFAULT, pRequest, "/");

        if (!pResponse || cupsLastError () != IPP_OK)
        {
            g_warning ("Error subscribing to CUPS notifications: %s\n", cupsLastErrorString ());

            return;
        }

        ippDelete (pResponse);
    }

    // Unexport the menus
    for (int i = 0; i < N_PROFILES; ++i)
    {
        guint *pId = &self->pPrivate->lMenus[i].nExportId;

        if (*pId)
        {
            g_dbus_connection_unexport_menu_model (self->pPrivate->pConnection, *pId);
            *pId = 0;
        }
    }

    // Unexport the actions
    if (self->pPrivate->nActionsId)
    {
        g_dbus_connection_unexport_action_group (self->pPrivate->pConnection, self->pPrivate->nActionsId);
        self->pPrivate->nActionsId = 0;
    }
}

static void onPrinterStateChanged (CupsNotifier *pNotifier, const gchar *sText, const gchar *sPrinterUri, const gchar *sPrinterName, guint nPrinterState, const gchar *sPrinterStateReasons, gboolean bPrinterIsAcceptingJobs, IndicatorPrintersService *self)
{
    rebuildNow(self, SECTION_PRINTERS | SECTION_HEADER);
}

static void onJobChanged (CupsNotifier *pNotifier, const gchar *sText, const gchar *sPrinterUri, const gchar *sPrinterName, guint nPrinterState, const gchar *sPrinterStateReasons, gboolean bPrinterIsAcceptingJobs, guint nJobId, guint nJobState, const gchar *sJobStateReasons, const gchar *sJobName, guint nJobImpressionsCompleted, IndicatorPrintersService *self)
{
    rebuildNow(self, SECTION_PRINTERS | SECTION_HEADER);
}

static void onDispose (GObject *pObject)
{
    IndicatorPrintersService *self = INDICATOR_PRINTERS_SERVICE (pObject);

    unexport (self);

    if (self->pPrivate->pCancellable != NULL)
    {
        g_cancellable_cancel (self->pPrivate->pCancellable);
        g_clear_object (&self->pPrivate->pCancellable);
    }

    if (self->pPrivate->pCupsNotifier)
    {
        g_object_disconnect (self->pPrivate->pCupsNotifier, "any-signal", onJobChanged, self, "any-signal", onPrinterStateChanged, self, NULL);
        g_clear_object (&self->pPrivate->pCupsNotifier);
    }

    g_object_unref (self->pPrivate->pStateNotifier);
    g_clear_object (&self->pPrivate->pPrinterAction);
    g_clear_object (&self->pPrivate->pHeaderAction);
    g_clear_object (&self->pPrivate->pActionGroup);
    g_clear_object (&self->pPrivate->pConnection);

    G_OBJECT_CLASS (indicator_printers_service_parent_class)->dispose (pObject);
}

static void indicator_printers_service_class_init (IndicatorPrintersServiceClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->dispose = onDispose;
    m_nSignal = g_signal_new ("name-lost", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (IndicatorPrintersServiceClass, pNameLost), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static int createSubscription ()
{
    int nId = 0;

    ipp_t *pRequest = ippNewRequest (IPP_CREATE_PRINTER_SUBSCRIPTION);
    ippAddString (pRequest, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, "/");
    ippAddString (pRequest, IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD, "notify-events", NULL, "all");
    ippAddString (pRequest, IPP_TAG_SUBSCRIPTION, IPP_TAG_URI, "notify-recipient-uri", NULL, "dbus://");
    ippAddInteger (pRequest, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER, "notify-lease-duration", NOTIFY_LEASE_DURATION);
    ipp_t *pResponse = cupsDoRequest (CUPS_HTTP_DEFAULT, pRequest, "/");

    if (!pResponse || cupsLastError () != IPP_OK)
    {
        g_warning ("Error subscribing to CUPS notifications: %s\n", cupsLastErrorString ());

        return 0;
    }

    ipp_attribute_t *pAttribute = ippFindAttribute (pResponse, "notify-subscription-id", IPP_TAG_INTEGER);

    if (pAttribute)
    {
        nId = ippGetInteger (pAttribute, 0);
    }
    else
    {
        g_warning ("ipp-create-printer-subscription response doesn't contain subscription id.\n");
    }

    ippDelete (pResponse);

    return nId;
}

static gboolean renewSubscriptionTimeout (gpointer pData)
{
    int *nSubscriptionId = pData;
    gboolean bRenewed = TRUE;
    ipp_t *pRequest = ippNewRequest (IPP_RENEW_SUBSCRIPTION);
    ippAddInteger (pRequest, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "notify-subscription-id", *nSubscriptionId);
    ippAddString (pRequest, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, "/");
    ippAddString (pRequest, IPP_TAG_SUBSCRIPTION, IPP_TAG_URI, "notify-recipient-uri", NULL, "dbus://");
    ippAddInteger (pRequest, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER, "notify-lease-duration", NOTIFY_LEASE_DURATION);
    ipp_t *pResponse = cupsDoRequest (CUPS_HTTP_DEFAULT, pRequest, "/");

    if (!pResponse || cupsLastError () != IPP_OK)
    {
        g_warning ("Error renewing CUPS subscription %d: %s\n", *nSubscriptionId, cupsLastErrorString ());
        bRenewed = FALSE;
    }
    else
    {
        ippDelete (pResponse);
    }

    if (*nSubscriptionId <= 0 || !bRenewed)
    {
        *nSubscriptionId = createSubscription ();
    }

    return TRUE;
}

static GVariant *createHeaderState (IndicatorPrintersService *self)
{
    GVariantBuilder b;

    g_variant_builder_init (&b, G_VARIANT_TYPE ("a{sv}"));
    g_variant_builder_add (&b, "{sv}", "title", g_variant_new_string (_("Printers")));
    g_variant_builder_add (&b, "{sv}", "tooltip", g_variant_new_string (_("Show print jobs and queues")));
    g_variant_builder_add (&b, "{sv}", "visible", g_variant_new_boolean (TRUE));

    if (self->pPrivate->bVisible)
    {
        GIcon *pIcon = g_themed_icon_new_with_default_fallbacks ("printer-symbolic");
        g_variant_builder_add (&b, "{sv}", "accessible-desc", g_variant_new_string (_("Printers")));

        if (pIcon)
        {
            GVariant *pSerialized = g_icon_serialize (pIcon);

            if (pSerialized != NULL)
            {
                g_variant_builder_add (&b, "{sv}", "icon", pSerialized);
                g_variant_unref (pSerialized);
            }

            g_object_unref (pIcon);
        }
    }

    return g_variant_builder_end (&b);
}

static void onPrinterItemActivated (GSimpleAction *pAction, GVariant *pVariant, gpointer pData)
{
    const gchar *sPrinter = g_variant_get_string(pVariant, NULL);
    spawn_printer_settings_with_args ("--show-jobs %s", sPrinter);
}

static void initActions (IndicatorPrintersService *self)
{
    self->pPrivate->pActionGroup = g_simple_action_group_new ();

    GSimpleAction *pAction = g_simple_action_new_stateful ("_header", NULL, createHeaderState (self));
    g_action_map_add_action (G_ACTION_MAP (self->pPrivate->pActionGroup), G_ACTION (pAction));
    self->pPrivate->pHeaderAction = pAction;

    pAction = g_simple_action_new("printer", G_VARIANT_TYPE_STRING);
    g_action_map_add_action (G_ACTION_MAP (self->pPrivate->pActionGroup), G_ACTION (pAction));
    self->pPrivate->pPrinterAction = pAction;
    g_signal_connect(pAction, "activate", G_CALLBACK(onPrinterItemActivated), self);

    rebuildNow (self, SECTION_HEADER);
}

static GMenuModel *createPrintersSection (IndicatorPrintersService *self)
{
    self->pPrivate->pPrintersSection = g_menu_new ();
    self->pPrivate->bVisible = FALSE;
    cups_dest_t *lDests;
    gint nDests = cupsGetDests (&lDests);

    for (gint i = 0; i < nDests; i++)
    {
        const gchar *sOption = cupsGetOption ("printer-state", lDests[i].num_options, lDests[i].options);

        if (sOption != NULL)
        {
            cups_job_t *lJobs;
            gint nJobs = cupsGetJobs (&lJobs, lDests[i].name, 1, CUPS_WHICHJOBS_ACTIVE);
            cupsFreeJobs (nJobs, lJobs);

            if (nJobs < 0)
            {
                g_warning ("printer '%s' does not exist\n", lDests[i].name);
            }
            else if (nJobs != 0)
            {
                GMenuItem *pItem = g_menu_item_new (lDests[i].name, NULL);
                g_menu_item_set_attribute (pItem, "x-ayatana-type", "s", "org.ayatana.indicator.basic");
                g_menu_item_set_action_and_target_value(pItem, "indicator.printer", g_variant_new_string (lDests[i].name));
                GIcon *pIcon = g_themed_icon_new_with_default_fallbacks ("printer");
                GVariant *pSerialized = g_icon_serialize(pIcon);

                if (pSerialized != NULL)
                {
                    g_menu_item_set_attribute_value(pItem, G_MENU_ATTRIBUTE_ICON, pSerialized);
                    g_variant_unref(pSerialized);
                }

                g_object_unref(pIcon);

                gint nState = atoi (sOption);

                switch (nState)
                {
                    case IPP_PRINTER_STOPPED:
                    {
                        g_menu_item_set_attribute (pItem, "x-ayatana-secondary-text", "s", _("Paused"));

                        break;
                    }
                    case IPP_PRINTER_PROCESSING:
                    {
                        g_menu_item_set_attribute (pItem, "x-ayatana-secondary-count", "i", nJobs);

                        break;
                    }
                }

                g_menu_append_item(self->pPrivate->pPrintersSection, pItem);
                g_object_unref(pItem);
                self->pPrivate->bVisible = TRUE;
            }
        }
    }

    cupsFreeDests (nDests, lDests);

    return G_MENU_MODEL (self->pPrivate->pPrintersSection);
}

static void createMenu (IndicatorPrintersService *self, int nProfile)
{
    g_assert (0 <= nProfile && nProfile < N_PROFILES);
    g_assert (self->pPrivate->lMenus[nProfile].pMenu == NULL);

    GMenuModel *lSections[16];
    guint nSection = 0;

    // Build the sections
    switch (nProfile)
    {
        case PROFILE_PHONE:
        case PROFILE_DESKTOP:
        {
            lSections[nSection++] = createPrintersSection (self);

            break;
        }

        break;
    }

    // Add sections to the submenu
    GMenu *pSubmenu = g_menu_new ();

    for (guint i = 0; i < nSection; ++i)
    {
        g_menu_append_section (pSubmenu, NULL, lSections[i]);
        g_object_unref (lSections[i]);
    }

    // Add submenu to the header
    GMenuItem *pHeader = g_menu_item_new (NULL, "indicator._header");
    g_menu_item_set_attribute (pHeader, "x-ayatana-type", "s", "org.ayatana.indicator.root");
    g_menu_item_set_submenu (pHeader, G_MENU_MODEL (pSubmenu));
    g_object_unref (pSubmenu);

    // Add header to the menu
    GMenu *pMenu = g_menu_new ();
    g_menu_append_item (pMenu, pHeader);
    g_object_unref (pHeader);

    self->pPrivate->lMenus[nProfile].pMenu = pMenu;
    self->pPrivate->lMenus[nProfile].pSubmenu = pSubmenu;
}

static void onBusAcquired (GDBusConnection *pConnection, const gchar *sName, gpointer pSelf)
{
    g_debug ("bus acquired: %s", sName);

    IndicatorPrintersService *self = INDICATOR_PRINTERS_SERVICE (pSelf);
    guint nId;
    GError *pError = NULL;
    GString *pPath = g_string_new (NULL);
    self->pPrivate->pConnection = (GDBusConnection*)g_object_ref (G_OBJECT (pConnection));

    // Export the actions
    if ((nId = g_dbus_connection_export_action_group (pConnection, INDICATOR_PRINTERS_DBUS_OBJECT_PATH, G_ACTION_GROUP (self->pPrivate->pActionGroup), &pError)))
    {
        self->pPrivate->nActionsId = nId;
    }
    else
    {
        g_warning ("cannot export action group: %s", pError->message);
        g_clear_error (&pError);
    }

    // Export the menus
    for (gint nProfile = 0; nProfile < N_PROFILES; ++nProfile)
    {
        struct ProfileMenuInfo *pInfo = &self->pPrivate->lMenus[nProfile];

        g_string_printf (pPath, "%s/%s", INDICATOR_PRINTERS_DBUS_OBJECT_PATH, lMenuNames[nProfile]);

        if ((nId = g_dbus_connection_export_menu_model (pConnection, pPath->str, G_MENU_MODEL (pInfo->pMenu), &pError)))
        {
            pInfo->nExportId = nId;
        }
        else
        {
            g_warning ("cannot export %s menu: %s", pPath->str, pError->message);
            g_clear_error (&pError);
        }
    }

    g_string_free (pPath, TRUE);
}

static void onNameLost (GDBusConnection *pConnection, const gchar *sName, gpointer pSelf)
{
    IndicatorPrintersService *self = INDICATOR_PRINTERS_SERVICE (pSelf);

    g_debug ("%s %s name lost %s", G_STRLOC, G_STRFUNC, sName);

    unexport (self);
}

static void indicator_printers_service_init (IndicatorPrintersService *self)
{
    self->pPrivate = indicator_printers_service_get_instance_private (self);
    self->pPrivate->pCancellable = g_cancellable_new ();

    GError *pError = NULL;
    self->pPrivate->nSubscriptionId = createSubscription ();
    g_timeout_add_seconds (NOTIFY_LEASE_DURATION - 60, renewSubscriptionTimeout, &self->pPrivate->nSubscriptionId);
    self->pPrivate->pCupsNotifier = cups_notifier_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM, 0, NULL, CUPS_DBUS_PATH, NULL, &pError);

    if (pError)
    {
        g_error ("Error creating cups notify handler: %s", pError->message);
        g_error_free (pError);
    }

    g_object_connect (self->pPrivate->pCupsNotifier, "signal::job-created", onJobChanged, self, "signal::job-state", onJobChanged, self, "signal::job-completed", onJobChanged, self, "signal::printer-state-changed", onPrinterStateChanged, self, NULL);
    self->pPrivate->pStateNotifier = g_object_new (INDICATOR_TYPE_PRINTER_STATE_NOTIFIER, "cups-notifier", self->pPrivate->pCupsNotifier, NULL);
    initActions (self);

    for (gint nProfile = 0; nProfile < N_PROFILES; ++nProfile)
    {
        createMenu (self, nProfile);
    }

    self->pPrivate->bMenusBuilt = TRUE;
    self->pPrivate->nOwnId = g_bus_own_name (G_BUS_TYPE_SESSION, INDICATOR_PRINTERS_DBUS_NAME, G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT, onBusAcquired, NULL, onNameLost, self, NULL);
}

IndicatorPrintersService *indicator_printers_service_new ()
{
    GObject *pObject = g_object_new (INDICATOR_TYPE_PRINTERS_SERVICE, NULL);

    return INDICATOR_PRINTERS_SERVICE (pObject);
}

static void rebuildSection (GMenu *pMenu, int nPos, GMenuModel *pSection)
{
    g_menu_remove (pMenu, nPos);
    g_menu_insert_section (pMenu, nPos, NULL, pSection);
    g_object_unref (pSection);
}

static void rebuildNow (IndicatorPrintersService *self, guint nSections)
{
    struct ProfileMenuInfo *pinfo = &self->pPrivate->lMenus[PROFILE_DESKTOP];

    if (nSections & SECTION_HEADER)
    {
        g_simple_action_set_state (self->pPrivate->pHeaderAction, createHeaderState (self));
    }

    if (!self->pPrivate->bMenusBuilt)
    {
        return;
    }

    if (nSections & SECTION_PRINTERS)
    {
        rebuildSection (pinfo->pSubmenu, 0, createPrintersSection (self));
    }
}
