OPTIMIZATION?=-O3
CFLAGS ?= -std=c99 -pedantic $(OPTIMIZATION) -rdynamic -Wall -W -Wwrite-strings $(ARCH) \
	$(shell pkg-config --cflags cairo simple-tiles) \
	$(shell gdal-config --cflags)
LDFLAGS = -lm $(shell pkg-config --libs cairo simple-tiles) \
	$(shell gdal-config --libs)

api: api.o
api.o: api.c

benchmark: benchmark.o
benchmark.o: benchmark.c

test_list.o: test_list.c ../src/list.h ../src/types.h
test_list: test_list.o

test: test_list api
	./test_list
	time ./api

dep:
	@$(CC) -MM *.c
