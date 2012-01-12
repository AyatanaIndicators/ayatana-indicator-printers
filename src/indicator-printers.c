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

#include "config.h"

#include "indicator-printers.h"
#include "dbus-names.h"

#include <gtk/gtk.h>

#include <libindicator/indicator.h>
#include <libindicator/indicator-image-helper.h>

#include <libdbusmenu-gtk3/menu.h>
#include <libdbusmenu-gtk3/menuitem.h>


INDICATOR_SET_VERSION
INDICATOR_SET_TYPE(INDICATOR_PRINTERS_TYPE)


G_DEFINE_TYPE (IndicatorPrinters, indicator_printers, INDICATOR_OBJECT_TYPE)

#define INDICATOR_PRINTERS_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), INDICATOR_PRINTERS_TYPE, IndicatorPrintersPrivate))


struct _IndicatorPrintersPrivate
{
    IndicatorObjectEntry entry;
};


static void
dispose (GObject *object)
{
    IndicatorPrintersPrivate *priv = INDICATOR_PRINTERS_GET_PRIVATE (object);
    g_clear_object (&priv->entry.menu);
    g_clear_object (&priv->entry.image);
    G_OBJECT_CLASS (indicator_printers_parent_class)->dispose (object);
}


static GList *
get_entries (IndicatorObject *io)
{
    IndicatorPrintersPrivate *priv = INDICATOR_PRINTERS_GET_PRIVATE (io);
    return g_list_append (NULL, &priv->entry);
}


static void
indicator_printers_class_init (IndicatorPrintersClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    IndicatorObjectClass *io_class = INDICATOR_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (IndicatorPrintersPrivate));

    object_class->dispose = dispose;

    io_class->get_entries = get_entries;
}


static void
indicator_printers_init (IndicatorPrinters *io)
{
    IndicatorPrintersPrivate *priv = INDICATOR_PRINTERS_GET_PRIVATE (io);
    DbusmenuGtkMenu *menu;
    GtkImage *image;

    menu = dbusmenu_gtkmenu_new(INDICATOR_PRINTERS_DBUS_NAME,
                                INDICATOR_PRINTERS_DBUS_OBJECT_PATH);

    image = indicator_image_helper ("printer-symbolic");
    gtk_widget_show (GTK_WIDGET (image));

    priv->entry.name_hint = PACKAGE_NAME;
    priv->entry.accessible_desc = "Printers";
    priv->entry.menu = GTK_MENU (menu);
    priv->entry.image = image;
}


IndicatorPrinters *
indicator_printers_new (void)
{
    return g_object_new (INDICATOR_PRINTERS_TYPE, NULL);
}

