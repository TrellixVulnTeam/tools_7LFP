# - Find libCrypto
# Find the native OpenSSL LIBCRYPTO includes and library
#
#  LIBCRYPTO_INCLUDE_DIR - where to find sha.h, etc.
#  LIBCRYPTO_LIBRARIES   - List of libraries when using libCrypto.
#  LIBCRYPTO_FOUND       - True if libCrypto found.


IF (LIBCRYPTO_INCLUDE_DIR)
  # Already in cache, be silent
  SET(LIBCRYPTO_FIND_QUIETLY TRUE)
ENDIF (LIBCRYPTO_INCLUDE_DIR)

# Require a regular OpenSSL even on OSX/iOS
# IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#   # MacOSX has deprecated the use of openssl crypto functions
#   # and replaced it with API-compatible CommonCrypto
#   FIND_PATH(LIBCRYPTO_INCLUDE_DIR CommonCrypto/CommonDigest.h)
#   SET(LIBCRYPTO_LIBRARY_NAMES_RELEASE ${LIBCRYPTO_LIBRARY_NAMES_RELEASE} ${LIBCRYPTO_LIBRARY_NAMES} ssl)
#   SET(LIBCRYPTO_LIBRARY_NAMES_DEBUG ${LIBCRYPTO_LIBRARY_NAMES_DEBUG} ssld)
# ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
  FIND_PATH(LIBCRYPTO_INCLUDE_DIR openssl/sha.h)
  SET(LIBCRYPTO_LIBRARY_NAMES_RELEASE ${LIBCRYPTO_LIBRARY_NAMES_RELEASE} ${LIBCRYPTO_LIBRARY_NAMES} crypto)
  SET(LIBCRYPTO_LIBRARY_NAMES_DEBUG ${LIBCRYPTO_LIBRARY_NAMES_DEBUG} cryptod)
# ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 

FIND_LIBRARY(LIBCRYPTO_LIBRARY_RELEASE NAMES ${LIBCRYPTO_LIBRARY_NAMES_RELEASE} )

# Find a debug library if one exists and use that for debug builds.
# This really only does anything for win32, but does no harm on other
# platforms.
FIND_LIBRARY(LIBCRYPTO_LIBRARY_DEBUG NAMES ${LIBCRYPTO_LIBRARY_NAMES_DEBUG})

INCLUDE(LibraryDebugAndRelease)
SET_LIBRARY_FROM_DEBUG_AND_RELEASE(LIBCRYPTO)

# handle the QUIETLY and REQUIRED arguments and set LIBCRYPTO_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBCRYPTO DEFAULT_MSG LIBCRYPTO_LIBRARY LIBCRYPTO_INCLUDE_DIR)

IF(LIBCRYPTO_FOUND)
  SET( LIBCRYPTO_LIBRARIES ${LIBCRYPTO_LIBRARY} )
ELSE(LIBCRYPTO_FOUND)
  SET( LIBCRYPTO_LIBRARIES )
ENDIF(LIBCRYPTO_FOUND)

MARK_AS_ADVANCED( LIBCRYPTO_LIBRARY LIBCRYPTO_INCLUDE_DIR )

