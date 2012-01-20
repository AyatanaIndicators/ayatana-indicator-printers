
#include "indicator-printers-menu.h"

#include <gio/gio.h>


G_DEFINE_TYPE (IndicatorPrintersMenu, indicator_printers_menu, G_TYPE_OBJECT)

#define PRINTERS_MENU_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), INDICATOR_TYPE_PRINTERS_MENU, IndicatorPrintersMenuPrivate))


struct _IndicatorPrintersMenuPrivate
{
    DbusmenuMenuitem *root;
};


static void
dispose (GObject *object)
{
    IndicatorPrintersMenuPrivate *priv = PRINTERS_MENU_PRIVATE (object);

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

    cmdline = g_strdup_printf ("gnome-control-center printing show-printer %s",
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
                      const gchar *printer)
{
    IndicatorPrintersMenuPrivate *priv = PRINTERS_MENU_PRIVATE (self);
    DbusmenuMenuitem *child;

    child = dbusmenu_menuitem_new ();
    dbusmenu_menuitem_property_set (child, "indicator-icon-name", "printer");
    dbusmenu_menuitem_property_set (child, "indicator-label", printer);
    dbusmenu_menuitem_property_set (child, "indicator-right", "Paused");
    dbusmenu_menuitem_property_set (child, "type", "indicator-item");
    g_signal_connect_data (child,
                           "item-activated",
                           G_CALLBACK (show_system_settings),
                           g_strdup (printer),
                           (GClosureNotify) g_free,
                           0);

    dbusmenu_menuitem_child_append(priv->root, child);
    g_object_unref (child);
}


static void
indicator_printers_menu_init (IndicatorPrintersMenu *self)
{
    IndicatorPrintersMenuPrivate *priv = PRINTERS_MENU_PRIVATE (self);

    priv->root = dbusmenu_menuitem_new ();
    add_printer_menuitem (self, "canon-mono-duplex");
    add_printer_menuitem (self, "kitchen");
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

