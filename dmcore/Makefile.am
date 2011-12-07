lib_LTLIBRARIES = libomadmclient.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = omadmclient.pc

libomadmclient_la_includedir = $(includedir)/omadmclient
libomadmclient_la_include_HEADERS = include/omadmclient.h \
                                    include/omadmtree_mo.h \
                                    include/syncml_error.h

libomadmclient_la_SOURCES = src/omadmclient.c \
                            src/callbacks.c \
                            src/codec.c \
                            src/package0.c \
                            src/credentials.c \
                            src/sml2tree.c \
                            src/utils.c \
                            src/momgr.c \
                            src/dmtree.c \
                            src/defaultroot.c \
                            src/log.c

libomadmclient_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                             -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                             -I $(srcdir)/include \
			                 -I $(top_srcdir)/externals/SyncMLRTK/src \
			                 -I $(top_srcdir)/externals/SyncMLRTK/src/sml/inc \
			                 -I $(top_srcdir)/externals/SyncMLRTK/src/sml/inc/win \
                             -I $(top_srcdir)/externals/SyncMLRTK/src/sml/mgr/inc \
                             $(LIBXML_CFLAGS)

libomadmclient_la_LIBADD = $(top_builddir)/externals/SyncMLRTK/libsyncml.a

libomadmclient_la_LDFLAGS = $(GNUTLS_LIBS) $(LIBXML_LIBS) -export-symbols-regex '^omadmclient_'
