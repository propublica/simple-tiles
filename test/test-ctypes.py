# Make sure ctypes can find simple tiles
import ctypes
from ctypes.util import find_library

library = ctypes.CDLL(find_library("simple-tiles"))
library.simplet_map_new()
