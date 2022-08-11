plugin_LTLIBRARIES = libgstsdrvfilter.la

libgstsdrvfilter_la_SOURCES = \
	gstdrmutils.c \
	gstg2dapi.c \
	gstsdrvfilter.c \
	gstsdrvfilterpool.c

libgstsdrvfilter_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) \
	$(GST_CFLAGS) \
	$(DRM_SDRVFILTER_CFLAGS) \
	$(SDRVFILTER_CFLAGS) \
	$(GST_ALLOCATORS_CFLAGS)
libgstsdrvfilter_la_LIBADD = $(GST_PLUGINS_BASE_LIBS)\
	$(GST_LIBS) \
	$(GST_BASE_LIBS) \
	$(GST_ALLOCATORS_LIBS) \
	$(DRM_SDRVFILTER_LIBS) \
	$(GST_VIDEO_LIBS)

libgstsdrvfilter_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) \
	$(DRM_SDRVFILTER_LDFLAGS) \
	$(SDRVFILTER_LDFLAGS) \
	$(GST_ALLOCATORS_LDFLAGS)

noinst_HEADERS = \
	gstsdrvfilterpool.h \
	gstdrmutils.h \
	sdrv_types.h \
	gstg2dapi.h \
	gstsdrvfilterpool.h
