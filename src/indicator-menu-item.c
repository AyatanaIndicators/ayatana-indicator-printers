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

#include "indicator-menu-item.h"

#include <math.h>

struct _IndicatorMenuItemPrivate
{
    GtkImage *image;
    GtkWidget *label;
    GtkWidget *right_label;
    gboolean right_is_lozenge;
};

G_DEFINE_TYPE_WITH_PRIVATE(IndicatorMenuItem, indicator_menu_item, GTK_TYPE_MENU_ITEM)

enum {
    PROP_0,
    PROP_ICON,
    PROP_ICON_NAME,
    PROP_LABEL,
    PROP_RIGHT,
    PROP_RIGHT_IS_LOZENGE,
    N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];


static gint
gtk_widget_get_font_size (GtkWidget *widget)
{
    const PangoFontDescription *font;

    gtk_style_context_get(gtk_widget_get_style_context(widget), gtk_widget_get_state_flags(widget), "font", &font, NULL);

    return pango_font_description_get_size (font) / PANGO_SCALE;
}

static void
cairo_lozenge (cairo_t *cr, double x, double y, double w, double h)
{
    double radius = MIN (w / 2.0, h / 2.0);
    double x1 = x + w - radius;
    double x2 = x + radius;
    double y1 = y + radius;
    double y2 = y + h - radius;

    cairo_move_to (cr, x+radius, y);
    cairo_arc (cr, x1, y1, radius, M_PI * 1.5, M_PI * 2);
    cairo_arc (cr, x1, y2, radius, 0,          M_PI * 0.5);
    cairo_arc (cr, x2, y2, radius, M_PI * 0.5, M_PI);
    cairo_arc (cr, x2, y1, radius, M_PI,       M_PI * 1.5);
}

static gboolean
detail_label_draw (GtkWidget *widget,
                   cairo_t *cr,
                   gpointer data)
{
    GtkAllocation allocation;
    double x, y, w, h;
    GdkRGBA color;
    PangoLayout *layout;
    PangoRectangle layout_extents;
    gboolean is_lozenge = *(gboolean *)data;
    gint font_size = gtk_widget_get_font_size (widget);

    /* let the label handle the drawing if it's not a lozenge */
    if (!is_lozenge)
        return FALSE;

    layout = gtk_label_get_layout (GTK_LABEL(widget));
    pango_layout_get_extents (layout, NULL, &layout_extents);
    pango_extents_to_pixels (&layout_extents, NULL);

    gtk_widget_get_allocation (widget, &allocation);
    x = -font_size / 2.0;
    y = 1;
    w = allocation.width;
    h = MIN (allocation.height, layout_extents.height + 4);

    if (layout_extents.width == 0)
        return TRUE;

    gtk_style_context_get_color (gtk_widget_get_style_context (widget),
                                 gtk_widget_get_state_flags (widget),
                                 &color);
    gdk_cairo_set_source_rgba (cr, &color);

    cairo_set_line_width (cr, 1.0);
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_lozenge (cr, x - font_size / 2.0, y, w + font_size, h);

    x += (w - layout_extents.width) / 2.0;
    y += (h - layout_extents.height) / 2.0;
    cairo_move_to (cr, floor (x), floor (y));
    pango_cairo_layout_path (cr, layout);
    cairo_fill (cr);

    return TRUE;
}


static void
indicator_menu_item_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    IndicatorMenuItem *self = INDICATOR_MENU_ITEM (object);

    switch (property_id)
    {
        case PROP_ICON:
            g_value_set_object (value, indicator_menu_item_get_icon (self));
            break;

        case PROP_ICON_NAME:
            g_value_set_string (value, indicator_menu_item_get_icon_name (self));
            break;

        case PROP_LABEL:
            g_value_set_string (value, gtk_label_get_label (GTK_LABEL (self->priv->label)));
            break;

        case PROP_RIGHT:
            g_value_set_string (value, gtk_label_get_label (GTK_LABEL (self->priv->right_label)));
            break;

        case PROP_RIGHT_IS_LOZENGE:
            g_value_set_boolean (value, self->priv->right_is_lozenge);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
indicator_menu_item_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    switch (property_id)
    {
        case PROP_ICON:
            indicator_menu_item_set_icon (INDICATOR_MENU_ITEM (object),
                                          g_value_get_object (value));
            break;

        case PROP_ICON_NAME:
            indicator_menu_item_set_icon_name (INDICATOR_MENU_ITEM (object),
                                               g_value_get_string (value));
            break;

        case PROP_LABEL:
            indicator_menu_item_set_label (INDICATOR_MENU_ITEM (object),
                                           g_value_get_string (value));
            break;

        case PROP_RIGHT:
            indicator_menu_item_set_right (INDICATOR_MENU_ITEM (object),
                                           g_value_get_string (value));
            break;

        case PROP_RIGHT_IS_LOZENGE:
            indicator_menu_item_set_right_is_lozenge (INDICATOR_MENU_ITEM (object),
                                                      g_value_get_boolean (value));
            break;


        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
indicator_menu_item_dispose (GObject *object)
{
    IndicatorMenuItem *self = INDICATOR_MENU_ITEM (object);

    g_clear_object (&self->priv->image);
    g_clear_object (&self->priv->label);
    g_clear_object (&self->priv->right_label);

    G_OBJECT_CLASS (indicator_menu_item_parent_class)->dispose (object);
}


static void
indicator_menu_item_class_init (IndicatorMenuItemClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = indicator_menu_item_get_property;
    object_class->set_property = indicator_menu_item_set_property;
    object_class->dispose = indicator_menu_item_dispose;

    properties[PROP_ICON] = g_param_spec_object ("icon",
                                                 "Icon",
                                                 "Icon for this menu item",
                                                 GDK_TYPE_PIXBUF,
                                                 G_PARAM_READWRITE);

    properties[PROP_ICON_NAME] = g_param_spec_string ("icon-name",
                                                      "Icon name",
                                                      "Name of the themed icon",
                                                      "",
                                                      G_PARAM_READWRITE);

    properties[PROP_LABEL] = g_param_spec_string ("label",
                                                  "Label",
                                                  "The text for the main label",
                                                  "",
                                                  G_PARAM_READWRITE);

    properties[PROP_RIGHT] = g_param_spec_string ("right",
                                                  "Right",
                                                  "The text on the right side of the menu item",
                                                  "",
                                                  G_PARAM_READWRITE);

    properties[PROP_RIGHT_IS_LOZENGE] = g_param_spec_boolean ("right-is-lozenge",
                                                              "Right is a lozenge",
                                                              "Whether the right label is displayed as a lonzenge",
                                                              FALSE,
                                                              G_PARAM_READWRITE);

    g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}


static void
indicator_menu_item_init (IndicatorMenuItem *self)
{
    IndicatorMenuItemPrivate *priv;
    gint spacing;
    GtkWidget *hbox;

    priv = indicator_menu_item_get_instance_private(self);
    self->priv = priv;

    gtk_widget_style_get (GTK_WIDGET (self),
                          "toggle-spacing", &spacing,
                          NULL);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, spacing);

    priv->image = g_object_new (GTK_TYPE_IMAGE, NULL);
    g_object_ref_sink (priv->image);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (priv->image), FALSE, FALSE, 0);

    priv->label = g_object_new (GTK_TYPE_LABEL,
                                "xalign", 0.0,
                                NULL);
    g_object_ref_sink (priv->label);
    gtk_box_pack_start (GTK_BOX (hbox), priv->label, TRUE, TRUE, 0);

    priv->right_label = g_object_new (GTK_TYPE_LABEL,
                                      "xalign", 1.0,
                                      "width-chars", 2,
                                      NULL);
    gtk_style_context_add_class (gtk_widget_get_style_context (priv->right_label),
                                 "accelerator");
    g_signal_connect (priv->right_label,
                      "draw",
                      G_CALLBACK (detail_label_draw),
                      &priv->right_is_lozenge);
    g_object_ref_sink (priv->right_label);
    gtk_box_pack_start (GTK_BOX (hbox),
                        priv->right_label,
                        FALSE,
                        FALSE,
                        gtk_widget_get_font_size (priv->right_label) / 2.0 + 1);

    gtk_container_add (GTK_CONTAINER (self), hbox);

    priv->right_is_lozenge = FALSE;
}


IndicatorMenuItem *
indicator_menu_item_new (void)
{
    return g_object_new (INDICATOR_TYPE_MENU_ITEM, NULL);
}


const gchar *
indicator_menu_item_get_label (IndicatorMenuItem *self)
{
    return gtk_label_get_label (GTK_LABEL (self->priv->label));
}


void
indicator_menu_item_set_label (IndicatorMenuItem *self,
                               const gchar *text)
{
    gtk_label_set_label (GTK_LABEL (self->priv->label), text);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LABEL]);
}


const gchar *
indicator_menu_item_get_right (IndicatorMenuItem *self)
{
    return gtk_label_get_label (GTK_LABEL (self->priv->right_label));
}


void
indicator_menu_item_set_right (IndicatorMenuItem *self,
                               const gchar *text)
{
    gtk_label_set_label (GTK_LABEL (self->priv->right_label), text);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RIGHT]);
}


gboolean
indicator_menu_item_get_right_is_lozenge (IndicatorMenuItem *self)
{
    return self->priv->right_is_lozenge;
}


void
indicator_menu_item_set_right_is_lozenge (IndicatorMenuItem *self,
                                          gboolean is_lozenge)
{
    self->priv->right_is_lozenge = is_lozenge;
    gtk_widget_queue_draw (self->priv->right_label);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RIGHT_IS_LOZENGE]);
}


GdkPixbuf *
indicator_menu_item_get_icon (IndicatorMenuItem *self)
{
    if (gtk_image_get_storage_type (self->priv->image) == GTK_IMAGE_PIXBUF)
        return gtk_image_get_pixbuf (self->priv->image);
    else
        return NULL;
}


void
indicator_menu_item_set_icon (IndicatorMenuItem *self,
                              GdkPixbuf *icon)
{
    gtk_image_set_from_pixbuf (self->priv->image, icon);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ICON]);
}


const gchar *
indicator_menu_item_get_icon_name (IndicatorMenuItem *self)
{
    const gchar *name = NULL;

    if (gtk_image_get_storage_type (self->priv->image) == GTK_IMAGE_ICON_NAME)
        gtk_image_get_icon_name (self->priv->image, &name, NULL);

    return name;
}


void
indicator_menu_item_set_icon_name (IndicatorMenuItem *self,
                                   const gchar *name)
{
    gtk_image_set_from_icon_name (self->priv->image, name, GTK_ICON_SIZE_MENU);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ICON_NAME]);
}
