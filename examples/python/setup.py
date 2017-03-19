from distutils.core import setup, Extension

c_ext = Extension('_buddy', 
                    sources = ["_buddy.c", 
                    		   "../usbhid_buddy.c", 
                    		   "../support.c", 
                    		   "../../common/buddy.c", 
                    		   "../hidapi/windows/hid.c"],
                    include_dirs=["../", "../hidapi/hidapi/", "../../common"],
                    libraries=["setupapi"],
                    library_dirs=["../hidapi/windows/Release"])

#c_ext = Extension("_buddy", ["_chi2.c", "chi2.c"])

setup(
    ext_modules=[c_ext]
)