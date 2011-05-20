OPTIMIZATION?=-O3
DEBUG ?= -g -ggdb
CFLAGS ?= -std=c99 -pedantic $(OPTIMIZATION) -rdynamic -fPIC -Wall -W -Wwrite-strings $(ARCH) \
	$(shell pkg-config --cflags cairo) \
	$(shell gdal-config --cflags)
LDFLAGS = -lm $(shell pkg-config --libs cairo) \
	$(shell gdal-config --libs)
OBJ = list.o bounds.o point.o style.o map.o util.o rule.o layer.o
PKG_CF = simple-tiles.pc

ifeq ($(shell uname -s),Darwin)
  DYLIBNAME?=libsimple-tiles.dylib
  DYLIB_MAKE?=libtool -dynamic -o ${DYLIBNAME} ${LDFLAGS} ${DEBUG} - ${OBJ}
  STLIBNAME?=libsimple-tiles.a
  STLIB_MAKE?=libtool -static -o ${STLIBNAME} - ${OBJ}
else
  DYLIBNAME?=libsimple-tiles.so
  DYLIB_MAKE?=gcc -shared -Wl,-soname,${DYLIBNAME} -o ${DYLIBNAME} ${LDFLAGS} ${DEBUG} ${OBJ}
  STLIBNAME?=libsimple-tiles.a
  STLIB_MAKE?=ar rcs ${STLIBNAME} ${OBJ}
endif

all: $(OBJ)

${DYLIBNAME}: ${OBJ}
	${DYLIB_MAKE}

${STLIBNAME}: ${OBJ}
	${STLIB_MAKE}

PREFIX?= /usr/local
INSTALL_INC= $(PREFIX)/include/simple-tiles
INSTALL_LIB= $(PREFIX)/lib
INSTALL_PKG= $(INSTALL_LIB)/pkgconfig
INSTALL= cp -a

install: ${DYLIBNAME} ${STLIBNAME}
	mkdir -p $(INSTALL_INC) $(INSTALL_LIB)
	$(INSTALL) *.h $(INSTALL_INC)
	$(INSTALL) ${DYLIBNAME} ${STLIBNAME} $(INSTALL_LIB)
	$(INSTALL) $(PKG_CF) $(INSTALL_PKG)

bounds.o: bounds.c bounds.h types.h point.h
layer.o: layer.c layer.h types.h rule.h list.h map.h bounds.h point.h \
  style.h
list.o: list.c list.h types.h
map.o: map.c map.h types.h list.h bounds.h point.h style.h layer.h rule.h \
  util.h
point.o: point.c point.h types.h
rule.o: rule.c style.h types.h list.h rule.h map.h bounds.h point.h \
  layer.h util.h
style.o: style.c map.h types.h list.h bounds.h point.h style.h layer.h \
  util.h
util.o: util.c util.h

dep:
	@$(CC) -MM *.c

.PHONY: dep dynamic static

