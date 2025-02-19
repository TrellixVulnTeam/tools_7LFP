set(LIB_NAME ossimpdal_plugin)
message( "************** LIBRARY SETUP FOR ossimpdal_plugin******************")

# Include ossim cmake stuff:
include(OssimVersion)
include(OssimCommonVariables)
include(OssimUtilities)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(requiredLibs)

#---
# Need boost:
#---
find_package(Boost)
if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
   set(requiredLibs ${requiredLibs} ${Boost_LIBRARIES})
else(Boost_FOUND)
   message(FATAL_ERROR "Could not find boost")
endif(Boost_FOUND)

find_package(LibXml2)
if(LIBXML2_FOUND)
   include_directories(${LIBXML2_INCLUDE_DIR})
   set(requiredLibs ${requiredLibs} ${LIBXML2_LIBRARIES})
else(LIBXML2_FOUND)
   message(FATAL_ERROR "Whoa! Could not find LibXml2")
endif(LIBXML2_FOUND)

#---
# FindPDAL.cmake sets:
# PDAL_INCLUDE_DIR, PDAL_LIBRARIES, and PDAL_FOUND
#---
find_package(PDAL)
if(PDAL_FOUND)
   include_directories( ${PDAL_INCLUDE_DIR} )
   set(requiredLibs ${requiredLibs} ${PDAL_LIBRARIES} )
else(PDAL_FOUND)
   message(FATAL_ERROR "Could not find pdal")
endif(PDAL_FOUND)

file(GLOB_RECURSE OSSIMPLUGIN_SRCS *.cpp)
file(GLOB_RECURSE OSSIMPLUGIN_HEADERS *.h)

#---
# OSSIMPLUGINSMAKINGDLL controls dll linkage on windows.  
# Adding this sets OSSIM_PLUGINS_DLL #define TO "__declspec(dllexport)".
#---
add_definitions("-DOSSIMPLUGINSMAKINGDLL")

# PDAL uses C++ 2011 code:
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )

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

# Add the test dir:
add_subdirectory( test )
