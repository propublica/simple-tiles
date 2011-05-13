OPTIMIZATION?=-O0
DEBUG ?= -g -rdynamic -ggdb
CFLAGS ?= -std=c99 -pedantic $(OPTIMIZATION) -Wall -W $(ARCH)
LIBS = -L/Library/Frameworks/GDAL.framework/unix/lib/ -lGDAL `pkg-config --cflags --libs cairo`
OBJ = list.o bounds.o point.o style.o map.o util.o

all: $(OBJ)

api.o: api.c simple_tiles.h map.h list.h bounds.h point.h style.h
bounds.o: bounds.c bounds.h point.h
list.o: list.c list.h
map.o: map.c map.h list.h bounds.h point.h style.h util.h
point.o: point.c point.h
style.o: style.c map.h list.h bounds.h point.h style.h util.h
util.o: util.c util.h


api: api.o $(OBJ)
	$(CC)  -o api $(CFLAGS) $(LIBS) api.o $(OBJ)

.c.o:
	$(CC) -c $(CFLAGS) $(DEBUG) $(COMPILE_TIME) $<

dep:
	@$(CC) -MM *.c


