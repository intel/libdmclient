bin_PROGRAMS = testdmclient
testdmclient_SOURCES = client/main.c \
                       client/test_plugin.c
testdmclient_CPPFLAGS = -I $(top_srcdir)/dmcore/include \
			$(GLIB_CFLAGS) $(LIBCURL_CFLAGS)
testdmclient_LDADD = $(top_builddir)/dmcore/libdmclient.la \
		      $(GLIB_LIBS) $(LIBCURL_LIBS)

libdir = $(MO_INSTALL_DIR)

install-exec-hook:
	find $(libdir) -type f -name \*.la -delete

lib_LTLIBRARIES = modevdetails.la modevinfo.la moinbox.la mostorage.la modmacc.la

modevdetails_la_SOURCES = mgtobj/DevDetail/plugin_devdetails.c mgtobj/utils/static_mo_util.c
modevdetails_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                           -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                           -I $(top_srcdir)/dmcore/include -I $(top_srcdir)/tests/mgtobj/utils
modevdetails_la_LDFLAGS = -avoid-version -module -shared -export-dynamic -export-symbols-regex '^omadm_get_mo_interface'

modevinfo_la_SOURCES = mgtobj/DevInfo/plugin_devinfo.c mgtobj/utils/static_mo_util.c
modevinfo_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                        -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                        -I $(top_srcdir)/dmcore/include -I $(top_srcdir)/tests/mgtobj/utils
modevinfo_la_LDFLAGS = -avoid-version -module -shared -export-dynamic -export-symbols-regex '^omadm_get_mo_interface'

modmacc_la_SOURCES = mgtobj/DmAcc/plugin_dmacc.c mgtobj/utils/static_mo_util.c
modmacc_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                      -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                      -I $(top_srcdir)/dmcore/include -I $(top_srcdir)/tests/mgtobj/utils
modmacc_la_LDFLAGS = -avoid-version -module -shared -export-dynamic -export-symbols-regex '^omadm_get_mo_interface'

moinbox_la_SOURCES = mgtobj/Inbox/plugin_inbox.c
moinbox_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                      -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                      -I $(top_srcdir)/dmcore/include
moinbox_la_LDFLAGS = -avoid-version -module -shared -export-dynamic -export-symbols-regex '^omadm_get_mo_interface'

mostorage_la_SOURCES = mgtobj/Storage/plugin_storage.c \
                       mgtobj/Storage/dyn_buf.c \
                       mgtobj/Storage/dmsettings.c \
                       mgtobj/Storage/dmsettings_utils.c
mostorage_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                        -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                        -I $(top_srcdir)/dmcore/include
mostorage_la_LDFLAGS = $(SQLITE3_LIBS) -avoid-version -module -shared -export-dynamic -export-symbols-regex '^omadm_get_mo_interface'
