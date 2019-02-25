
cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED libbuddydir)
   # project based build
   set(libbuddydir ../../../../libbuddy)
   set(python_swig_path .)
else()
   # default, non-project based build
   set(python_swig_path ${libbuddydir}/bindings/python)
endif()

if(NOT DEFINED libdir)
   # default, non-project based build
   set(libdir ../../../..)
endif()

#message(STATUS "using libbuddydir = ${libbuddydir}")
#message(STATUS "using libdir = ${libdir}")
##message(STATUS "using python_swig_path = ${python_swig_path}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")

# generate coverage report
add_custom_command(OUTPUT swig_gen
   COMMAND swig -outcurrentdir -I${libbuddydir}/common -I${libbuddydir}/host -python ${libbuddydir}/bindings/python/buddy.i
)

add_custom_target(
   build_python_bindings ALL
   COMMAND python ${libbuddydir}/bindings/python/setup.py --libpath ${libdir} --swigpath ${CMAKE_CURRENT_BINARY_DIR} build install
   DEPENDS swig_gen
)