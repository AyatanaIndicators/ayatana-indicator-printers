
#include <glib.h>
#include <cups-notifier.h>


int main (int argc, char **argv)
{
    GMainLoop *loop;
    CupsNotifier *notifier = NULL;
    GDBusConnection *con;
    GError *error = NULL;

    loop = g_main_loop_new (NULL, FALSE);

    con = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, &error);
    if (error) {
        g_printerr ("Error getting system bus: %s\n", error->message);
        g_error_free (error);
        goto out;
    }

    notifier = cups_notifier_skeleton_new ();

    g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (notifier),
                                      con,
                                      "/org/cups/cupsd/Notifier",
                                      &error);
    if (error) {
        g_printerr ("Error exporting cups Notifier object: %s\n", error->message);
        g_error_free (error);
        goto out;
    }

    cups_notifier_emit_printer_state_changed (notifier,
                                              "Printer state changed!",
                                              "file:///tmp/print",
                                              "hp-LaserJet-1012",
                                              5,
                                              "toner-low",
                                              FALSE);

    g_main_context_iteration (NULL, FALSE);

out:
    g_clear_object (&notifier);
    g_clear_object (&con);
    g_main_loop_unref (loop);
    return 0;
}

