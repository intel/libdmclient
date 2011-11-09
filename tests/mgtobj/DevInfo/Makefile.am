libdir = $(MOBJS_DIR)

lib_LTLIBRARIES = modevinfo.la

modevinfo_la_SOURCES = plugin_devinfo.c

modevinfo_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                           -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                           -I $(top_srcdir)/dmcore/include

modevinfo_la_LDFLAGS = -avoid-version -module -shared -export-dynamic
