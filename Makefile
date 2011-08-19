all:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

test memcheck: all
	$(MAKE) install
	cd $@ && $(MAKE) $@

clean:
	rm -rf bin build test/*.o src/*.o test/runner test/api test/benchmark

lint:
	CC=scan-build $(MAKE) $@

.PHONY: all test clean install lint
