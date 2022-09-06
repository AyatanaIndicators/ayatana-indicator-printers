/*
 * Copyright 2022 Robert Tari
 *
 * Authors: Robert Tari <robert@tari.in>
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

#ifndef __INDICATOR_PRINTERS_SERVICE_H__
#define __INDICATOR_PRINTERS_SERVICE_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define INDICATOR_PRINTERS_SERVICE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), INDICATOR_TYPE_PRINTERS_SERVICE, IndicatorPrintersService))
#define INDICATOR_TYPE_PRINTERS_SERVICE (indicator_printers_service_get_type ())
#define INDICATOR_IS_PRINTERS_SERVICE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), INDICATOR_TYPE_PRINTERS_SERVICE))

typedef struct _IndicatorPrintersService IndicatorPrintersService;
typedef struct _IndicatorPrintersServiceClass IndicatorPrintersServiceClass;
typedef struct _IndicatorPrintersServicePrivate IndicatorPrintersServicePrivate;

struct _IndicatorPrintersService
{
    GObject parent;
    IndicatorPrintersServicePrivate *pPrivate;
};

struct _IndicatorPrintersServiceClass
{
    GObjectClass parent_class;
    void (*pNameLost) (IndicatorPrintersService *self);
};

GType indicator_printers_service_get_type (void);
IndicatorPrintersService *indicator_printers_service_new ();

G_END_DECLS

#endif
