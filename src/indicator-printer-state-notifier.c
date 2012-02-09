
#include "indicator-printer-state-notifier.h"

#include <gtk/gtk.h>
#include <cups/cups.h>
#include <string.h>
#include <stdarg.h>

#include "cups-notifier.h"


#define RESPONSE_SHOW_SYSTEM_SETTINGS 1


G_DEFINE_TYPE (IndicatorPrinterStateNotifier, indicator_printer_state_notifier, G_TYPE_OBJECT)


struct _IndicatorPrinterStateNotifierPrivate
{
    CupsNotifier *cups_notifier;

    /* printer states that were already notified about in this session */
    GHashTable *notified_printer_states;

    /* state-reason -> user visible string with a %s for printer name */
    GHashTable *printer_alerts;
};


enum {
    PROP_0,
    PROP_CUPS_NOTIFIER,
    NUM_PROPERTIES
};

GParamSpec *properties[NUM_PROPERTIES];


static void
g_hash_table_insert_many (GHashTable *hash_table,
                          gpointer key,
                          ...)
{
    va_list args;

    va_start (args, key);
    while (key) {
        gpointer value = va_arg (args, gpointer);
        g_hash_table_insert (hash_table, key, value);
        key = va_arg (args, gpointer);
    }
    va_end (args);
}


static gboolean
g_strv_contains (gchar **str_array,
                 gchar *needle)
{
    gchar **str;

    if (!str_array)
        return FALSE;

    for (str = str_array; *str; str++) {
        if (!strcmp (*str, needle))
            return TRUE;
    }

    return FALSE;
}


/* returns a list of strings that are in a but not in b; does not copy the
 * strings */
static GList *
g_strv_diff (gchar **a,
             gchar **b)
{
    GList *result = NULL;
    gchar **p;

    if (!a)
        return NULL;

    for (p = a; *p; p++) {
        if (!g_strv_contains (b, *p))
            result = g_list_prepend (result, *p);
    }

    return g_list_reverse (result);
}


void
show_alert_box (const gchar *printer,
                const gchar *reason,
                int njobs)
{
    GtkWidget *dialog;
    GtkWidget *image;
    gchar *primary_text;
    gchar *secondary_text;
    const gchar *fmt;

    image = gtk_image_new_from_icon_name ("printer", GTK_ICON_SIZE_DIALOG);
    primary_text = g_strdup_printf (reason, printer);

    if (njobs == 1)
        fmt = "You have %d job queued to print on this printer.";
    else
        fmt = "You have %d jobs queued to print on this printer.";
    secondary_text = g_strdup_printf (fmt, njobs);

    dialog = g_object_new (GTK_TYPE_MESSAGE_DIALOG,
                           "title", "Printing Problem",
                           "icon-name", "printer",
                           "image", image,
                           "text", primary_text,
                           "secondary-text", secondary_text,
                           "urgency-hint", TRUE,
                           "focus-on-map", FALSE,
                           "window-position", GTK_WIN_POS_CENTER,
                           "skip-taskbar-hint", FALSE,
                           "deletable", FALSE,
                           NULL);

    g_free (primary_text);
    g_free (secondary_text);

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            "_Settings…", RESPONSE_SHOW_SYSTEM_SETTINGS,
                            GTK_STOCK_OK, GTK_RESPONSE_OK,
                            NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                     GTK_RESPONSE_OK);
    gtk_widget_show_all (dialog);

    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);
}


static void
on_printer_state_changed (CupsNotifier *object,
                          const gchar *text,
                          const gchar *printer_uri,
                          const gchar *printer,
                          guint printer_state,
                          const gchar *printer_state_reasons,
                          gboolean printer_is_accepting_jobs,
                          gpointer user_data)
{
    IndicatorPrinterStateNotifierPrivate *priv = INDICATOR_PRINTER_STATE_NOTIFIER (user_data)->priv;
    int njobs;
    cups_job_t *jobs;
    gchar **state_reasons, **already_notified;
    GList *new_state_reasons, *it;

    njobs = cupsGetJobs (&jobs, printer, 1, CUPS_WHICHJOBS_ACTIVE);
    cupsFreeJobs (njobs, jobs);

    /* don't show any events if the current user does not have jobs queued on
     * that printer or this printer is unknown to CUPS */
    if (njobs <= 0)
        return;

    state_reasons = g_strsplit (printer_state_reasons, " ", 0);
    already_notified = g_hash_table_lookup (priv->notified_printer_states,
                                            printer);

    new_state_reasons = g_strv_diff (state_reasons, already_notified);

    for (it = new_state_reasons; it; it = g_list_next (new_state_reasons)) {
        const gchar *reason_text = g_hash_table_lookup (priv->printer_alerts,
                                                        it->data);
        if (reason_text)
            show_alert_box (printer, reason_text, njobs);
    }

    g_list_free (new_state_reasons);

    g_hash_table_replace (priv->notified_printer_states,
                          g_strdup (printer),
                          state_reasons);
}


static void
get_property (GObject    *object,
              guint       property_id,
              GValue     *value,
              GParamSpec *pspec)
{
    IndicatorPrinterStateNotifier *self = INDICATOR_PRINTER_STATE_NOTIFIER (object);

    switch (property_id)
    {
        case PROP_CUPS_NOTIFIER:
            g_value_set_object (value,
                                indicator_printer_state_notifier_get_cups_notifier (self));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
set_property (GObject      *object,
              guint         property_id,
              const GValue *value,
              GParamSpec   *pspec)
{
    IndicatorPrinterStateNotifier *self = INDICATOR_PRINTER_STATE_NOTIFIER (object);

    switch (property_id)
    {
        case PROP_CUPS_NOTIFIER:
            indicator_printer_state_notifier_set_cups_notifier (self,
                                                                g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
dispose (GObject *object)
{
    IndicatorPrinterStateNotifier *self = INDICATOR_PRINTER_STATE_NOTIFIER (object);

    if (self->priv->notified_printer_states) {
        g_hash_table_unref (self->priv->notified_printer_states);
        self->priv->notified_printer_states = NULL;
    }
    if (self->priv->printer_alerts) {
        g_hash_table_unref (self->priv->printer_alerts);
        self->priv->printer_alerts = NULL;
    }
    g_clear_object (&self->priv->cups_notifier);

    G_OBJECT_CLASS (indicator_printer_state_notifier_parent_class)->dispose (object);
}


static void
finalize (GObject *object)
{
    G_OBJECT_CLASS (indicator_printer_state_notifier_parent_class)->finalize (object);
}


static void
indicator_printer_state_notifier_class_init (IndicatorPrinterStateNotifierClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (IndicatorPrinterStateNotifierPrivate));

    object_class->get_property = get_property;
    object_class->set_property = set_property;
    object_class->dispose = dispose;
    object_class->finalize = finalize;

    properties[PROP_CUPS_NOTIFIER] = g_param_spec_object ("cups-notifier",
                                                          "Cups Notifier",
                                                          "A cups notifier object",
                                                          CUPS_TYPE_NOTIFIER,
                                                          G_PARAM_READWRITE);

    g_object_class_install_properties (object_class, NUM_PROPERTIES, properties);
}


static void
indicator_printer_state_notifier_init (IndicatorPrinterStateNotifier *self)
{
    IndicatorPrinterStateNotifierPrivate *priv;

    priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                        INDICATOR_TYPE_PRINTER_STATE_NOTIFIER,
                                        IndicatorPrinterStateNotifierPrivate);
    self->priv = priv;

    priv->notified_printer_states = g_hash_table_new_full (g_str_hash,
                                                           g_str_equal,
                                                           g_free,
                                                           (GDestroyNotify)g_strfreev);

    priv->printer_alerts = g_hash_table_new (g_str_hash, g_str_equal);
    g_hash_table_insert_many ( priv->printer_alerts,
                               "media-low", "The printer “%s” is low on paper.",
                               "media-empty", "The printer “%s” is out of paper.",
                               "toner-low", "The printer “%s” is low on toner.",
                               "toner-empty", "The printer “%s” is out of toner.",
                               "cover-open", "A cover is open on the printer “%s”.",
                               "door-open", "A door is open on the printer “%s”.",
                               "cups-missing-filter", "The printer “%s” can’t be used, because required software is missing.",
                               "offline", "The printer “%s” is currently off-line.",
                               NULL);
}


CupsNotifier *
indicator_printer_state_notifier_get_cups_notifier (IndicatorPrinterStateNotifier *self)
{
    return self->priv->cups_notifier;
}


void
indicator_printer_state_notifier_set_cups_notifier (IndicatorPrinterStateNotifier *self,
                                                    CupsNotifier *cups_notifier)
{
    if (self->priv->cups_notifier) {
        g_signal_handlers_disconnect_by_func (self->priv->cups_notifier,
                                              on_printer_state_changed,
                                              self);
        g_clear_object (&self->priv->cups_notifier);
    }

    if (cups_notifier) {
        self->priv->cups_notifier = g_object_ref (cups_notifier);
        g_signal_connect (cups_notifier, "printer-state-changed",
                          G_CALLBACK (on_printer_state_changed), self);
    }
}

