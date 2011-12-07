ifeq ($(shell uname -s),Darwin)
  DYLIBNAME?=libsimple-tiles.dylib
  DYLIB_MAKE?=libtool -dynamic -o $(DYLIBNAME) $(LDLIBS) $(DEBUG) - $(OBJ)
  STLIBNAME?=libsimple-tiles.a
  STLIB_MAKE?=libtool -static -o $(STLIBNAME) - $(OBJ)
	INSTALL?=cp -a
else
	DEFINES = -D_BSD_SOURCE -D_GNU_SOURCE
	CC = libtool --mode=compile gcc
	DYLIBNAME?=libsimple-tiles.la
	DYLIB_MAKE?=libtool --mode=link gcc -rpath /usr/local/lib -o $(DYLIBNAME) $(LDFLAGS) $(DEBUG) $(OBJ:.o=.lo)
	INSTALL?=libtool --mode=install install -c
	AFTER = ldconfig
endif

OPTIMIZATION ?= -O3
DEBUG ?= -g -ggdb
CFLAGS ?= -fPIC -std=c99 $(OPTIMIZATION) $(DEFINES) $(DEBUG) -Wall -Werror -Wextra -Wwrite-strings $(ARCH) \
  $(shell pkg-config --cflags pangocairo) \
  $(shell gdal-config --cflags)
LDLIBS = -lm $(shell pkg-config --libs pangocairo) \
  $(shell gdal-config --libs)
OBJ = list.o bounds.o style.o map.o util.o filter.o layer.o error.o init.o text.o
PKG_CF = simple-tiles.pc


all: $(OBJ)

$(DYLIBNAME): $(OBJ)
	$(DYLIB_MAKE)

$(STLIBNAME): $(OBJ)
	$(STLIB_MAKE)

PREFIX?= /usr/local
INSTALL_INC= $(PREFIX)/include/simple-tiles/
INSTALL_LIB= $(PREFIX)/lib
INSTALL_PKG= $(INSTALL_LIB)/pkgconfig/

install: $(DYLIBNAME) $(STLIBNAME)
	mkdir -p $(INSTALL_INC) $(INSTALL_LIB) $(INSTALL_PKG)
	cp *.h $(INSTALL_INC)
	$(INSTALL) $(DYLIBNAME) $(STLIBNAME) $(INSTALL_LIB)
	cp $(PKG_CF) $(INSTALL_PKG)
	$(AFTER)

bounds.o: bounds.c bounds.h types.h
error.o: error.c error.h types.h
filter.o: filter.c style.h types.h list.h filter.h map.h text.h util.h \
  bounds.h
init.o: init.c error.h types.h
layer.o: layer.c layer.h types.h text.h list.h filter.h map.h style.h \
  util.h
list.o: list.c list.h types.h
map.o: map.c init.h error.h types.h map.h layer.h text.h list.h filter.h \
  style.h util.h bounds.h
style.o: style.c map.h types.h style.h list.h util.h
text.o: text.c text.h types.h list.h style.h util.h bounds.h
util.o: util.c util.h


dep:
	@$(CC) -MM *.c

.PHONY: all dep dynamic static

