OPTIMIZATION?=-O0
CFLAGS ?= -std=c99 -pedantic $(OPTIMIZATION) -rdynamic -Wall -W -Wwrite-strings $(ARCH) \
	$(shell pkg-config --cflags cairo simple-tiles) \
	$(shell gdal-config --cflags)
LDFLAGS = -lm $(shell pkg-config --libs cairo simple-tiles) \
	$(shell gdal-config --libs)


OBJ = test_list.o test_style.o test_rule.o test_layer.o

api.o: api.c
benchmark.o: benchmark.c
runner.o: runner.c runner.h test.h
test_layer.o: test_layer.c test.h
test_list.o: test_list.c test.h
test_rule.o: test_rule.c test.h
test_style.o: test_style.c test.h

api: api.o
benchmark: benchmark.o
runner: runner.o $(OBJ)

test: runner api
	./runner
	time ./api

dep:
	@$(CC) -MM *.c
