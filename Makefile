all:
	./waf

install:
	./waf install

test: all
	build/test/runner
	build/test/api
	build/test/benchmark

.PHONY: all install test