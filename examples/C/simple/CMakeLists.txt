project( simple )
cmake_minimum_required(VERSION 2.8)

# header files
set (MAIN_INCLUDE ${PROJECT_SOURCE_DIR}) 
set (LIBBUDDY_COMMON_INCLUDE ${PROJECT_SOURCE_DIR}/../../../lib/libbuddy/common)
set (LIBBUDDY_HOST_INCLUDE ${PROJECT_SOURCE_DIR}/../../../lib/libbuddy/host)
set (HIDAPI_INCLUDE ${PROJECT_SOURCE_DIR}/../../../lib/hidapi/hidapi) 
set (BUDDY_INCLUDE_DIRS 
        ${MAIN_INCLUDE} 
        ${LIBBUDDY_COMMON_INCLUDE}
        ${LIBBUDDY_HOST_INCLUDE}
        ${HIDAPI_INCLUDE}
)

# source files
set( LIBBUDDY_SOURCES
    ${PROJECT_SOURCE_DIR}/../../../lib/libbuddy/common/codec.c
    ${PROJECT_SOURCE_DIR}/../../../lib/libbuddy/common/support.c
    ${PROJECT_SOURCE_DIR}/../../../lib/libbuddy/host/buddy.c
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(HIDAPI_SOURCES ${PROJECT_SOURCE_DIR}/../../../lib/hidapi/windows/hid.c)
    set(EXTRA_LIBS ${IOKIT_LIBRARY} setupapi.lib)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(HIDAPI_SOURCES ${PROJECT_SOURCE_DIR}/../../../lib/hidapi/linux/hid.c)
    set(EXTRA_LIBS ${IOKIT_LIBRARY} udev)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(HIDAPI_SOURCES ${PROJECT_SOURCE_DIR}/../../../lib/hidapi/mac/hid.c)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    mark_as_advanced(IOKIT_LIBRARY CORE_FOUNDATION_LIBRARY)
    set(EXTRA_LIBS ${IOKIT_LIBRARY} ${CORE_FOUNDATION_LIBRARY})
else ()
    message( FATAL_ERROR "The host build platform is not supported." )
endif()

set (BUDDY_SOURCES ${LIBBUDDY_SOURCES} ${HIDAPI_SOURCES})

# debug: print variables to screen
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

include_directories(${BUDDY_INCLUDE_DIRS})
add_executable (simple simple.c ${BUDDY_SOURCES})
target_link_libraries(simple ${EXTRA_LIBS})