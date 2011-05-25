all:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

test: all
	$(MAKE) install
	cd $@ && $(MAKE) $@

clean:
	rm -rf bin build test/*.o src/*.o test/runner test/api data/armory2* data/ARMORY2* data/tl_2010* data/*.zip

.PHONY: all test clean install
