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
#include "indicator-menu-item.h"
#include "dbus-names.h"

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <libindicator/indicator.h>
#include <libindicator/indicator-image-helper.h>
#include <libindicator/indicator-service-manager.h>

#include <libdbusmenu-gtk/menu.h>
#include <libdbusmenu-gtk/menuitem.h>


INDICATOR_SET_VERSION
INDICATOR_SET_TYPE(INDICATOR_PRINTERS_TYPE)


G_DEFINE_TYPE (IndicatorPrinters, indicator_printers, INDICATOR_OBJECT_TYPE)


struct _IndicatorPrintersPrivate
{
    IndicatorServiceManager *service;
    IndicatorObjectEntry entry;
};


static void
dispose (GObject *object)
{
    IndicatorPrinters *self = INDICATOR_PRINTERS (object);
    g_clear_object (&self->priv->service);
    g_clear_object (&self->priv->entry.menu);
    g_clear_object (&self->priv->entry.image);
    G_OBJECT_CLASS (indicator_printers_parent_class)->dispose (object);
}


static GList *
get_entries (IndicatorObject *io)
{
    IndicatorPrinters *self = INDICATOR_PRINTERS (io);
    return g_list_append (NULL, &self->priv->entry);
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
connection_changed (IndicatorServiceManager *service,
                    gboolean connected,
                    gpointer user_data)
{
    IndicatorPrinters *self = INDICATOR_PRINTERS (user_data);

    if (!connected)
        indicator_object_set_visible (INDICATOR_OBJECT (self), FALSE);
}


static GdkPixbuf *
gdk_pixbuf_new_from_encoded_data (guchar *data,
                                  gsize length)
{
    GInputStream * input;
    GError *err = NULL;
    GdkPixbuf *img;

    input = g_memory_input_stream_new_from_data(data, length, NULL);
    if (input == NULL)
        return NULL;

    img = gdk_pixbuf_new_from_stream(input, NULL, &err);
    if (err) {
        g_warning("%s", err->message);
        g_error_free(err);
    }

    g_object_unref(input);
    return img;
}


static GdkPixbuf *
g_variant_get_image (GVariant *value)
{
    const gchar *strvalue = NULL;
    gsize length = 0;
    guchar *icondata;
    GdkPixbuf *img;

    if (g_variant_is_of_type (value, G_VARIANT_TYPE_STRING))
        strvalue = g_variant_get_string(value, NULL);

    if (!strvalue || !*strvalue) {
        g_warning ("%s: value does not contain a base64 encoded image",
                   __func__);
        return NULL;
    }

    icondata = g_base64_decode(strvalue, &length);
    img = gdk_pixbuf_new_from_encoded_data (icondata, length);

    g_free(icondata);
    return img;
}


static gboolean
properties_match (const gchar *name,
                  const gchar *prop,
                  GVariant *value,
                  const GVariantType *type)
{
    return !g_strcmp0 (name, prop) && g_variant_is_of_type (value, type);
}


static void
indicator_prop_change_cb (DbusmenuMenuitem *mi,
                          gchar *prop,
                          GVariant *value,
                          gpointer user_data)
{
    IndicatorMenuItem *menuitem = user_data;

    if (properties_match (prop, "indicator-label", value, G_VARIANT_TYPE_STRING))
        indicator_menu_item_set_label (menuitem, g_variant_get_string (value, NULL));

    else if (properties_match (prop, "indicator-right", value, G_VARIANT_TYPE_STRING))
        indicator_menu_item_set_right (menuitem, g_variant_get_string (value, NULL));

    else if (properties_match (prop, "indicator-icon-name", value, G_VARIANT_TYPE_STRING))
        indicator_menu_item_set_icon_name (menuitem, g_variant_get_string (value, NULL));

    else if (properties_match (prop, "indicator-icon", value, G_VARIANT_TYPE_STRING)) {
        GdkPixbuf *pb = g_variant_get_image (value);
        indicator_menu_item_set_icon (menuitem, pb);
        g_object_unref (pb);
    }

    else if (properties_match (prop, "visible", value, G_VARIANT_TYPE_BOOLEAN))
        gtk_widget_set_visible (GTK_WIDGET (menuitem), g_variant_get_boolean (value));

    else if (properties_match (prop, "indicator-right-is-lozenge", value, G_VARIANT_TYPE_BOOLEAN))
        indicator_menu_item_set_right_is_lozenge (menuitem, g_variant_get_boolean (value));
}


static void
root_property_changed (DbusmenuMenuitem *mi,
                       gchar *prop,
                       GVariant *value,
                       gpointer user_data)
{
    IndicatorObject *io = user_data;

    if (properties_match (prop, "visible", value, G_VARIANT_TYPE_BOOLEAN))
        indicator_object_set_visible (io, g_variant_get_boolean (value));
}


static gboolean
new_indicator_item (DbusmenuMenuitem *newitem,
                    DbusmenuMenuitem *parent,
                    DbusmenuClient *client,
                    gpointer user_data)
{
    GtkWidget *menuitem;
    const gchar *icon_name, *text, *right_text;
    GVariant *icon;
    gboolean is_lozenge, visible;

    icon_name = dbusmenu_menuitem_property_get (newitem, "indicator-icon-name");
    icon = dbusmenu_menuitem_property_get_variant (newitem, "indicator-icon");
    text = dbusmenu_menuitem_property_get (newitem, "indicator-label");
    right_text = dbusmenu_menuitem_property_get (newitem, "indicator-right");
    is_lozenge = dbusmenu_menuitem_property_get_bool (newitem, "indicator-right-is-lozenge");
    visible = dbusmenu_menuitem_property_get_bool (newitem, "visible");

    menuitem = g_object_new (INDICATOR_TYPE_MENU_ITEM,
                             "icon-name", icon_name,
                             "label", text,
                             "right", right_text,
                             "right-is-lozenge", is_lozenge,
                             "visible", visible,
                             NULL);
    if (icon) {
        GdkPixbuf *pb = g_variant_get_image (icon);
        indicator_menu_item_set_icon (INDICATOR_MENU_ITEM (menuitem), pb);
        g_object_unref (pb);
    }
    gtk_widget_show_all (menuitem);

    dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client),
                                    newitem,
                                    GTK_MENU_ITEM (menuitem),
                                    parent);

    g_signal_connect(G_OBJECT(newitem),
                     "property-changed",
                     G_CALLBACK(indicator_prop_change_cb),
                     menuitem);

    return TRUE;
}


static void
root_changed (DbusmenuClient *client,
              DbusmenuMenuitem *newroot,
              gpointer user_data)
{
    IndicatorPrinters *indicator = user_data;
    gboolean is_visible;

    if (newroot) {
        is_visible = dbusmenu_menuitem_property_get_bool (newroot, "visible");
        g_signal_connect (newroot, "property-changed",
                          G_CALLBACK (root_property_changed), indicator);
    }
    else
        is_visible = FALSE;

    indicator_object_set_visible (INDICATOR_OBJECT (indicator), is_visible);
}


static void
indicator_printers_init (IndicatorPrinters *self)
{
    IndicatorPrintersPrivate *priv;
    DbusmenuGtkMenu *menu;
    DbusmenuClient *client;
    GtkImage *image;

    priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                        INDICATOR_PRINTERS_TYPE,
                                        IndicatorPrintersPrivate);
    self->priv = priv;

    priv->service = indicator_service_manager_new_version (INDICATOR_PRINTERS_DBUS_NAME,
                                                           INDICATOR_PRINTERS_DBUS_VERSION);
    g_signal_connect (priv->service, "connection-change",
                      G_CALLBACK (connection_changed), self);

    menu = dbusmenu_gtkmenu_new(INDICATOR_PRINTERS_DBUS_NAME,
                                INDICATOR_PRINTERS_DBUS_OBJECT_PATH);

    client = DBUSMENU_CLIENT (dbusmenu_gtkmenu_get_client (menu));
    dbusmenu_client_add_type_handler(client,
                                     "indicator-item",
                                     new_indicator_item);
    g_signal_connect (client, "root-changed", G_CALLBACK (root_changed), self);

    image = indicator_image_helper ("printer-symbolic");
    gtk_widget_show (GTK_WIDGET (image));

    priv->entry.name_hint = PACKAGE_NAME;
    priv->entry.accessible_desc = _("Printers");
    priv->entry.menu = GTK_MENU (g_object_ref_sink (menu));
    priv->entry.image = g_object_ref_sink (image);

    indicator_object_set_visible (INDICATOR_OBJECT (self), FALSE);
}


IndicatorPrinters *
indicator_printers_new (void)
{
    return g_object_new (INDICATOR_PRINTERS_TYPE, NULL);
}

