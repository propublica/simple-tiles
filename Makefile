all:
	./waf

install:
	./waf install

test:
	test/waf test

.PHONY: all install test