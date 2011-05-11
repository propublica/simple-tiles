OPTIMIZATION?=-O3
DEBUG ?= -g -rdynamic -ggdb
CFLAGS ?= -std=c99 -pedantic $(OPTIMIZATION) -Wall -W $(ARCH) $(DEBUG)
LIBS = -L/Library/Frameworks/GDAL.framework/unix/lib/ -lGDAL `pkg-config --cflags --libs cairo`
OBJ = map.o list.o bounds.o point.o style.o

all: $(OBJ)

bounds.o: bounds.c bounds.h point.h
list.o: list.c list.h
map.o: map.c map.h style.h list.h bounds.h point.h
point.o: point.c point.h
style.o: style.c map.h style.h list.h bounds.h point.h

dep:
	@$(CC) -MM *.c


