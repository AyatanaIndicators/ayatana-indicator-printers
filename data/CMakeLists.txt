# ayatana-indicator-printers.service
pkg_check_modules (SYSTEMD systemd)

if (${SYSTEMD_FOUND})
    pkg_get_variable (SYSTEMD_USER_DIR systemd systemduserunitdir)
    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/ayatana-indicator-printers.service.in" "${CMAKE_CURRENT_BINARY_DIR}/ayatana-indicator-printers.service" @ONLY)
    install (FILES "${CMAKE_CURRENT_BINARY_DIR}/ayatana-indicator-printers.service" DESTINATION "${SYSTEMD_USER_DIR}")
endif ()

# ayatana-indicator-printers.desktop
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/ayatana-indicator-printers.desktop.in" "${CMAKE_CURRENT_BINARY_DIR}/ayatana-indicator-printers.desktop" @ONLY)
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/ayatana-indicator-printers.desktop" DESTINATION "/etc/xdg/autostart")

# org.ayatana.indicator.printers
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/org.ayatana.indicator.printers" DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/ayatana/indicators")
