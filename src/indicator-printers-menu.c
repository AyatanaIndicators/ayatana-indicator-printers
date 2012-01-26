
#include "indicator-printers-menu.h"

#include <gio/gio.h>
#include <cups/cups.h>


G_DEFINE_TYPE (IndicatorPrintersMenu, indicator_printers_menu, G_TYPE_OBJECT)

#define PRINTERS_MENU_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), INDICATOR_TYPE_PRINTERS_MENU, IndicatorPrintersMenuPrivate))


struct _IndicatorPrintersMenuPrivate
{
    DbusmenuMenuitem *root;
    GHashTable *printers;    /* printer name -> dbusmenuitem */
};


static void
dispose (GObject *object)
{
    IndicatorPrintersMenuPrivate *priv = PRINTERS_MENU_PRIVATE (object);

    if (priv->printers) {
        g_hash_table_unref (priv->printers);
        priv->printers = NULL;
    }

    g_clear_object (&priv->root);

    G_OBJECT_CLASS (indicator_printers_menu_parent_class)->dispose (object);
}


static void
indicator_printers_menu_class_init (IndicatorPrintersMenuClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (IndicatorPrintersMenuPrivate));

    object_class->dispose = dispose;
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
add_printer_menuitem (IndicatorPrintersMenu *self,
                      const char *printer,
                      int state,
                      int njobs)
{
    IndicatorPrintersMenuPrivate *priv = PRINTERS_MENU_PRIVATE (self);
    DbusmenuMenuitem *item;

    item = dbusmenu_menuitem_new ();
    dbusmenu_menuitem_property_set (item, "type", "indicator-item");
    dbusmenu_menuitem_property_set (item, "indicator-icon-name", "printer");
    dbusmenu_menuitem_property_set (item, "indicator-label", printer);
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

    g_signal_connect_data (item, "item-activated",
                           G_CALLBACK (show_system_settings),
                           g_strdup (printer), (GClosureNotify) g_free, 0);

    dbusmenu_menuitem_child_append(priv->root, item);
    g_hash_table_insert (priv->printers, g_strdup (printer), item);
}


static void
indicator_printers_menu_init (IndicatorPrintersMenu *self)
{
    IndicatorPrintersMenuPrivate *priv = PRINTERS_MENU_PRIVATE (self);
    int ndests, njobs, i, state;
    cups_dest_t *dests;
    cups_job_t *jobs;

    priv->root = dbusmenu_menuitem_new ();

    priv->printers = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            g_object_unref);

    /* create initial menu items */
    ndests = cupsGetDests (&dests);
    for (i = 0; i < ndests; i++) {
        njobs = cupsGetJobs (&jobs, dests[i].name, 1, CUPS_WHICHJOBS_ACTIVE);
        state = atoi (cupsGetOption ("printer-state",
                                     dests[i].num_options,
                                     dests[i].options));

        if (njobs > 0)
            add_printer_menuitem (self, dests[i].name, state, njobs);

        cupsFreeJobs (njobs, jobs);
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
    IndicatorPrintersMenuPrivate *priv = PRINTERS_MENU_PRIVATE (self);
    return priv->root;
}

