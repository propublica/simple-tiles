all:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

test run-api memcheck test-all: all
	$(MAKE) install
	cd test && $(MAKE) $@

clean:
	rm -rf bin build test/*.o src/*.o test/runner test/api test/benchmark

lint:
	CC=scan-build $(MAKE)

.PHONY: all test clean install lint
