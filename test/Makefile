OPTIMIZATION?=-O0
CFLAGS ?= -std=c99 -pedantic $(OPTIMIZATION) -rdynamic -Wall -W -Wwrite-strings $(ARCH) \
	$(shell pkg-config --cflags cairo simple-tiles) \
	$(shell gdal-config --cflags)
LDFLAGS = -lm $(shell pkg-config --libs cairo simple-tiles) \
	$(shell gdal-config --libs)

api.o: api.c
benchmark.o: benchmark.c
test_list.o: test_list.c test.h
test_rule.o: test_rule.c test.h
test_style.o: test_style.c test.h

api: api.o
benchmark: benchmark.o
test_list: test_list.o
test_style: test_style.o
test_rule: test_rule.o

test: test_list test_style test_rule api
	./test_list
	./test_style
	./test_rule
	time ./api

dep:
	@$(CC) -MM *.c
