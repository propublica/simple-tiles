check_PROGRAMS = runner api benchmark
runner_SOURCES = runner.c test_list.c test_style.c test_query.c test_layer.c test_map.c test_integration.c test_bounds.c
api_SOURCES = api.c
benchmark_SOURCES = benchmark.c

AM_CFLAGS=${pangocairo_CFLAGS} ${GDAL_CFLAGS}
AM_LDFLAGS=-L../src -lm ${pangocairo_LDFLAGS} ${GDAL_LDFLAGS} -lsimple_tiles

data:
	cd ../data && make && cd ../test

run_tests: data runner
	./runner
	python test-ctypes.py # test to see if ctypes works for bindings

run_benchmark: data benchmark
	./benchmark

run_api: api
	time ./api

GLIB_OPTS = G_SLICE=always-malloc G_DEBUG=gc-friendly
MEM_OPTS = --leak-resolution=high --leak-check=full --suppressions=./suppressions.txt

memcheck: api runner benchmark
	$(GLIB_OPTS) valgrind $(MEM_OPTS) ./api

test: check data run_tests run_api run_benchmark

dep:
	@$(CC) -MM *.c

.PHONY: dep test run_benchmark run_tests run_api memcheck
