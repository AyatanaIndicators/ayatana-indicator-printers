/*
 * Copyright 2022 Robert Tari
 *
 * Authors:
 *   Robert Tari <robert@tari.in>
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

#include <locale.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "indicator-printers-service.h"

static void onNameLost (gpointer pInstance G_GNUC_UNUSED, gpointer pLoop)
{
    g_message("Exiting: service couldn't acquire or lost ownership of busname");
    g_main_loop_quit ((GMainLoop*) pLoop);
}

int main (int argc G_GNUC_UNUSED, char **argv G_GNUC_UNUSED)
{
    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    IndicatorPrintersService *pService = indicator_printers_service_new ();
    GMainLoop *pLoop = g_main_loop_new (NULL, FALSE);
    g_signal_connect (pService, "name-lost", G_CALLBACK (onNameLost), pLoop);
    g_main_loop_run (pLoop);

    g_main_loop_unref (pLoop);
    g_clear_object (&pService);

    return 0;
}
