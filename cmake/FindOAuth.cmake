# - Try to find liboauth
# Once done this will define
#  LIBOAUTH_FOUND - System has liboauth
#  LIBOAUTH_INCLUDE_DIRS - The liboauth include directories
#  LIBOAUTH_LIBRARIES - The libraries needed to use liboauth
#  LIBOAUTH_DEFINITIONS - Compiler switches required for using liboauth

find_package(PkgConfig)
pkg_check_modules(PC_LIBOAUTH_QUIET oauth)
set(LIBOAUTH_DEFINITIONS ${PC_LIBOAUTH_CFLAGS_OTHER})

find_path(LIBOAUTH_INCLUDE_DIR oauth.h
	HINTS ${PC_LIBOAUTH_INCLUDEDIR} ${PC_LIBOAUTH_INCLUDE_DIRS}
	PATH_SUFFIXES liboauth)

find_library(LIBOAUTH_LIBRARY NAMES oauth 
	HINTS ${PC_LIBOAUTH_LIBDIR}
	${PC_LIBOAUTH_LIBRARY_DIRS})

set(LIBOAUTH_LIBRARIES ${LIBOAUTH_LIBRARY})
set(LIBOAUTH_INCLUDE_DIRS ${LIBOAUTH_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(liboauth DEFAULT_MSG
	LIBOAUTH_LIBRARY LIBOAUTH_INCLUDE_DIR)
mark_as_advanced(LIBOAUTH INCLUDE_DIR LIBOAUTH_LIBRARY)
