ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -I./ \
            $(LOCPLA_CFLAGS) \
            $(GPSUTILS_CFLAGS) \
            -I./data-items/ \
            -I./data-items/common \
            -I./observer \
            -I$(WORKSPACE)/gps-noship/flp \
            -D__func__=__PRETTY_FUNCTION__ \
            -fno-short-enums \
            -std=c++11

libloc_core_la_h_sources = \
           LocApiBase.h \
           LocAdapterBase.h \
           ContextBase.h \
           LocContext.h \
           LBSProxyBase.h \
           loc_core_log.h \
           LocAdapterProxyBase.h \
           EngineHubProxyBase.h \
           LocGlinkBase.h \
           data-items/DataItemId.h \
           data-items/IDataItemCore.h \
           data-items/DataItemConcreteTypes.h \
           data-items/DataItemsFactoryProxy.h \
           observer/IDataItemObserver.h \
           observer/IDataItemSubscription.h \
           observer/IFrameworkActionReq.h \
           observer/IOsObserver.h \
           SystemStatusOsObserver.h \
           SystemStatus.h

libloc_core_la_c_sources = \
           LocApiBase.cpp \
           LocAdapterBase.cpp \
           ContextBase.cpp \
           LocContext.cpp \
           loc_core_log.cpp \
           data-items/DataItemsFactoryProxy.cpp \
           data-items/DataItemConcreteTypes.cpp \
           SystemStatusOsObserver.cpp \
           SystemStatus.cpp

if USE_EXTERNAL_AP
AM_CFLAGS += -DFEATURE_EXTERNAL_AP
endif
if PTP_SUPPORTED
AM_CFLAGS += -DPTP_SUPPORTED
AM_CFLAGS += -I$(WORKSPACE)/external/open-avb/lib/libgptp
endif

library_includedir = $(pkgincludedir)

library_include_HEADERS = $(libloc_core_la_h_sources)

libloc_core_la_SOURCES = $(libloc_core_la_c_sources)

if USE_GLIB
libloc_core_la_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libloc_core_la_LDFLAGS = -lstdc++ -Wl,-z,defs -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
libloc_core_la_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
libloc_core_la_CFLAGS = $(AM_CFLAGS)
libloc_core_la_LDFLAGS = -Wl,-z,defs -lpthread -shared -version-info 1:0:0
libloc_core_la_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

if USE_FEATURE_AUTOMOTIVE
AM_CFLAGS += -DFEATURE_AUTOMOTIVE
endif

libloc_core_la_LIBADD = -ldl $(GPSUTILS_LIBS)

if PTP_SUPPORTED
libloc_core_la_LIBADD += -lgptp
endif

#Create and Install libraries
lib_LTLIBRARIES = libloc_core.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = loc-core.pc
EXTRA_DIST = $(pkgconfig_DATA)
