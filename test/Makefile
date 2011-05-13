OPTIMIZATION?=-O3
INCLUDES = ../src/
CFLAGS?= -std=c99 -pedantic $(OPTIMIZATION) -Wall -W $(ARCH) -g -rdynamic -ggdb -I$(INCLUDES)

api.o: api.c ../src/simple_tiles.h ../src/map.h ../src/list.h \
  ../src/bounds.h ../src/point.h ../src/style.h
benchmark.o: benchmark.c
test_list.o: test_list.c ../src/list.h

../bin:
	mkdir -p ../bin

test_list: test_list.o ../bin
	$(CC) $(CFLAGS) test_list.o ../src/list.o -o ../bin/test_list

test: test_list
	../bin/$<

dep:
	@$(CC) -MM *.c -I$(INCLUDES)
