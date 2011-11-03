bin_PROGRAMS = testdmclient
testdmclient_SOURCES = main.c
testdmclient_CPPFLAGS = -I $(top_srcdir)/dmcore/include \
			$(GLIB_CFLAGS) $(LIBSOUP_CFLAGS)
testdmclient_LDADD = $(top_builddir)/dmcore/libomadmclient.la \
		      $(GLIB_LIBS) $(LIBSOUP_LIBS)
