#!/usr/bin/env python
import platform
import sys
import os
from distutils.core import setup, Extension

if platform.system() == 'Windows':
    print 'windows building'
    hidapi_src_path = '../../../hidapi/windows/hid.c'
    lib_list = ['setupapi']
elif platform.system() == 'Linux':
    print 'linux building'
    hidapi_src_path = '../../../hidapi/linux/hid.c'
    lib_list = ['udev']
elif platform.system() == 'Darwin':
    print 'macos building'
    hidapi_src_path = '../../../hidapi/mac/hid.c'
    lib_list = None

    os.environ['LDFLAGS'] = '-framework IOKit -framework CoreFoundation'
else:
    print 'ERROR: operating system %s not supported' % platform.platform()
    sys.exit()

buddy_module = Extension('_buddy',
                         sources=['usbhid_buddy_wrap.c',
                                  '../../usbhid_buddy.c',
                                  '../../support.c',
                                  '../../buddy.c',
                                  '../../utility.c',
                                  hidapi_src_path],
                         include_dirs=['../../',
                                       '../../../hidapi/hidapi'],
                         libraries=lib_list)

setup (name='buddy',
       version='0.1',
       author='Wiggle Labs',
       description='Python binding for USBHID Buddy DAQ device',
       ext_modules=[buddy_module],
       py_modules=['buddy'])