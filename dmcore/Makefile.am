dmclient_src = dmclient/omadmclient.c \
               dmclient/callbacks.c \
               dmclient/codec.c \
               dmclient/package0.c \
               dmclient/credentials.c \
               dmclient/sml2tree.c \
               dmclient/utils.c

dmtree_src = dmtree/dmtree.c \
             dmtree/plugin_inbox.c \
             dmtree/plugin_root.c \
             dmtree/plugin_devdetails.c \
             dmtree/plugin_devinfo.c \
             dmtree/command_tl.c \
             dmtree/tl_session.c \
             dmtree/syncml_error.c \
             dmtree/dmsettings.c \
             dmtree/dmsettings_utils.c \
             dmtree/transaction.c \
             dmtree/dmsession.c \
             dmtree/dyn_buf.c \
             dmtree/log.c

lib_LTLIBRARIES = libomadmclient.la

libomadmclient_la_SOURCES = $(dmclient_src) \
                            $(dmtree_src)

libomadmclient_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                             -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                             -I $(srcdir)/include \
			                 -I $(top_srcdir)/externals/SyncMLRTK/src \
			                 -I $(top_srcdir)/externals/SyncMLRTK/src/sml/inc \
			                 -I $(top_srcdir)/externals/SyncMLRTK/src/sml/inc/win \
                             -I $(top_srcdir)/externals/SyncMLRTK/src/sml/mgr/inc \
                             $(LIBXML_CFLAGS)

libomadmclient_la_LIBADD = $(top_builddir)/externals/SyncMLRTK/libsyncml.la

libomadmclient_la_LDFLAGS = $(GNUTLS_LIBS) $(LIBXML_LIBS) $(SQLITE3_LIBS)
