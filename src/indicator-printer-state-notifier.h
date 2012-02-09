
#ifndef INDICATOR_PRINTER_STATE_NOTIFIER_H
#define INDICATOR_PRINTER_STATE_NOTIFIER_H

#include <glib-object.h>
#include "cups-notifier.h"

G_BEGIN_DECLS

#define INDICATOR_TYPE_PRINTER_STATE_NOTIFIER indicator_printer_state_notifier_get_type()

#define INDICATOR_PRINTER_STATE_NOTIFIER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  INDICATOR_TYPE_PRINTER_STATE_NOTIFIER, IndicatorPrinterStateNotifier))

#define INDICATOR_PRINTER_STATE_NOTIFIER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  INDICATOR_TYPE_PRINTER_STATE_NOTIFIER, IndicatorPrinterStateNotifierClass))

#define INDICATOR_IS_PRINTER_STATE_NOTIFIER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  INDICATOR_TYPE_PRINTER_STATE_NOTIFIER))

#define INDICATOR_IS_PRINTER_STATE_NOTIFIER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  INDICATOR_TYPE_PRINTER_STATE_NOTIFIER))

#define INDICATOR_PRINTER_STATE_NOTIFIER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  INDICATOR_TYPE_PRINTER_STATE_NOTIFIER, IndicatorPrinterStateNotifierClass))

typedef struct _IndicatorPrinterStateNotifier IndicatorPrinterStateNotifier;
typedef struct _IndicatorPrinterStateNotifierClass IndicatorPrinterStateNotifierClass;
typedef struct _IndicatorPrinterStateNotifierPrivate IndicatorPrinterStateNotifierPrivate;

struct _IndicatorPrinterStateNotifier
{
  GObject parent;

  IndicatorPrinterStateNotifierPrivate *priv;
};

struct _IndicatorPrinterStateNotifierClass
{
  GObjectClass parent_class;
};

GType indicator_printer_state_notifier_get_type (void) G_GNUC_CONST;

CupsNotifier * indicator_printer_state_notifier_get_cups_notifier (IndicatorPrinterStateNotifier *self);
void indicator_printer_state_notifier_set_cups_notifier (IndicatorPrinterStateNotifier *self,
                                                         CupsNotifier *cups_notifier);


G_END_DECLS

#endif

