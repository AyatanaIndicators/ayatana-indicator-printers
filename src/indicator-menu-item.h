
#ifndef INDICATOR_MENU_ITEM_H
#define INDICATOR_MENU_ITEM_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define INDICATOR_TYPE_MENU_ITEM indicator_menu_item_get_type()

#define INDICATOR_MENU_ITEM(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  INDICATOR_TYPE_MENU_ITEM, IndicatorMenuItem))

#define INDICATOR_MENU_ITEM_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  INDICATOR_TYPE_MENU_ITEM, IndicatorMenuItemClass))

#define INDICATOR_IS_MENU_ITEM(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  INDICATOR_TYPE_MENU_ITEM))

#define INDICATOR_IS_MENU_ITEM_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  INDICATOR_TYPE_MENU_ITEM))

#define INDICATOR_MENU_ITEM_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  INDICATOR_TYPE_MENU_ITEM, IndicatorMenuItemClass))

typedef struct _IndicatorMenuItem IndicatorMenuItem;
typedef struct _IndicatorMenuItemClass IndicatorMenuItemClass;
typedef struct _IndicatorMenuItemPrivate IndicatorMenuItemPrivate;

struct _IndicatorMenuItem
{
    GtkMenuItem parent;
};

struct _IndicatorMenuItemClass
{
    GtkMenuItemClass parent_class;
};

GType indicator_menu_item_get_type (void) G_GNUC_CONST;

IndicatorMenuItem *indicator_menu_item_new (void);
void indicator_menu_item_set_right (IndicatorMenuItem *self, const gchar *text);
void indicator_menu_item_set_label (IndicatorMenuItem *self, const gchar *text);

const gchar * indicator_menu_item_get_icon_name (IndicatorMenuItem *self);
void indicator_menu_item_set_icon (IndicatorMenuItem *self, GdkPixbuf *icon);
GdkPixbuf * indicator_menu_item_get_icon (IndicatorMenuItem *self);
void indicator_menu_item_set_icon_name (IndicatorMenuItem *self, const gchar *name);

G_END_DECLS

#endif

