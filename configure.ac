AC_INIT(ayatana-indicator-printers, 22.2.0)

AC_PREREQ(2.53)
AM_INIT_AUTOMAKE([])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])

AC_DEFUN([AC_DEFINE_PATH], [
	test "x$prefix" = xNONE && prefix="$ac_default_prefix"
	test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
	ac_define_path=`eval echo [$]$2`
	ac_define_path=`eval echo [$]ac_define_path`
	$1="$ac_define_path"
	AC_SUBST($1)
	ifelse($3, ,
		AC_DEFINE_UNQUOTED($1, "$ac_define_path"),
		AC_DEFINE_UNQUOTED($1, "$ac_define_path", $3))
])

IT_PROG_INTLTOOL([0.35.0])
GETTEXT_PACKAGE=ayatana-indicator-printers
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [gettext package])
AC_DEFINE_PATH(LOCALEDIR, "${datadir}/locale", [locale directory])
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
                          ayatana-indicator3-0.4 >= 0.2
                          dbusmenu-gtk3-0.4 >= 0.2)
PKG_CHECK_MODULES(SERVICE, gtk+-3.0 >= 3.0
                           ayatana-indicator3-0.4 >= 0.2
                           dbusmenu-glib-0.4 >= 0.2)

AC_PATH_PROG(CUPS_CONFIG, cups-config, no)
if test "x$CUPS_CONFIG" = "xno"; then
    AC_MSG_ERROR([could not find cups-config])
fi
AC_CHECK_HEADER(cups/cups.h,,AC_MSG_ERROR([could not find cups.h]))
CUPS_CFLAGS=`$CUPS_CONFIG --cflags`
CUPS_LIBS=`$CUPS_CONFIG --libs`
SERVICE_CFLAGS="$SERVICE_CFLAGS $CUPS_CFLAGS"
SERVICE_LIBS="$SERVICE_LIBS $CUPS_LIBS"

#########################
# Check for systemd
#########################
PKG_CHECK_MODULES(SYSTEMD,  systemd,
    [has_systemd=yes],
    []
)
if test "x$has_systemd" = "xyes"; then
       SYSTEMD_USERDIR=`$PKG_CONFIG --variable=systemduserunitdir systemd`
       AC_SUBST(SYSTEMD_USERDIR)
       AC_DEFINE(HAVE_SYSTEMD, 1, [create ayatana-indicator-printers.service for systemd])
fi

INDICATORDIR=`$PKG_CONFIG --variable=indicatordir ayatana-indicator3-0.4`
INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir ayatana-indicator3-0.4`

AC_SUBST(INDICATORDIR)
AC_SUBST(INDICATORICONSDIR)

###########################
# DBus Service Info
###########################

DBUSSERVICEDIR=`$PKG_CONFIG --variable=session_bus_services_dir dbus-1`
AC_SUBST(DBUSSERVICEDIR)

###########################
# gcov coverage reporting
###########################

m4_include([m4/gcov.m4])
AC_TDD_GCOV
AM_CONDITIONAL([HAVE_GCOV], [test "x$ac_cv_check_gcov" = xyes])
AM_CONDITIONAL([HAVE_LCOV], [test "x$ac_cv_check_lcov" = xyes])
AM_CONDITIONAL([HAVE_GCOVR], [test "x$ac_cv_check_gcovr" = xyes])
AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)

AC_SUBST(AM_CFLAGS, "-Wall")

AC_OUTPUT
