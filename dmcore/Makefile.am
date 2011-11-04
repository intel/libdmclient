dmclient_src = dmclient/omadmclient.c \
               dmclient/callbacks.c \
               dmclient/codec.c \
               dmclient/package0.c \
               dmclient/credentials.c \
               dmclient/sml2tree.c \
               dmclient/utils.c

dmtree_src = dmtree/dmtree.c \
             dmtree/transaction.c \
             dmtree/dmsession.c \
             dmtree/dyn_buf.c \
             dmtree/log.c

mo_root_src = dmtree/mgtobj/Root/plugin_root.c \
              dmtree/mgtobj/Root/dmsettings.c \
              dmtree/mgtobj/Root/dmsettings_utils.c

mo_inbox_src = dmtree/mgtobj/Inbox/plugin_inbox.c

mo_devdetails_src = dmtree/mgtobj/DevDetails/plugin_devdetails.c

mo_devinfo_src = dmtree/mgtobj/DevInfo/plugin_devinfo.c

lib_LTLIBRARIES = libomadmclient.la

libomadmclient_la_SOURCES = $(dmclient_src) \
                            $(dmtree_src) \
                            $(mo_root_src) \
                            $(mo_inbox_src) \
                            $(mo_devdetails_src) \
                            $(mo_devinfo_src)

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
