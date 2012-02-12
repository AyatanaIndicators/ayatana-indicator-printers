
#include "spawn-printer-settings.h"

void
spawn_printer_settings ()
{
    spawn_printer_settings_with_args (NULL);
}


void
spawn_printer_settings_with_args (const gchar *fmt,
                                  ...)
{
    GString *cmdline;
    GError *err = NULL;

    cmdline = g_string_new ("gnome-control-center printers ");

    if (fmt) {
        va_list args;
        va_start (args, fmt);
        g_string_append_vprintf (cmdline, fmt, args);
        va_end (args);
    }

    g_spawn_command_line_async (cmdline->str, &err);
    if (err) {
        g_warning ("Could not spawn printer settings: %s", err->message);
        g_error_free (err);
    }

    g_string_free (cmdline, TRUE);
}

