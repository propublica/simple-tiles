all:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

test run_api memcheck test-all: all
	$(MAKE) install
	cd test && $(MAKE) $@

clean:
	rm -rf bin build test/*.o src/*.o src/*.lo src/.libs test/runner test/api test/benchmark

lint:
	CC=scan-build $(MAKE)

docco:
	perl tools/rename.pl 's/\.h/_h.h/' src/*.h
	docco src/*.h src/*.c
	perl tools/rename.pl 's/_h.h/.h/' src/*.h

.PHONY: all test clean install lint
