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

#include <gio/gio.h>


void
show_printer_settings (const gchar *printer)
{
    GAppInfo *appinfo;
    GError *err = NULL;
    gchar *cmdline;

    if (printer)
        cmdline = g_strdup_printf ("gnome-control-center printers show-printer %s",
                                   printer);
    else
        cmdline = g_strdup_printf ("gnome-control-center printers");

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

