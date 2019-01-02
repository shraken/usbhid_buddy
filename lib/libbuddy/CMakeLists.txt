project( buddy )
cmake_minimum_required(VERSION 3.1)

# header files
set (LIBBUDDY_COMMON_INCLUDE ${PROJECT_SOURCE_DIR}/common)
set (LIBBUDDY_HOST_INCLUDE ${PROJECT_SOURCE_DIR}/host)
set (HIDAPI_INCLUDE ${PROJECT_SOURCE_DIR}/../hidapi/hidapi) 


set (BUDDY_INCLUDE_DIRS 
    ${LIBBUDDY_COMMON_INCLUDE}
    ${LIBBUDDY_HOST_INCLUDE}
    ${HIDAPI_INCLUDE}
)

set( LIBBUDDY_SOURCES
    ${PROJECT_SOURCE_DIR}/common/codec.c
    ${PROJECT_SOURCE_DIR}/common/support.c
    ${PROJECT_SOURCE_DIR}/host/buddy.c
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(HIDAPI_SOURCES ${PROJECT_SOURCE_DIR}/../hidapi/windows/hid.c)
    set(EXTRA_LIBS ${IOKIT_LIBRARY} setupapi.lib)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(HIDAPI_SOURCES ${PROJECT_SOURCE_DIR}/../hidapi/linux/hid.c)
    set(EXTRA_LIBS ${IOKIT_LIBRARY} udev)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(HIDAPI_SOURCES ${PROJECT_SOURCE_DIR}/../hidapi/mac/hid.c)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    mark_as_advanced(IOKIT_LIBRARY CORE_FOUNDATION_LIBRARY)
    set(EXTRA_LIBS ${IOKIT_LIBRARY} ${CORE_FOUNDATION_LIBRARY})
else ()
    message( FATAL_ERROR "The host build platform is not supported." )
endif()

set (BUDDY_SOURCES ${LIBBUDDY_SOURCES} ${HIDAPI_SOURCES})

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

include_directories(${BUDDY_INCLUDE_DIRS})
add_library(buddy SHARED ${BUDDY_SOURCES})
target_link_libraries(buddy ${EXTRA_LIBS})