OPTIMIZATION?=-O3
CFLAGS ?= -std=c99 -pedantic $(OPTIMIZATION) -g -ggdb -Wall -W -Wwrite-strings -I/usr/local/include\
	$(ARCH) $(shell pkg-config --cflags simple-tiles pangocairo) \
	$(shell gdal-config --cflags)
LDLIBS = -L/usr/local/lib -lm $(shell pkg-config --libs simple-tiles pangocairo) \
	$(shell gdal-config --libs)
OBJ = test_list.o test_style.o test_filter.o test_layer.o test_map.o test_integration.o test_bounds.o

api.o: api.c
benchmark.o: benchmark.c
runner.o: runner.c runner.h test.h
test_bounds.o: test_bounds.c
test_filter.o: test_filter.c test.h
test_integration.o: test_integration.c test.h
test_layer.o: test_layer.c test.h
test_list.o: test_list.c test.h
test_map.o: test_map.c test.h
test_style.o: test_style.c test.h

api: api.o
benchmark: benchmark.o
runner: runner.o $(OBJ)

data:
	cd ../data && make && cd ../test

run_tests: data runner
	./runner

run_benchmark: data benchmark
	./benchmark

run_api: api
	time ./api


GLIB_OPTS = G_SLICE=always-malloc G_DEBUG=gc-friendly
MEM_OPTS = --leak-resolution=high --leak-check=full --suppressions=./suppressions.txt

memcheck: api runner benchmark
	$(GLIB_OPTS) valgrind $(MEM_OPTS) ./api

test: data run_tests run_api run_benchmark

dep:
	@$(CC) -MM *.c

.PHONY: dep test run_benchmark run_tests run_api memcheck
