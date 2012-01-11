/*
 * Copyright 2012 Canonical Ltd.
 *
 * Authors: Lars Uebernickel <lars.uebernickel@canonical.com>
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

#include <libindicator/indicator-service.h>
#include <libdbusmenu-glib/dbusmenu-glib.h>
#include <gtk/gtk.h>
#include "dbus-names.h"


static void
service_shutdown (IndicatorService *service, gpointer user_data)
{
    g_debug("Shutting down indicator-printers-service");
    gtk_main_quit ();
}


static void
show_system_settings (DbusmenuMenuitem *menuitem,
                      guint timestamp,
                      gpointer user_data)
{
    GAppInfo *appinfo;
    GError *err = NULL;

    appinfo = g_app_info_create_from_commandline ("gnome-control-center printing",
                                                  "gnome-control-center",
                                                  G_APP_INFO_CREATE_SUPPORTS_STARTUP_NOTIFICATION,
                                                  &err);
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
dbusmenu_menuitem_append_separator (DbusmenuMenuitem *item)
{
    DbusmenuMenuitem *separator;

    separator = dbusmenu_menuitem_new();
    dbusmenu_menuitem_property_set(separator,
                                   "type",
                                   DBUSMENU_CLIENT_TYPES_SEPARATOR);

    dbusmenu_menuitem_child_append(item, separator);
    g_object_unref (separator);
}


static void
dbusmenu_menuitem_append_label (DbusmenuMenuitem *item,
                                const gchar *text,
                                GCallback activated,
                                gpointer user_data)
{
    DbusmenuMenuitem *child;

    child = dbusmenu_menuitem_new ();
    dbusmenu_menuitem_property_set (child, "label", text);
    g_signal_connect (child, "item-activated", activated, user_data);

    dbusmenu_menuitem_child_append(item, child);
    g_object_unref (child);
}


static DbusmenuMenuitem *
create_menu ()
{
    DbusmenuMenuitem *root;

    root = dbusmenu_menuitem_new ();
    dbusmenu_menuitem_append_separator (root);
    dbusmenu_menuitem_append_label (root,
                                    "Printer Settings…",
                                    G_CALLBACK(show_system_settings),
                                    NULL);
    return root;
}


int main (int argc, char *argv[])
{
    IndicatorService *service;
    DbusmenuServer *menuserver;
    DbusmenuMenuitem *root;
    GError *err = NULL;

    gtk_init (&argc, &argv);

    service = indicator_service_new_version (INDICATOR_PRINTERS_DBUS_NAME,
                                             INDICATOR_PRINTERS_DBUS_VERSION);
    g_signal_connect (service,
                      "shutdown",
                      G_CALLBACK (service_shutdown),
                      NULL);

    menuserver = dbusmenu_server_new (INDICATOR_PRINTERS_DBUS_OBJECT_PATH);
    root = create_menu ();
    dbusmenu_server_set_root (menuserver, root);
    g_object_unref (root);

    gtk_main ();

    g_object_unref (menuserver);
    g_object_unref (service);
    return 0;
}

