ACLOCAL_AMFLAGS = -I ../aclocal

SUBDIRS = 

if SMPTESTS
SUBDIRS += smp01
SUBDIRS += smp02
SUBDIRS += smp03
SUBDIRS += smp05
SUBDIRS += smp07
SUBDIRS += smp08
SUBDIRS += smp09
SUBDIRS += smpatomic01
SUBDIRS += smplock01
SUBDIRS += smpmigration01
SUBDIRS += smpschedule01
SUBDIRS += smpsignal01
SUBDIRS += smpswitchextension01
SUBDIRS += smpunsupported01
if HAS_POSIX
SUBDIRS += smppsxsignal01
endif
endif

include $(top_srcdir)/../automake/subdirs.am
include $(top_srcdir)/../automake/local.am
