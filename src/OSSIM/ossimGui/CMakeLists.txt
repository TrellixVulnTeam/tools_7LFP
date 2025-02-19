PROJECT(ossimGui)
MESSAGE("**********************Setting up ossimGui********************")
INCLUDE(OssimVersion)
INCLUDE(OssimUtilities)
INCLUDE(OssimCommonVariables)
INCLUDE(OssimQt4Macros)

SET (QT_MIN_VERSION "4.2.2")

FIND_PACKAGE(Qt4 REQUIRED) # find and setup Qt4 for this project
INCLUDE(${QT_USE_FILE})
SET(QT_USE_QTOPENGL ON)
find_package(Qt4)

find_package(OpenThreads)
find_package(ossim)
########find_package(TIFF)
IF(BUILD_OSSIMPLANET)
	find_package(ossimPlanet)
	find_package(OpenGL)
	find_package(OSG)
ENDIF(BUILD_OSSIMPLANET)

ADD_DEFINITIONS(${QT_DEFINITIONS}
	-DQT_CORE_LIB
	-DQT_GUI_LIB
	-DQT_THREAD_SUPPORT
)
INCLUDE_DIRECTORIES(${${PROJECT_NAME}_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${OSSIM_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${QT_INCLUDES})
INCLUDE_DIRECTORIES(${OPENTHREADS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})

#MESSAGE(STATUS "QT_INCLUDES = ${QT_INCLUDES}")

############################################## BUILD THE LIBRARY ############################################
SET(LIB_NAME ossimGui)
FILE(GLOB OSSIMGUI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include/ossimGui/*.h")
FILE(GLOB OSSIMGUI_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/ossimGui/*.cpp" )
FILE(GLOB OSSIMGUI_UIS  "${CMAKE_CURRENT_SOURCE_DIR}/src/ossimGui/ui/*.ui")

SET(ADDITIONAL_LIBRARIES "")
IF(BUILD_OSSIMPLANET)
#   LIST(APPEND ADDITIONAL_INCLUDES ${OSSIMPLANET_INCLUDE_DIR})
#   LIST(APPEND ADDITIONAL_INCLUDES ${OSG_INCLUDE_DIR})
   INCLUDE_DIRECTORIES(${OSSIMPLANET_INCLUDE_DIR})
   INCLUDE_DIRECTORIES(${OSG_INCLUDE_DIR})

   LIST(APPEND ADDITIONAL_LIBRARIES ${OSSIMPLANET_LIBRARY})
   LIST(APPEND ADDITIONAL_LIBRARIES ${QT_QTOPENGL_LIBRARY})

   ADD_DEFINITIONS(-DOSSIM_PLANET_ENABLED)
ELSE()

   LIST(REMOVE_ITEM OSSIMGUI_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/ossimGui/PlanetMdiSubWindow.cpp")
   LIST(REMOVE_ITEM OSSIMGUI_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/ossimGui/GlWidget.cpp")
   LIST(REMOVE_ITEM OSSIMGUI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include/ossimGui/PlanetMdiSubWindow.h")
   LIST(REMOVE_ITEM OSSIMGUI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include/ossimGui/GlWidget.h")

ENDIF()

MESSAGE("QT4_LIBRARIES === ${QT_LIBRARIES}")
QT4_WRAP_CPP(OSSIMGUI_MOC_SRCS ${OSSIMGUI_INCLUDES} )

MESSAGE("UI_DESTINATION_DIR ${${PROJECT_NAME}_SOURCE_DIR}/include/ossimGui")
SET(UI_DESTINATION_DIR ${${PROJECT_NAME}_SOURCE_DIR}/include/ossimGui)
OSSIM_QT4_WRAP_UI(OSSIMGUI_UI_SRC ${OSSIMGUI_UIS} )

SET(INCLUDE_DIR "include/ossimGui")

#---
# NOTE: INSTALL_HEADERS removed from last argument line for rpms as they were
# not prefixed and not needed for development purposes at this time.
#---
OSSIM_LINK_LIBRARY(${LIB_NAME} COMPONENT_NAME ossim 
                   TYPE "${OSSIM_USER_DEFINED_DYNAMIC_OR_STATIC}" HEADERS ${OSSIMGUI_INCLUDES} ${OSSIMGUI_UI_SRC} SOURCE_FILES ${OSSIMGUI_SRCS}  ${OSSIMGUI_MOC_SRCS}
                   LIBRARIES ${OSSIM_LIBRARY} ${OSG_LIBRARY}  ${OPENTHREADS_LIBRARY} ${JPEG_LIBRARY} ${ZLIB_LIBRARY} ${QT_LIBRARIES} ${TIFF_LIBRARIES}  ${ADDITIONAL_LIBRARIES}
                   INSTALL_LIB VERSION_SYMLINKS ADDITIONAL_COMPILE_FLAGS "-DOSSIMGUI_LIBRARY")

############################################# BUILD THE APPLICATIONS #################################################

QT4_ADD_RESOURCES(OSSIMGUI_RC "${PROJECT_SOURCE_DIR}/images/images.qrc")
set(TARGET_COMMON_LIBRARIES ossimGui)
FILE(GLOB SOURCE_FILES "${PROJECT_SOURCE_DIR}/apps/ossim-geocell/ossim-geocell.cpp")
SET(OSSIM_BUILD_APPLICATION_BUNDLES ON)
MESSAGE(STATUS "IMAGELINKER SOURCE FILES = ${SOURCE_FILES}")
OSSIM_SETUP_APPLICATION(ossim-geocell INSTALL REQUIRE_WINMAIN_FLAG COMPONENT_NAME ossim SOURCE_FILES ${SOURCE_FILES} ${OSSIMGUI_RC})

########################### SETUP UNINSTALL ############################
OSSIM_ADD_COMMON_MAKE_UNINSTALL()
