
AC_INIT(indicator-printers, 0.1.2)

AC_PREREQ(2.53)
AM_INIT_AUTOMAKE([])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AM_SILENT_RULES([yes])

IT_PROG_INTLTOOL([0.35.0])
GETTEXT_PACKAGE=indicator-printers
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [gettext package])
AM_GLIB_GNU_GETTEXT

AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    data/Makefile
    test/Makefile
    po/Makefile.in
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


with_localinstall="no"
AC_ARG_ENABLE(localinstall, AS_HELP_STRING([--enable-localinstall],
                                           [install all files locally (for distcheck)]),
              with_localinstall=$enableval,
              with_localinstall=no)

if test "x$with_localinstall" = "xyes"; then
	INDICATORDIR="${libdir}/indicators/2/"
	INDICATORICONSDIR="${datadir}/libindicate/icons/"
else
	INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator3-0.4`
	INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir indicator3-0.4`
fi
AC_SUBST(INDICATORDIR)
AC_SUBST(INDICATORICONSDIR)

###########################
# DBus Service Info
###########################

if test "x$with_localinstall" = "xyes"; then
	DBUSSERVICEDIR="${datadir}/dbus-1/services/"
else
	DBUSSERVICEDIR=`$PKG_CONFIG --variable=session_bus_services_dir dbus-1`
fi
AC_SUBST(DBUSSERVICEDIR)


AC_SUBST(AM_CFLAGS, "-Wall")

AC_OUTPUT

