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

#include "cups-notifier.h"
#include "indicator-printers-menu.h"
#include "indicator-printer-state-notifier.h"


static void
service_shutdown (IndicatorService *service, gpointer user_data)
{
    g_debug("Shutting down indicator-printers-service");
    gtk_main_quit ();
}


int main (int argc, char *argv[])
{
    IndicatorService *service;
    DbusmenuServer *menuserver;
    CupsNotifier *cups_notifier;
    IndicatorPrintersMenu *menu;
    IndicatorPrinterStateNotifier *state_notifier;
    GError *error = NULL;

    gtk_init (&argc, &argv);

    service = indicator_service_new_version (INDICATOR_PRINTERS_DBUS_NAME,
                                             INDICATOR_PRINTERS_DBUS_VERSION);
    g_signal_connect (service,
                      "shutdown",
                      G_CALLBACK (service_shutdown),
                      NULL);

    cups_notifier = cups_notifier_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                          0,
                                                          NULL,
                                                          CUPS_DBUS_PATH,
                                                          NULL,
                                                          &error);
    if (error) {
        g_warning ("Error creating cups notify handler: %s", error->message);
        g_error_free (error);
        g_object_unref (service);
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
    g_object_unref (service);
    return 0;
}

