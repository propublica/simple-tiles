all:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

api: all
	$(MAKE) install
	cd $@ && $(MAKE) $@

test: all
	$(MAKE) install
	cd $@ && $(MAKE) $@

clean:
	rm -rf bin build test/*.o src/*.o

.PHONY: all test clean install
