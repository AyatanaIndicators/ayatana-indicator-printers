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

#ifndef INDICATOR_PRINTERS_H
#define INDICATOR_PRINTERS_H

#include <libindicator/indicator-object.h>

G_BEGIN_DECLS

#define INDICATOR_PRINTERS_TYPE indicator_printers_get_type()

#define INDICATOR_PRINTERS(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  INDICATOR_PRINTERS_TYPE, IndicatorPrinters))

#define INDICATOR_PRINTERS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  INDICATOR_PRINTERS_TYPE, IndicatorPrintersClass))

#define INDICATOR_IS_PRINTERS(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  INDICATOR_PRINTERS_TYPE))

#define INDICATOR_IS_PRINTERS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  INDICATOR_PRINTERS_TYPE))

#define INDICATOR_PRINTERS_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  INDICATOR_PRINTERS_TYPE, IndicatorPrintersClass))

typedef struct _IndicatorPrinters IndicatorPrinters;
typedef struct _IndicatorPrintersClass IndicatorPrintersClass;
typedef struct _IndicatorPrintersPrivate IndicatorPrintersPrivate;

struct _IndicatorPrinters
{
  IndicatorObject parent;
};

struct _IndicatorPrintersClass
{
  IndicatorObjectClass parent_class;
};

GType indicator_printers_get_type (void) G_GNUC_CONST;

IndicatorPrinters *indicator_printers_new (void);

G_END_DECLS

#endif

