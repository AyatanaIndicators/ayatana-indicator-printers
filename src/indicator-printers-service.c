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

#include <libayatana-indicator/indicator-service.h>
#include <libdbusmenu-glib/dbusmenu-glib.h>
#include <gtk/gtk.h>
#include <cups/cups.h>
#include "dbus-names.h"
#include <glib/gi18n-lib.h>
#include "cups-notifier.h"
#include "indicator-printers-menu.h"
#include "indicator-printer-state-notifier.h"

#define NOTIFY_LEASE_DURATION (24 * 60 * 60)


static int
create_subscription ()
{
    ipp_t *req;
    ipp_t *resp;
    ipp_attribute_t *attr;
    int id = 0;

    req = ippNewRequest (IPP_CREATE_PRINTER_SUBSCRIPTION);
    ippAddString (req, IPP_TAG_OPERATION, IPP_TAG_URI,
                  "printer-uri", NULL, "/");
    ippAddString (req, IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD,
                  "notify-events", NULL, "all");
    ippAddString (req, IPP_TAG_SUBSCRIPTION, IPP_TAG_URI,
                  "notify-recipient-uri", NULL, "dbus://");
    ippAddInteger (req, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER,
                   "notify-lease-duration", NOTIFY_LEASE_DURATION);

    resp = cupsDoRequest (CUPS_HTTP_DEFAULT, req, "/");
    if (!resp || cupsLastError() != IPP_OK) {
        g_warning ("Error subscribing to CUPS notifications: %s\n",
                   cupsLastErrorString ());
        return 0;
    }

    attr = ippFindAttribute (resp, "notify-subscription-id", IPP_TAG_INTEGER);
    if (attr)
        id = ippGetInteger (attr, 0);
    else
        g_warning ("ipp-create-printer-subscription response doesn't contain "
                   "subscription id.\n");

    ippDelete (resp);
    return id;
}


static gboolean
renew_subscription (int id)
{
    ipp_t *req;
    ipp_t *resp;

    req = ippNewRequest (IPP_RENEW_SUBSCRIPTION);
    ippAddInteger (req, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                   "notify-subscription-id", id);
    ippAddString (req, IPP_TAG_OPERATION, IPP_TAG_URI,
                  "printer-uri", NULL, "/");
    ippAddString (req, IPP_TAG_SUBSCRIPTION, IPP_TAG_URI,
                  "notify-recipient-uri", NULL, "dbus://");
    ippAddInteger (req, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER,
                   "notify-lease-duration", NOTIFY_LEASE_DURATION);

    resp = cupsDoRequest (CUPS_HTTP_DEFAULT, req, "/");
    if (!resp || cupsLastError() != IPP_OK) {
        g_warning ("Error renewing CUPS subscription %d: %s\n",
                   id, cupsLastErrorString ());
        return FALSE;
    }

    ippDelete (resp);
    return TRUE;
}


static gboolean
renew_subscription_timeout (gpointer userdata)
{
    int *subscription_id = userdata;

    if (*subscription_id <= 0 || !renew_subscription (*subscription_id))
        *subscription_id = create_subscription ();

    return TRUE;
}


void
cancel_subscription (int id)
{
    ipp_t *req;
    ipp_t *resp;

    if (id <= 0)
        return;

    req = ippNewRequest (IPP_CANCEL_SUBSCRIPTION);
    ippAddString (req, IPP_TAG_OPERATION, IPP_TAG_URI,
                  "printer-uri", NULL, "/");
    ippAddInteger (req, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                   "notify-subscription-id", id);

    resp = cupsDoRequest (CUPS_HTTP_DEFAULT, req, "/");
    if (!resp || cupsLastError() != IPP_OK) {
        g_warning ("Error subscribing to CUPS notifications: %s\n",
                   cupsLastErrorString ());
        return;
    }

    ippDelete (resp);
}

static void
name_lost (GDBusConnection *connection,
           const gchar     *name,
           gpointer         user_data)
{
    int subscription_id = GPOINTER_TO_INT (user_data);

    cancel_subscription (subscription_id);
    gtk_main_quit ();
}

int main (int argc, char *argv[])
{
    /* Init i18n */
    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    DbusmenuServer *menuserver;
    CupsNotifier *cups_notifier;
    IndicatorPrintersMenu *menu;
    IndicatorPrinterStateNotifier *state_notifier;
    GError *error = NULL;
    int subscription_id;

    gtk_init (&argc, &argv);

    subscription_id = create_subscription ();
    g_timeout_add_seconds (NOTIFY_LEASE_DURATION - 60,
                           renew_subscription_timeout,
                           &subscription_id);

    g_bus_own_name (G_BUS_TYPE_SESSION,
                    INDICATOR_PRINTERS_DBUS_NAME,
                    G_BUS_NAME_OWNER_FLAGS_NONE,
                    NULL, NULL, name_lost,
                    GINT_TO_POINTER (subscription_id), NULL);

    cups_notifier = cups_notifier_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                          0,
                                                          NULL,
                                                          CUPS_DBUS_PATH,
                                                          NULL,
                                                          &error);
    if (error) {
        g_warning ("Error creating cups notify handler: %s", error->message);
        g_error_free (error);
        return 1;
    }

    menu = g_object_new (INDICATOR_TYPE_PRINTERS_MENU,
                         "cups-notifier", cups_notifier,
                         NULL);

    menuserver = dbusmenu_server_new (INDICATOR_PRINTERS_DBUS_OBJECT_PATH);
    dbusmenu_server_set_root (menuserver,
                              indicator_printers_menu_get_root (menu));

    state_notifier = g_object_new (INDICATOR_TYPE_PRINTER_STATE_NOTIFIER,
                                   "cups-notifier", cups_notifier,
                                   NULL);

    gtk_main ();

    g_object_unref (menu);
    g_object_unref (menuserver);
    g_object_unref (state_notifier);
    g_object_unref (cups_notifier);
    return 0;
}
