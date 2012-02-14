
AC_INIT(indicator-printers, 0.1)

AC_PREREQ(2.53)
AM_INIT_AUTOMAKE([])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AM_SILENT_RULES([yes])

AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    data/Makefile
    test/Makefile
])

PKG_CHECK_MODULES(APPLET, gtk+-3.0 >= 3.0
                          indicator3-0.4 >= 0.2
                          dbusmenu-gtk3-0.4 >= 0.2)
PKG_CHECK_MODULES(SERVICE, gtk+-3.0 >= 3.0
                           indicator3-0.4 >= 0.2
                           dbusmenu-glib-0.4 >= 0.2)

AC_PATH_PROG(CUPS_CONFIG, cups-config, no)
if test "x$CUPS_CONFIG" = "xno"; then
    AC_MSG_ERROR([could not find cups-config])
fi
AC_CHECK_HEADER(cups/cups.h,,AC_MSG_ERROR([could not find cups.h]))
SERVICE_CFLAGS+=`$CUPS_CONFIG --cflags`
SERVICE_LIBS+=`$CUPS_CONFIG --libs`

INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator3-0.4`
INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir indicator3-0.4`

DBUSSERVICEDIR=`$PKG_CONFIG --variable=session_bus_services_dir dbus-1`

AC_SUBST(INDICATORDIR)
AC_SUBST(INDICATORICONDIR)
AC_SUBST(DBUSSERVICEDIR)

AC_SUBST(AM_CFLAGS, "-Wall")

AC_OUTPUT

