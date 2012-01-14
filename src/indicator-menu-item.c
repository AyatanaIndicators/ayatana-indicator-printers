
#include "indicator-menu-item.h"

#include <math.h>


G_DEFINE_TYPE (IndicatorMenuItem, indicator_menu_item, GTK_TYPE_MENU_ITEM)

#define MENU_ITEM_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), INDICATOR_TYPE_MENU_ITEM, IndicatorMenuItemPrivate))

struct _IndicatorMenuItemPrivate
{
    GtkWidget *label;
    GtkWidget *right_label;
    gboolean right_is_lozenge;
};


enum {
    PROP_0,
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

    font = gtk_style_context_get_font (gtk_widget_get_style_context (widget),
                                       gtk_widget_get_state_flags (widget));

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
    PangoLayout * layout;
    gboolean is_lozenge = *(gboolean *)data;

    gtk_widget_get_allocation (widget, &allocation);
    x = 0;
    y = 0;
    w = allocation.width;
    h = allocation.height;

    gtk_style_context_get_color (gtk_widget_get_style_context (widget),
                                 gtk_widget_get_state_flags (widget),
                                 &color);
    gdk_cairo_set_source_rgba (cr, &color);

    if (is_lozenge) {
        gint font_size = gtk_widget_get_font_size (widget);
        cairo_set_line_width (cr, 1.0);
        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_lozenge (cr, x - font_size / 2.0, y, w + font_size, h);
    }

    layout = gtk_label_get_layout (GTK_LABEL(widget));
    cairo_move_to (cr, x, y);
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
    IndicatorMenuItemPrivate *priv = MENU_ITEM_PRIVATE (object);

    switch (property_id)
    {
        case PROP_LABEL:
            g_value_set_string (value, gtk_label_get_label (GTK_LABEL (priv->label)));
            break;

        case PROP_RIGHT:
            g_value_set_string (value, gtk_label_get_label (GTK_LABEL (priv->right_label)));
            break;

        case PROP_RIGHT_IS_LOZENGE:
            g_value_set_boolean (value, priv->right_is_lozenge);
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
    IndicatorMenuItemPrivate *priv = MENU_ITEM_PRIVATE (object);

    switch (property_id)
    {
        case PROP_LABEL:
            indicator_menu_item_set_label (INDICATOR_MENU_ITEM (object),
                                           g_value_get_string (value));
            break;

        case PROP_RIGHT:
            indicator_menu_item_set_right (INDICATOR_MENU_ITEM (object),
                                           g_value_get_string (value));
            break;

        case PROP_RIGHT_IS_LOZENGE:
            priv->right_is_lozenge = g_value_get_boolean (value);
            gtk_widget_queue_draw (priv->right_label);
            break;


        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
indicator_menu_item_dispose (GObject *object)
{
    IndicatorMenuItemPrivate *priv = MENU_ITEM_PRIVATE (object);

    g_clear_object (&priv->label);
    g_clear_object (&priv->right_label);

    G_OBJECT_CLASS (indicator_menu_item_parent_class)->dispose (object);
}


static void
indicator_menu_item_class_init (IndicatorMenuItemClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (IndicatorMenuItemPrivate));

    object_class->get_property = indicator_menu_item_get_property;
    object_class->set_property = indicator_menu_item_set_property;
    object_class->dispose = indicator_menu_item_dispose;

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
    IndicatorMenuItemPrivate *priv = MENU_ITEM_PRIVATE (self);
    gint spacing;
    GtkWidget *hbox;

    gtk_widget_style_get (GTK_WIDGET (self),
                          "toggle-spacing", &spacing,
                          NULL);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, spacing);

    priv->label = g_object_new (GTK_TYPE_LABEL,
                                "xalign", 0.0,
                                NULL);
    g_object_ref_sink (priv->label);
    gtk_box_pack_start (GTK_BOX (hbox), priv->label, TRUE, TRUE, 0);

    priv->right_label = g_object_new (GTK_TYPE_LABEL,
                                      "xalign", 1.0,
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


void
indicator_menu_item_set_label (IndicatorMenuItem *self,
                               const gchar *text)
{
    IndicatorMenuItemPrivate *priv = MENU_ITEM_PRIVATE (self);
    gtk_label_set_label (GTK_LABEL (priv->label), text);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LABEL]);
}


void
indicator_menu_item_set_right (IndicatorMenuItem *self,
                               const gchar *text)
{
    IndicatorMenuItemPrivate *priv = MENU_ITEM_PRIVATE (self);
    gtk_label_set_label (GTK_LABEL (priv->right_label), text);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RIGHT]);
}

