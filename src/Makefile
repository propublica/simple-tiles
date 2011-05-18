OPTIMIZATION?=-O3
DEBUG ?= -g -rdynamic -ggdb
CFLAGS ?= -std=c99 -pedantic $(OPTIMIZATION) -Wall -W $(ARCH) \
	$(shell pkg-config --cflags cairo) \
	$(shell gdal-config --cflags)
LDFLAGS = $(shell pkg-config --libs cairo) \
	$(shell gdal-config --libs)
OBJ = list.o bounds.o point.o style.o map.o util.o rule.o

all: $(OBJ)

api.o: api.c simple_tiles.h map.h types.h list.h bounds.h point.h style.h
bounds.o: bounds.c bounds.h types.h point.h
list.o: list.c list.h types.h
map.o: map.c map.h types.h list.h bounds.h point.h style.h rule.h util.h
point.o: point.c point.h types.h
rule.o: rule.c style.h types.h list.h rule.h map.h bounds.h point.h \
  util.h
style.o: style.c map.h types.h list.h bounds.h point.h style.h util.h
util.o: util.c util.h

api: api.o $(OBJ)

dep:
	@$(CC) -MM *.c

.PHONY: dep

