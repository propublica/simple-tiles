all:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

test: all
	$(MAKE) install
	cd $@ && $(MAKE) $@

clean:
	rm -rf bin build test/*.o src/*.o src/*.dylib src/*.a test/*.png test/runner \
		test/api test/benchmark

.PHONY: all test clean install
