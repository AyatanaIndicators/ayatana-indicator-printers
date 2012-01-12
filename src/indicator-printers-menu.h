#ifndef INDICATOR_PRINTERS_MENU_H
#define INDICATOR_PRINTERS_MENU_H

#include <glib-object.h>
#include <libdbusmenu-glib/dbusmenu-glib.h>

G_BEGIN_DECLS

#define INDICATOR_TYPE_PRINTERS_MENU indicator_printers_menu_get_type()

#define INDICATOR_PRINTERS_MENU(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  INDICATOR_TYPE_PRINTERS_MENU, IndicatorPrintersMenu))

#define INDICATOR_PRINTERS_MENU_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  INDICATOR_TYPE_PRINTERS_MENU, IndicatorPrintersMenuClass))

#define INDICATOR_IS_PRINTERS_MENU(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  INDICATOR_TYPE_PRINTERS_MENU))

#define INDICATOR_IS_PRINTERS_MENU_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  INDICATOR_TYPE_PRINTERS_MENU))

#define INDICATOR_PRINTERS_MENU_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  INDICATOR_TYPE_PRINTERS_MENU, IndicatorPrintersMenuClass))

typedef struct _IndicatorPrintersMenu IndicatorPrintersMenu;
typedef struct _IndicatorPrintersMenuClass IndicatorPrintersMenuClass;
typedef struct _IndicatorPrintersMenuPrivate IndicatorPrintersMenuPrivate;

struct _IndicatorPrintersMenu
{
  GObject parent;
};

struct _IndicatorPrintersMenuClass
{
  GObjectClass parent_class;
};

GType indicator_printers_menu_get_type (void) G_GNUC_CONST;

IndicatorPrintersMenu *indicator_printers_menu_new (void);
DbusmenuMenuitem * indicator_printers_menu_get_root (IndicatorPrintersMenu *menu);

G_END_DECLS

#endif

