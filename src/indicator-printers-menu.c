
#include "indicator-printers-menu.h"

#include <gio/gio.h>
#include <cups/cups.h>


G_DEFINE_TYPE (IndicatorPrintersMenu, indicator_printers_menu, G_TYPE_OBJECT)


struct _IndicatorPrintersMenuPrivate
{
    DbusmenuMenuitem *root;
    GHashTable *printers;    /* printer name -> dbusmenuitem */
    CupsNotifier *cups_notifier;
};


enum {
    PROP_0,
    PROP_CUPS_NOTIFIER,
    NUM_PROPERTIES
};

GParamSpec *properties[NUM_PROPERTIES];


static void
dispose (GObject *object)
{
    IndicatorPrintersMenu *self = INDICATOR_PRINTERS_MENU (object);

    if (self->priv->printers) {
        g_hash_table_unref (self->priv->printers);
        self->priv->printers = NULL;
    }

    g_clear_object (&self->priv->root);
    g_clear_object (&self->priv->cups_notifier);

    G_OBJECT_CLASS (indicator_printers_menu_parent_class)->dispose (object);
}


void
set_property (GObject        *object,
              guint           property_id,
              const GValue   *value,
              GParamSpec     *pspec)
{
    IndicatorPrintersMenu *self = INDICATOR_PRINTERS_MENU (object);

    switch (property_id) {
        case PROP_CUPS_NOTIFIER:
            indicator_printers_menu_set_cups_notifier (self,
                                                       g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


void
get_property (GObject        *object,
              guint           property_id,
              GValue         *value,
              GParamSpec     *pspec)
{
    IndicatorPrintersMenu *self = INDICATOR_PRINTERS_MENU (object);

    switch (property_id) {
        case PROP_CUPS_NOTIFIER:
            g_value_set_object (value,
                                indicator_printers_menu_get_cups_notifier (self));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
indicator_printers_menu_class_init (IndicatorPrintersMenuClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (IndicatorPrintersMenuPrivate));

    object_class->dispose = dispose;
    object_class->get_property = get_property;
    object_class->set_property = set_property;

    properties[PROP_CUPS_NOTIFIER] = g_param_spec_object ("cups-notifier",
                                                          "Cups Notifier",
                                                          "A cups notifier object",
                                                          CUPS_TYPE_NOTIFIER,
                                                          G_PARAM_READWRITE);

    g_object_class_install_properties (object_class, NUM_PROPERTIES, properties);
}


static int
get_number_of_active_jobs (const gchar *printer)
{
    int njobs;
    cups_job_t *jobs;

    njobs = cupsGetJobs (&jobs, printer, 1, CUPS_WHICHJOBS_ACTIVE);
    cupsFreeJobs (njobs, jobs);

    return njobs;
}


static void
show_system_settings (DbusmenuMenuitem *menuitem,
                      guint timestamp,
                      gpointer user_data)
{
    GAppInfo *appinfo;
    GError *err = NULL;
    const gchar *printer = user_data;
    gchar *cmdline;

    cmdline = g_strdup_printf ("gnome-control-center printers show-printer %s",
                               printer);

    appinfo = g_app_info_create_from_commandline (cmdline,
                                                  "gnome-control-center",
                                                  G_APP_INFO_CREATE_SUPPORTS_STARTUP_NOTIFICATION,
                                                  &err);
    g_free (cmdline);

    if (err) {
        g_warning ("failed to create application info: %s", err->message);
        g_error_free (err);
        return;
    }

    g_app_info_launch (appinfo, NULL, NULL, &err);
    if (err) {
        g_warning ("failed to launch gnome-control-center: %s", err->message);
        g_error_free (err);
    }

    g_object_unref (appinfo);
}


static void
update_printer_menuitem (IndicatorPrintersMenu *self,
                         const char *printer,
                         int state,
                         int njobs)
{
    DbusmenuMenuitem *item;

    item = g_hash_table_lookup (self->priv->printers, printer);

    if (!item) {
        item = dbusmenu_menuitem_new ();
        dbusmenu_menuitem_property_set (item, "type", "indicator-item");
        dbusmenu_menuitem_property_set (item, "indicator-icon-name", "printer");
        dbusmenu_menuitem_property_set (item, "indicator-label", printer);
        g_signal_connect_data (item, "item-activated",
                               G_CALLBACK (show_system_settings),
                               g_strdup (printer), (GClosureNotify) g_free, 0);

        dbusmenu_menuitem_child_append(self->priv->root, item);
        g_hash_table_insert (self->priv->printers, g_strdup (printer), item);
    }

    if (njobs == 0) {
        dbusmenu_menuitem_property_set_bool (item, "visible", FALSE);
        return;
    }

    dbusmenu_menuitem_property_set_bool (item, "visible", TRUE);

    switch (state) {
        case IPP_PRINTER_STOPPED:
            dbusmenu_menuitem_property_set (item, "indicator-right", "Paused");
            dbusmenu_menuitem_property_set_bool (item, "indicator-right-is-lozenge", FALSE);
            break;

        case IPP_PRINTER_PROCESSING: {
            gchar *jobstr = g_strdup_printf ("%d", njobs);
            dbusmenu_menuitem_property_set (item, "indicator-right", jobstr);
            dbusmenu_menuitem_property_set_bool (item, "indicator-right-is-lozenge", TRUE);
            g_free (jobstr);
            break;
        }
    }
}


static void
update_job (CupsNotifier *cups_notifier,
            const gchar *text,
            const gchar *printer_uri,
            const gchar *printer_name,
            guint printer_state,
            const gchar *printer_state_reasons,
            gboolean printer_is_accepting_jobs,
            guint job_id,
            guint job_state,
            const gchar *job_state_reasons,
            const gchar *job_name,
            guint job_impressions_completed,
            gpointer user_data)
{
    IndicatorPrintersMenu *self = INDICATOR_PRINTERS_MENU (user_data);

    update_printer_menuitem (self,
                             printer_name,
                             printer_state,
                             get_number_of_active_jobs (printer_name));
}


static void
on_printer_state_changed (CupsNotifier *object,
                          const gchar *text,
                          const gchar *printer_uri,
                          const gchar *printer_name,
                          guint printer_state,
                          const gchar *printer_state_reasons,
                          gboolean printer_is_accepting_jobs,
                          gpointer user_data)
{
    IndicatorPrintersMenu *self = INDICATOR_PRINTERS_MENU (user_data);

    update_printer_menuitem (self,
                             printer_name,
                             printer_state,
                             get_number_of_active_jobs (printer_name));
}


static void
indicator_printers_menu_init (IndicatorPrintersMenu *self)
{
    int ndests, i;
    cups_dest_t *dests;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              INDICATOR_TYPE_PRINTERS_MENU,
                                              IndicatorPrintersMenuPrivate);

    self->priv->root = dbusmenu_menuitem_new ();

    self->priv->printers = g_hash_table_new_full (g_str_hash,
                                                  g_str_equal,
                                                  g_free,
                                                  g_object_unref);

    /* create initial menu items */
    ndests = cupsGetDests (&dests);
    for (i = 0; i < ndests; i++) {
        int state = atoi (cupsGetOption ("printer-state",
                                         dests[i].num_options,
                                         dests[i].options));
        update_printer_menuitem (self,
                                 dests[i].name,
                                 state,
                                 get_number_of_active_jobs (dests[i].name));
    }
    cupsFreeDests (ndests, dests);
}


IndicatorPrintersMenu *
indicator_printers_menu_new (void)
{
    return g_object_new (INDICATOR_TYPE_PRINTERS_MENU, NULL);
}


DbusmenuMenuitem *
indicator_printers_menu_get_root (IndicatorPrintersMenu *self)
{
    return self->priv->root;
}


CupsNotifier *
indicator_printers_menu_get_cups_notifier (IndicatorPrintersMenu *self)
{
    return self->priv->cups_notifier;
}


void
indicator_printers_menu_set_cups_notifier (IndicatorPrintersMenu *self,
                                           CupsNotifier *cups_notifier)
{
    if (self->priv->cups_notifier) {
        g_object_disconnect (self->priv->cups_notifier,
                             "any-signal", update_job, self,
                             "any-signal", on_printer_state_changed, self,
                             NULL);
        g_object_unref (self->priv->cups_notifier);
    }

    self->priv->cups_notifier = cups_notifier;

    if (self->priv->cups_notifier) {
        g_object_connect (self->priv->cups_notifier,
                          "signal::job-created", update_job, self,
                          "signal::job-state", update_job, self,
                          "signal::job-completed", update_job, self,
                          "signal::printer-state-changed", on_printer_state_changed, self,
                          NULL);
    }
}

