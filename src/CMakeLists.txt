# libayatana-printersmenu.so
add_library (ayatana-printersmenu SHARED
    indicator-printers.c
    indicator-printers.h
    indicator-menu-item.c
    indicator-menu-item.h
    dbus-names.h)
target_include_directories (ayatana-printersmenu PUBLIC ${SERVICE_INCLUDE_DIRS})
target_compile_definitions (ayatana-printersmenu PUBLIC GETTEXT_PACKAGE="${GETTEXT_PACKAGE}" PACKAGE_NAME="${PACKAGE}")
install (TARGETS ayatana-printersmenu DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/ayatana-indicators3/7/")

# cups-notifier.h
# cups-notifier.c
include (GdbusCodegen)
add_gdbus_codegen_with_namespace (CUPS_NOTIFIER cups-notifier org.cups.cupsd Cups "${CMAKE_CURRENT_SOURCE_DIR}/org.cups.cupsd.Notifier.xml")

# ayatana-indicator-printers-service
add_executable (ayatana-indicator-printers-service
    indicator-printers-service.c
    indicator-printers-menu.c
    indicator-printers-menu.h
    indicator-printer-state-notifier.c
    indicator-printer-state-notifier.h
    spawn-printer-settings.c
    spawn-printer-settings.h
    dbus-names.h
    ${CUPS_NOTIFIER})
target_include_directories (ayatana-indicator-printers-service PUBLIC ${SERVICE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries (ayatana-indicator-printers-service ${SERVICE_LIBRARIES})
target_compile_definitions (ayatana-indicator-printers-service PUBLIC GETTEXT_PACKAGE="${GETTEXT_PACKAGE}" LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")
install (TARGETS ayatana-indicator-printers-service RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR}/${CMAKE_PROJECT_NAME})
