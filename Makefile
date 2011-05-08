all:
	cd src && $(MAKE) $@

test: all
	cd $@ && $(MAKE) $@

clean:
	rm -rf bin build test/*.o src/*.o

.PHONY: test clean benchmark
