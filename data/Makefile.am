
BUILT_SOURCES=
CLEANFILES=
EXTRA_DIST=

SUBDIRS = upstart

#
# the upstart job file
#

upstart_jobsdir = $(datadir)/upstart/sessions
upstart_jobs_DATA = indicator-printers.conf
upstart_jobs_in = $(upstart_jobs_DATA:.conf=.conf.in)
$(upstart_jobs_DATA): $(upstart_jobs_in)
	$(AM_V_GEN) $(SED) -e "s|\@pkglibexecdir\@|$(pkglibexecdir)|" $< > $@
BUILT_SOURCES += $(upstart_jobs_DATA)
CLEANFILES += $(upstart_jobs_DATA)
EXTRA_DIST += $(upstart_jobs_in)

#
# the xdg autostart job file
#

xdg_autostartdir = /etc/xdg/autostart
xdg_autostart_DATA = indicator-printers.desktop
xdg_autostart_in = $(xdg_autostart_DATA:.desktop=.desktop.in)
$(xdg_autostart_DATA): $(xdg_autostart_in)
	$(AM_V_GEN) $(SED) -e "s|\@pkglibexecdir\@|$(pkglibexecdir)|" $< > $@
BUILT_SOURCES += $(xdg_autostart_DATA)
CLEANFILES += $(xdg_autostart_DATA)
EXTRA_DIST += $(xdg_autostart_in)


