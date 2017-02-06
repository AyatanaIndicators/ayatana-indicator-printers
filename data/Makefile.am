
BUILT_SOURCES=
CLEANFILES=
EXTRA_DIST=

#
# the systemd unit file
#

systemddir = $(SYSTEMD_USERDIR)
systemd_DATA = indicator-printers.service
systemd_in = $(systemd_DATA:.service=.service.in)
$(systemd_DATA): $(systemd_in)
	$(AM_V_GEN) $(SED) -e "s|\@pkglibexecdir\@|$(pkglibexecdir)|" $< > $@
BUILT_SOURCES += $(systemd_DATA)
EXTRA_DIST += $(systemd_in)
CLEANFILES += $(systemd_DATA)

#
# the systemd upstart override
#

upstart_overridedir = $(datadir)/upstart/systemd-session/upstart
upstart_override_DATA = indicator-printers.override
EXTRA_DIST += $(upstart_override_DATA)

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
