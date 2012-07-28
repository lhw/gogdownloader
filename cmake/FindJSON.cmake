# - Try to find libjson
# Once done this will define
#  LIBJSON_FOUND - System has libjson
#  LIBJSON_INCLUDE_DIRS - The libjson include directories
#  LIBJSON_LIBRARIES - The libraries needed to use libjson
#  LIBJSON_DEFINITIONS - Compiler switches required for using libjson

find_package(PkgConfig)
pkg_check_modules(PC_LIBJSON_QUIET json)
set(LIBJSON_DEFINITIONS ${PC_LIBJSON_CFLAGS_OTHER})

find_path(LIBJSON_INCLUDE_DIR json/json_tokener.h
	HINTS ${PC_LIBJSON_INCLUDEDIR} ${PC_LIBJSON_INCLUDE_DIRS}
	PATH_SUFFIXES libjson)

find_library(LIBJSON_LIBRARY NAMES json 
	HINTS ${PC_LIBJSON_LIBDIR}
	${PC_LIBJSON_LIBRARY_DIRS})

set(LIBJSON_LIBRARIES ${LIBJSON_LIBRARY})
set(LIBJSON_INCLUDE_DIRS ${LIBJSON_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libjson DEFAULT_MSG
	LIBJSON_LIBRARY LIBJSON_INCLUDE_DIR)
mark_as_advanced(LIBJSON INCLUDE_DIR LIBJSON_LIBRARY)
