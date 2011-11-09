libdir = $(MOBJS_DIR)

lib_LTLIBRARIES = moroot.la

moroot_la_SOURCES = plugin_root.c \
                       dmsettings.c \
                       dmsettings_utils.c

moroot_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                         -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                         -I $(top_srcdir)/dmcore/include

moroot_la_LDFLAGS = $(SQLITE3_LIBS) -avoid-version -module -shared -export-dynamic
