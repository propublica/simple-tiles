all:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

test: all
	$(MAKE) install
	cd $@ && $(MAKE) $@

clean:
	rm -rf bin build test/*.o src/*.o

.PHONY: all test clean install
