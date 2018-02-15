#  FDS_FOUND - System has libfds
#  FDS_INCLUDE_DIRS - The libfds include directories
#  FDS_LIBRARIES - The libraries needed to use libfds
#  FDS_DEFINITIONS - Compiler switches required for using libfds

# use pkg-config to get the directories and then use these values
# in the find_path() and find_library() calls
find_package(PkgConfig)
pkg_check_modules(PC_FDS QUIET libfds)
set(FDS_DEFINITIONS ${PC_FDS_CFLAGS_OTHER})

find_path(
	FDS_INCLUDE_DIR libfds.h
	HINTS ${PC_FDS_INCLUDEDIR} ${PC_FDS_INCLUDE_DIRS}
	PATH_SUFFIXES include
)

find_library(
	FDS_LIBRARY NAMES fds libfds
	HINTS ${PC_FDS_LIBDIR} ${PC_FDS_LIBRARY_DIRS}
	PATH_SUFFIXES lib lib64
)

set(FDS_VERSION_STRING ${PC_FDS_VERSION})

# handle the QUIETLY and REQUIRED arguments and set LIBFDS_FOUND to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibFds
	REQUIRED_VARS FDS_LIBRARY FDS_INCLUDE_DIR
	VERSION_VAR FDS_VERSION_STRING
)

set(FDS_LIBRARIES ${FDS_LIBRARY})
set(FDS_INCLUDE_DIRS ${FDS_INCLUDE_DIR})
mark_as_advanced(FDS_INCLUDE_DIR FDS_LIBRARY)