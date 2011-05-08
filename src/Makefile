OPTIMIZATION?=-O3
DEBUG ?= -g -rdynamic -ggdb
CFLAGS ?= -std=c99 -pedantic $(OPTIMIZATION) -Wall -W $(ARCH) $(DEBUG)

OBJ = map.o list.o bounds.o point.o
bounds.o: bounds.c bounds.h point.h
list.o: list.c list.h
map.o: map.c
point.o: point.c point.h

all: ../build/libsimple_tiles.a

../build/libsimple_tiles.a: $(OBJ)
	mkdir -p ../build
	$(AR) rcs $@ $^

dep:
	@$(CC) -MM *.c


