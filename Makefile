all:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

test: all
	$(MAKE) install
	cd $@ && $(MAKE) $@

clean:
	rm -rf bin build test/*.o src/*.o test/runner test/api

.PHONY: all test clean install
