
#include "indicator-menu-item.h"


G_DEFINE_TYPE (IndicatorMenuItem, indicator_menu_item, GTK_TYPE_MENU_ITEM)

#define MENU_ITEM_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), INDICATOR_TYPE_MENU_ITEM, IndicatorMenuItemPrivate))

struct _IndicatorMenuItemPrivate
{
    GtkWidget *label;
    GtkWidget *right_label;
};


enum {
    PROP_0,
    PROP_LABEL,
    PROP_RIGHT,
    N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];


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
        case PROP_LABEL:
            indicator_menu_item_set_label (INDICATOR_MENU_ITEM (object),
                                           g_value_get_string (value));

            break;

        case PROP_RIGHT:
            indicator_menu_item_set_right (INDICATOR_MENU_ITEM (object),
                                           g_value_get_string (value));
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
    g_object_ref_sink (priv->right_label);
    gtk_box_pack_start (GTK_BOX (hbox), priv->right_label, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (self), hbox);
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

