libdir = $(MOBJS_DIR)

lib_LTLIBRARIES = modevdetails.la

modevdetails_la_SOURCES = plugin_devdetails.c

modevdetails_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                             -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                             -I $(top_srcdir)/dmcore/include

modevdetails_la_LDFLAGS = -avoid-version -module -shared -export-dynamic
