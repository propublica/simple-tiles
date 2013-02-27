lib_LTLIBRARIES = libsimple_tiles.la
libsimple_tiles_la_SOURCES = bounds.c error.c init.c layer.c list.c map.c memory.c query.c style.c text.c user_data.c util.c

pkginclude_HEADERS = bounds.h error.h init.h layer.h list.h map.h memory.h query.h simple_tiles.h style.h text.h types.h user_data.h util.h

libsimple_tiles_la_CFLAGS=${pangocairo_CFLAGS} ${GDAL_CFLAGS}
libsimple_tiles_la_LDFLAGS=-lm ${pangocairo_LIBS} ${GDAL_LDFLAGS}

EXTRA_DIST = simple-tiles.pc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = simple-tiles.pc
