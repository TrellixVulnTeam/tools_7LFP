set(LIB_NAME ossimhdf_plugin)
MESSAGE( "************** LIBRARY SETUP FOR ossimhdf_plugin******************")

# Include ossim cmake stuff:
include(OssimVersion)
include(OssimCommonVariables)
include(OssimUtilities)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(requiredLibs)

find_package(HDF4)
IF(HDF4_FOUND)
   INCLUDE_DIRECTORIES( ${HDF4_INCLUDE_DIR} )
   SET(requiredLibs ${requiredLibs} ${HDF4_LIBRARIES} )
ELSE(HDF4_FOUND)
   MESSAGE(FATAL_ERROR "Could not find hdf4")
ENDIF(HDF4_FOUND)

set(HDF5_USE_STATIC_LIBRARIES FALSE)
find_package(HDF5 COMPONENTS CXX C)
IF(HDF5_FOUND)
   INCLUDE_DIRECTORIES( ${HDF5_INCLUDE_DIR} )
   SET(requiredLibs ${requiredLibs} ${HDF5_CXX_LIBRARIES} ${HDF5_LIBRARIES} )
ELSE(HDF5_FOUND)
   MESSAGE(FATAL_ERROR "Could not find hdf5")
ENDIF(HDF5_FOUND)

FILE(GLOB_RECURSE OSSIMPLUGIN_SRCS *.cpp)
FILE(GLOB_RECURSE OSSIMPLUGIN_HEADERS *.h)

####
# OSSIMPLUGINSMAKINGDLL controls dll linkage on windows.  
# Adding this sets OSSIM_PLUGINS_DLL #define TO "__declspec(dllexport)".
###
add_definitions("-DOSSIMPLUGINSMAKINGDLL")

# Adjust the install library path:
if(OSSIM_INSTALL_PLUGINS_WITH_VERSION)
   set(INSTALL_LIBRARY_DIR   "${INSTALL_LIBRARY_DIR}/ossim-${OSSIM_VERSION}/plugins")
else(OSSIM_INSTALL_PLUGINS_WITH_VERSION)
   set(INSTALL_LIBRARY_DIR   "${INSTALL_LIBRARY_DIR}/ossim/plugins")
endif(OSSIM_INSTALL_PLUGINS_WITH_VERSION)

OSSIM_LINK_LIBRARY(${LIB_NAME}
                   COMPONENT_NAME ossim TYPE "${OSSIM_PLUGIN_LINK_TYPE}"
		   LIBRARIES ${OSSIM_LIBRARY} ${requiredLibs}
                   HEADERS "${OSSIMPLUGIN_HEADERS}"
		   SOURCE_FILES "${OSSIMPLUGIN_SRCS}"
                   INSTALL_LIB)

MESSAGE( STATUS "HDF Plugin required libs          = ${requiredLibs}" )
