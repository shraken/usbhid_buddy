#!/usr/bin/env python
import platform
import sys
import os
from distutils.core import setup, Extension

if platform.system() == 'Windows':
    hidapi_src_path = '../../../hidapi/windows/hid.c'
    lib_list = ['setupapi']
elif platform.system() == 'Linux':
    hidapi_src_path = '../../../hidapi/linux/hid.c'
    lib_list = ['udev']
elif platform.system() == 'Darwin':
    hidapi_src_path = '../../../hidapi/mac/hid.c'
    lib_list = None

    os.environ['LDFLAGS'] = '-framework IOKit -framework CoreFoundation'
else:
    print 'ERROR: operating system %s not supported' % platform.platform()
    sys.exit()

buddy_module = Extension('_buddy',
                         sources=['buddy_wrap.c',
                                  '../../common/codec.c',
                                  '../../common/support.c',
                                  '../../host/buddy.c',
                                  hidapi_src_path],
                         include_dirs=['../../common',
                                       '../../host',
                                       '../../../hidapi/hidapi'],
                         libraries=lib_list)

setup (name='buddy',
       version='0.1',
       author='Wiggle Labs',
       description='Python binding for Buddy DAQ device',
       ext_modules=[buddy_module],
       py_modules=['buddy'])