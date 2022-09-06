# cups-notifier.h
# cups-notifier.c
include (GdbusCodegen)
add_gdbus_codegen_with_namespace (CUPS_NOTIFIER cups-notifier org.cups.cupsd Cups "${CMAKE_CURRENT_SOURCE_DIR}/org.cups.cupsd.Notifier.xml")

# libayatanaindicatorprinterssservice.a
add_library (ayatanaindicatorprinterssservice STATIC
    indicator-printers-service.h
    indicator-printers-service.c
    indicator-printer-state-notifier.c
    indicator-printer-state-notifier.h
    spawn-printer-settings.c
    spawn-printer-settings.h
    dbus-names.h
    ${CUPS_NOTIFIER})
target_include_directories (ayatanaindicatorprinterssservice PUBLIC ${SERVICE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions (ayatanaindicatorprinterssservice PUBLIC GETTEXT_PACKAGE="${GETTEXT_PACKAGE}" LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")

# ayatana-indicator-printers-service
add_executable (ayatana-indicator-printers-service main.c)
target_link_libraries (ayatana-indicator-printers-service ayatanaindicatorprinterssservice ${SERVICE_LIBRARIES})
install (TARGETS ayatana-indicator-printers-service RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR}/${CMAKE_PROJECT_NAME})
