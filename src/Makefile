OPTIMIZATION ?= -O3
DEBUG ?= -g -ggdb
CFLAGS ?= -std=c99 $(OPTIMIZATION) -g -ggdb -rdynamic -fPIC -Wall -W -Wwrite-strings -pthread $(ARCH) \
  $(shell pkg-config --cflags cairo) \
  $(shell gdal-config --cflags)
LDFLAGS = -lm $(shell pkg-config --libs cairo) \
  $(shell gdal-config --libs)
OBJ = list.o bounds.o point.o style.o map.o util.o filter.o layer.o error.o
PKG_CF = simple-tiles.pc

ifeq ($(shell uname -s),Darwin)
  DYLIBNAME?=libsimple-tiles.dylib
  DYLIB_MAKE?=libtool -dynamic -o $(DYLIBNAME) $(LDFLAGS) $(DEBUG) - $(OBJ)
  STLIBNAME?=libsimple-tiles.a
  STLIB_MAKE?=libtool -static -o $(STLIBNAME) - $(OBJ)
else
  DYLIBNAME?=libsimple-tiles.so
  DYLIB_MAKE?=gcc -shared -Wl,-soname,$(DYLIBNAME) -o $(DYLIBNAME) $(LDFLAGS) $(DEBUG) $(OBJ)
  STLIBNAME?=libsimple-tiles.a
  STLIB_MAKE?=ar rcs $(STLIBNAME) $(OBJ)
	AFTER = ldconfig
endif

all: $(OBJ)

$(DYLIBNAME): $(OBJ)
	$(DYLIB_MAKE)

$(STLIBNAME): $(OBJ)
	$(STLIB_MAKE)

PREFIX?= /usr/local
INSTALL_INC= $(PREFIX)/include/simple-tiles/
INSTALL_LIB= $(PREFIX)/lib
INSTALL_PKG= $(INSTALL_LIB)/pkgconfig/
INSTALL= cp -a

install: $(DYLIBNAME) $(STLIBNAME)
	mkdir -p $(INSTALL_INC) $(INSTALL_LIB) $(INSTALL_PKG)
	$(INSTALL) *.h $(INSTALL_INC)
	$(INSTALL) $(DYLIBNAME) $(STLIBNAME) $(INSTALL_LIB)
	$(INSTALL) $(PKG_CF) $(INSTALL_PKG)
	$(AFTER)

bounds.o: bounds.c bounds.h types.h point.h
error.o: error.c error.h types.h
filter.o: filter.c style.h types.h list.h filter.h map.h bounds.h point.h \
  layer.h util.h
layer.o: layer.c layer.h types.h filter.h list.h map.h bounds.h point.h \
  style.h util.h
list.o: list.c list.h types.h
map.o: map.c map.h types.h list.h bounds.h point.h style.h layer.h \
  error.h filter.h util.h
point.o: point.c point.h types.h
style.o: style.c map.h types.h list.h bounds.h point.h style.h layer.h \
  util.h
util.o: util.c util.h

dep:
	@$(CC) -MM *.c

.PHONY: all dep

