bin_PROGRAMS = testdmclient
testdmclient_SOURCES = client/main.c
testdmclient_CPPFLAGS = -I $(top_srcdir)/dmcore/include \
			$(GLIB_CFLAGS) $(LIBSOUP_CFLAGS)
testdmclient_LDADD = $(top_builddir)/dmcore/libomadmclient.la \
		      $(GLIB_LIBS) $(LIBSOUP_LIBS)

libdir = $(MOBJS_DIR)
lib_LTLIBRARIES = modevdetails.la modevinfo.la moinbox.la moroot.la

modevdetails_la_SOURCES = mgtobj/DevDetail/plugin_devdetails.c
modevdetails_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                           -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                           -I $(top_srcdir)/dmcore/include
modevdetails_la_LDFLAGS = -avoid-version -module -shared -export-dynamic

modevinfo_la_SOURCES = mgtobj/DevInfo/plugin_devinfo.c
modevinfo_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                        -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                        -I $(top_srcdir)/dmcore/include
modevinfo_la_LDFLAGS = -avoid-version -module -shared -export-dynamic

moinbox_la_SOURCES = mgtobj/Inbox/plugin_inbox.c
moinbox_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                      -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                      -I $(top_srcdir)/dmcore/include
moinbox_la_LDFLAGS = -avoid-version -module -shared -export-dynamic

moroot_la_SOURCES = mgtobj/Root/plugin_root.c \
                    mgtobj/Root/dmsettings.c \
                    mgtobj/Root/dmsettings_utils.c
moroot_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                     -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                     -I $(top_srcdir)/dmcore/include
moroot_la_LDFLAGS = $(SQLITE3_LIBS) -avoid-version -module -shared -export-dynamic
