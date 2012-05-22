all:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

test run_api memcheck test-all: install
	cd test && $(MAKE) $@

clean:
	rm -rf bin build test/*.o src/*.o src/*.lo src/.libs test/runner test/api test/benchmark

lint:
	CC=scan-build $(MAKE)

publish:
	git checkout gh-pages
	git merge master
	git push
	git checkout master

.PHONY: all test clean install lint
