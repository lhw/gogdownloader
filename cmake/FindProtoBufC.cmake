# - Try to find ProtoBufC
# Once done this will define
#  PROTOBUFC_FOUND - system has ProtoBufC
#  PROTOBUFC_INCLUDE_DIR - the ProtoBufC include directory
#  PROTOBUFC_LIBRARIES - Link these to use ProtoBufC
#  PROTOBUFC_DEFINITIONS - Compiler switches required for using ProtoBufC


find_package(PkgConfig)
pkg_check_modules(PC_LIBPROTOBUFC_QUIET libprotobuf-c)
set(LIBPROTOBUFC_DEFINITIONS ${PC_LIBPROTOBUFC_CFLAGS_OTHER})

find_path(LIBPROTOBUFC_INCLUDE_DIR google/protobuf-c/protobuf-c.h
	HINTS ${PC_LIBPROTOBUFC_INCLUDEDIR} ${PC_LIBPROTOBUFC_INCLUDE_DIRS}
	PATH_SUFFIXES libprotobuf-c)

find_library(LIBPROTOBUFC_LIBRARY NAMES libprotobuf-c
	HINTS ${PC_LIBPROTOBUFC_LIBDIR}
	${PC_LIBPROTOBUFC_LIBRARY_DIRS})

set(LIBPROTOBUFC_LIBRARIES ${LIBPROTOBUFC_LIBRARY})
set(LIBPROTOBUFC_INCLUDE_DIRS ${LIBPROTOBUFC_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(liboauth DEFAULT_MSG
	LIBPROTOBUFC_LIBRARY LIBPROTOBUFC_INCLUDE_DIR)
mark_as_advanced(LIBPROTOBUFC INCLUDE_DIR LIBPROTOBUFC_LIBRARY)
