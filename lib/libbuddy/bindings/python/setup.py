#!/usr/bin/env python
import platform
import sys
import os
from distutils.core import setup, Extension

libpath = ''
swigpath = ''

LIBPATH_ARG_LABEL = '--libpath'             # path to `libbuddy`, `hidapi` and other libraries
SWIGPATH_ARG_LABEL = '--swigpath'           # path to generated swig binding files

print(sys.argv)
# todo: clean up this hack.  we do this to support a global project cmake build and
# a *just* binding building
if LIBPATH_ARG_LABEL in sys.argv:
    index = sys.argv.index(LIBPATH_ARG_LABEL)
    sys.argv.pop(index)
    libpath = sys.argv.pop(index)

if SWIGPATH_ARG_LABEL in sys.argv:
    index = sys.argv.index(SWIGPATH_ARG_LABEL)
    sys.argv.pop(index)
    swigpath = sys.argv.pop(index)

print('using libpath = {}'.format(libpath))
print('using swigpath = {}'.format(swigpath))

if platform.system() == 'Windows':
    hidapi_src_path = '{}/hidapi/windows/hid.c'.format(libpath)
    lib_list = ['setupapi']
elif platform.system() == 'Linux':
    hidapi_src_path = '{}/hidapi/linux/hid.c'.format(libpath)
    lib_list = ['udev']
elif platform.system() == 'Darwin':
    hidapi_src_path = '{}/hidapi/mac/hid.c'.format(libpath)
    lib_list = None

    os.environ['LDFLAGS'] = '-framework IOKit -framework CoreFoundation'
else:
    print 'ERROR: operating system %s not supported' % platform.platform()
    sys.exit()

buddy_module = Extension('_buddy',
                         sources=['{}/buddy_wrap.c'.format(swigpath),
                                  '{}/libbuddy/common/codec.c'.format(libpath),
                                  '{}/libbuddy/common/support.c'.format(libpath),
                                  '{}/libbuddy/host/buddy.c'.format(libpath),
                                  hidapi_src_path],
                         include_dirs=['{}/libbuddy/common'.format(libpath),
                                       '{}/libbuddy/host'.format(libpath),
                                       '{}/hidapi/hidapi'.format(libpath)],
                         libraries=lib_list)

setup (name='buddy',
       version='0.1',
       author='Wiggle Labs',
       description='Python binding for Buddy DAQ device',
       ext_modules=[buddy_module],
       py_modules=['buddy'])