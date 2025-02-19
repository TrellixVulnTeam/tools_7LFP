# Version checking for libLAS

set(PACKAGE_VERSION "1.8.0")
set(PACKAGE_VERSION_MAJOR "1")
set(PACKAGE_VERSION_MINOR "8")
set(PACKAGE_VERSION_PATCH "0")
 
if (WIN32 AND NOT "${CMAKE_GENERATOR}" STREQUAL "Visual Studio 14 2015")
  # Reject if there's a mismatch on compiler versions (Windows only)
  set (PACKAGE_VERSION_UNSUITABLE TRUE)
elseif (PACKAGE_FIND_VERSION)
  if (${PACKAGE_FIND_VERSION} VERSION_EQUAL ${PACKAGE_VERSION})
    set (PACKAGE_VERSION_EXACT TRUE)
  elseif (${PACKAGE_FIND_VERSION} VERSION_LESS ${PACKAGE_VERSION}
    AND ${PACKAGE_FIND_VERSION_MAJOR} EQUAL ${PACKAGE_VERSION_MAJOR})
    set (PACKAGE_VERSION_COMPATIBLE TRUE)
  endif ()
endif ()
