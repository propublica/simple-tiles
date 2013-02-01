CFLAGS ?= -fPIC -std=c99 $(OPTIMIZATION) $(DEFINES) $(DEBUG) -Wall -Werror -Wextra -Wwrite-strings $(ARCH) \
  $(shell pkg-config --cflags pangocairo) \
  $(shell gdal-config --cflags)
LDLIBS = -lm $(shell pkg-config --libs pangocairo) \
  $(shell gdal-config --libs)


 test_ogr: test_ogr.c
