# Make sure ctypes can find simple tiles
import ctypes
from ctypes.util import find_library
try:
    library = ctypes.CDLL(find_library("simple-tiles"))
    library.simplet_map_new()
except:
    print "can't find the library for this test, you might want to run make install"
