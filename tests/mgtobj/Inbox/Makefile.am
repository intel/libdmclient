libdir = $(MOBJS_DIR)

lib_LTLIBRARIES = moinbox.la

moinbox_la_SOURCES = plugin_inbox.c

moinbox_la_CPPFLAGS = -Wall -Wextra -Wcast-align \
                         -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement \
                         -I $(top_srcdir)/dmcore/include

moinbox_la_LDFLAGS = -avoid-version -module -shared -export-dynamic
