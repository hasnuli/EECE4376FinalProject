from distutils.core
import setup, Extension
import numpy.distutils.misc_util

setup
(
      ext_modules=[Extension("_retrieveNote", ["_retrieveNote.c", "retrieveNote.c"])],
      ext_modules=[Extension("_returnState", ["_returnState.c", "returnState.c"])]
      include_dirs=numpy.distutils.misc_util.get_numpy_include_dirs(),
)
