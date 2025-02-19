#------------------------------------------------------------------------------
#
# build.py
#
# Summary : Builds all of the third-party libraries.
#
#------------------------------------------------------------------------------

import os
import sys

from BuildSettingSet import *
from SystemManager import *

#------------------------------------------------------------------------------
# The Program class represents the main class of the script.
class Program :

    #--------------------------------------------------------------------------
    # constants
    
        #----------------------------------------------------------------------
        # the name of the build file to build APR
        _FILE_NAME_BUILD_APR = "build_apr.py"
        #----------------------------------------------------------------------
        # the name of the build file to build APR Util
        _FILE_NAME_BUILD_APR_UTIL = "build_aprutil.py"
        #----------------------------------------------------------------------
        # the name of the build file to build Bison
        _FILE_NAME_BUILD_BISON = "build_bison.py"
        #----------------------------------------------------------------------
        # the name of the build file to build Boost
        _FILE_NAME_BUILD_BOOST = "build_boost.py"
        #----------------------------------------------------------------------
        # the name of the build file to build Crypto++
        _FILE_NAME_BUILD_CRYPTO = "build_crypto.py"
        #----------------------------------------------------------------------
        # the name of the build file to build CryptoWrapper
        _FILE_NAME_BUILD_CRYPTOWRAPPER = "build_cryptowrapper.py"
        #----------------------------------------------------------------------
        # the name of the build file to build CURL
        _FILE_NAME_BUILD_CURL = "build_curl.py"
        #----------------------------------------------------------------------
        # the name of the build file to build FreeType
        _FILE_NAME_BUILD_FREETYPE = "build_freetype.py"
        #----------------------------------------------------------------------
        # the name of the build file to build FreeType
        _FILE_NAME_BUILD_GALIB = "build_galib.py"
        #----------------------------------------------------------------------
         # the name of the build file to build Geos
        _FILE_NAME_BUILD_GEOS = "build_geos.py"
        #----------------------------------------------------------------------
        # the name of the build file to build Glew
        _FILE_NAME_BUILD_GLEW = "build_glew.py"
        #----------------------------------------------------------------------
        # the name of the build file to build FreeType
        _FILE_NAME_BUILD_GLUT = "build_glut.py"
        #----------------------------------------------------------------------
        # the name of the build file to build GSoap
        _FILE_NAME_BUILD_GSOAP = "build_gsoap.py"
        #----------------------------------------------------------------------
        # the name of the build file to build HawkNL
        _FILE_NAME_BUILD_HAWKNL = "build_hawknl.py"
        #----------------------------------------------------------------------
        # the name of the build file to build HDF5
        _FILE_NAME_BUILD_HDF5 = "build_hdf5.py"
        #----------------------------------------------------------------------
        # the name of the build file to build KDIS
        _FILE_NAME_BUILD_KDIS = "build_kdis.py"
        #----------------------------------------------------------------------
        # the name of the build file to build LibGeoTIFF
        _FILE_NAME_BUILD_LIBGEOTIFF = "build_libgeotiff.py"
        #----------------------------------------------------------------------
        # the name of the build file to build LibGIST
        _FILE_NAME_BUILD_LIBGIST = "build_libgist.py"
        #----------------------------------------------------------------------
        # the name of the build file to build LibICONV
        _FILE_NAME_BUILD_LIBICONV = "build_libiconv.py"
        #----------------------------------------------------------------------
        # the name of the build file to build LibJPEG
        _FILE_NAME_BUILD_LIBJPEG = "build_libjpeg.py"
        #----------------------------------------------------------------------
        # the name of the build file to build LibPNG
        _FILE_NAME_BUILD_LIBKML = "build_libkml.py"
        #----------------------------------------------------------------------
        # the name of the build file to build LibPNG
        _FILE_NAME_BUILD_LIBPNG = "build_libpng.py"
        #----------------------------------------------------------------------
        # the name of the build file to build LibXML
        _FILE_NAME_BUILD_LIBXML = "build_libxml.py"
        #----------------------------------------------------------------------
        # the name of the build file to build LibTIFF
        _FILE_NAME_BUILD_LIBTIFF = "build_libtiff.py"
        #----------------------------------------------------------------------
         #the name of the build file to build minizip
        _FILE_NAME_BUILD_MINIZIP = "build_minizip.py"
        #----------------------------------------------------------------------
         #the name of the build file to build muparser
        _FILE_NAME_BUILD_MUPARSER = "build_muparser.py"
        #----------------------------------------------------------------------
        # the name of the build file to build Newmat
        _FILE_NAME_BUILD_NEWMAT = "build_newmat.py"
        #----------------------------------------------------------------------
        # the name of the build file to build OpenCV
        _FILE_NAME_BUILD_OPENCV = "build_opencv.py"
        #----------------------------------------------------------------------
        # the name of the build file to build OpenDIS
        _FILE_NAME_BUILD_OPENDIS = "build_opendis.py"
        #----------------------------------------------------------------------
        # the name of the build file to build OpenThreads
        _FILE_NAME_BUILD_OPENTHREADS = "build_openthreads.py"
        #----------------------------------------------------------------------
        # the name of the build file to build PoDoFo
        _FILE_NAME_BUILD_PODOFO = "build_podofo.py"
        #----------------------------------------------------------------------
        # the name of the build file to build PROJ.4
        _FILE_NAME_BUILD_PROJ4 = "build_proj4.py"
        #----------------------------------------------------------------------
        # the name of the build file to build LibPNG
        _FILE_NAME_BUILD_PTHREADS = "build_pthreads.py"
        #----------------------------------------------------------------------
        # the name of the build file to build Qt
        _FILE_NAME_BUILD_QT = "build_qt.py"
        #----------------------------------------------------------------------
        # the name of the build file to build QuaZIP
        _FILE_NAME_BUILD_QUAZIP = "build_quazip.py"
        #----------------------------------------------------------------------
        # the name of the build file to build QWT
        _FILE_NAME_BUILD_QWT = "build_qwt.py"
        #----------------------------------------------------------------------
        # the name of the build file to build SZip
        _FILE_NAME_BUILD_SZIP = "build_szip.py"
        #----------------------------------------------------------------------
        # the name of the build file to build ThreadingBuildingBlocks
        _FILE_NAME_BUILD_TBB = "build_tbb.py"
        #----------------------------------------------------------------------
        # the name of the build file to build Visual Leak Detector
        _FILE_NAME_BUILD_VLD = "build_vld.py"
        #----------------------------------------------------------------------
        # the name of the build file to build Xerces
        _FILE_NAME_BUILD_XERCES = "build_xerces.py"
        #----------------------------------------------------------------------
        # the name of the build file to build ZLib
        _FILE_NAME_BUILD_ZLIB = "build_zlib.py"
        #----------------------------------------------------------------------
        # the name of the build file to build CppUnit
        _FILE_NAME_BUILD_CPPUNIT = "build_cppunit.py"
        #----------------------------------------------------------------------
        # the name of the build file to build GDAL
        _FILE_NAME_BUILD_GDAL = "build_gdal.py"
        #----------------------------------------------------------------------
        # the name of the build file to build LasZip
        _FILE_NAME_BUILD_LASZIP = "build_laszip.py"
        #----------------------------------------------------------------------
        # the name of the build file to build LasZip
        _FILE_NAME_BUILD_LIBLAS = "build_liblas.py"
        #----------------------------------------------------------------------
        # the name of the build file to build OGDI
        _FILE_NAME_BUILD_OGDI = "build_ogdi.py"
        #----------------------------------------------------------------------
        #the name of the build file to build Expat
        _FILE_NAME_BUILD_EXPAT = "build_expat.py"
        #----------------------------------------------------------------------
        #the name of the build file to build uriparser
        _FILE_NAME_BUILD_URIPARSER = "build_uriparser.py"
        #----------------------------------------------------------------------
         #the name of the build file to build googletest
        _FILE_NAME_BUILD_GOOGLETEST = "build_googletest.py"
        #----------------------------------------------------------------------
         #the name of the build file to build OpenSceneGraph
        _FILE_NAME_BUILD_OSG = "build_osg.py"
        #----------------------------------------------------------------------
         #the name of the build file to build Log4cxx
        _FILE_NAME_BUILD_LOG4CXX = "build_log4cxx.py"
        #----------------------------------------------------------------------
         #the name of the build file to build qxrunner
        _FILE_NAME_BUILD_QXRUNNER = "build_qxrunner.py"
        #----------------------------------------------------------------------
         #the name of the build file to build firebird
        _FILE_NAME_BUILD_FIREBIRD = "build_firebird.py"
        #----------------------------------------------------------------------
        # the name of the Python executable file
        _FILE_NAME_PYTHON = "python.exe"
        #----------------------------------------------------------------------
        
    #--------------------------------------------------------------------------
    # constructors
    
        #----------------------------------------------------------------------
        # Constructs this program.
        #
        # Parameters :
        #     self : this program
        def __init__(self) :
        
            pass
        #----------------------------------------------------------------------
        
    #--------------------------------------------------------------------------
    # public methods
    
        #----------------------------------------------------------------------
        # The main method of the program.
        #
        # Parameters :
        #     self : this program
        def main(self) :
        
            if ( len(sys.argv) > 1 ) :
            
                # build all configurations of the specified build file
                self._build(sys.argv[1])
            
            else :
                # build libraries that do not depend on other libraries
                #     (order does not matter)
                if True:
                    self._build(Program._FILE_NAME_BUILD_APR        )
                    self._build(Program._FILE_NAME_BUILD_BISON      )
                    self._build(Program._FILE_NAME_BUILD_BOOST      )
                    self._build(Program._FILE_NAME_BUILD_CPPUNIT    )
                    self._build(Program._FILE_NAME_BUILD_CRYPTO     )
                    self._build(Program._FILE_NAME_BUILD_CURL       )
                    self._build(Program._FILE_NAME_BUILD_EXPAT      )
                    self._build(Program._FILE_NAME_BUILD_FIREBIRD   )
                    self._build(Program._FILE_NAME_BUILD_GALIB      )
                    self._build(Program._FILE_NAME_BUILD_GEOS       )
                    self._build(Program._FILE_NAME_BUILD_GLEW       )
                    self._build(Program._FILE_NAME_BUILD_GLUT       )
                    self._build(Program._FILE_NAME_BUILD_GOOGLETEST )
                    self._build(Program._FILE_NAME_BUILD_GSOAP      )
                    self._build(Program._FILE_NAME_BUILD_HDF5       )
                    self._build(Program._FILE_NAME_BUILD_KDIS       )
                    self._build(Program._FILE_NAME_BUILD_LASZIP     )
                    self._build(Program._FILE_NAME_BUILD_LIBGIST    )
                    self._build(Program._FILE_NAME_BUILD_LIBICONV   )
                    self._build(Program._FILE_NAME_BUILD_LIBJPEG    )
                    self._build(Program._FILE_NAME_BUILD_LIBKML     )
                    self._build(Program._FILE_NAME_BUILD_MUPARSER   )
                    self._build(Program._FILE_NAME_BUILD_NEWMAT     )
                    self._build(Program._FILE_NAME_BUILD_OPENCV     )
                    self._build(Program._FILE_NAME_BUILD_OPENDIS    )
                    self._build(Program._FILE_NAME_BUILD_OPENTHREADS)
                    self._build(Program._FILE_NAME_BUILD_PROJ4      )
                    self._build(Program._FILE_NAME_BUILD_PTHREADS   )
                    self._build(Program._FILE_NAME_BUILD_SZIP       )
                    self._build(Program._FILE_NAME_BUILD_TBB        )
                    self._build(Program._FILE_NAME_BUILD_URIPARSER  )
                    self._build(Program._FILE_NAME_BUILD_VLD        )
                    self._build(Program._FILE_NAME_BUILD_ZLIB       )

                # build libraries that depend on other libraries
                #     (order does matter)
                self._build(Program._FILE_NAME_BUILD_QT         )
                self._build(Program._FILE_NAME_BUILD_QXRUNNER   )
                self._build(Program._FILE_NAME_BUILD_FREETYPE   )
                self._build(Program._FILE_NAME_BUILD_HAWKNL     )
                self._build(Program._FILE_NAME_BUILD_QWT       )
                self._build(Program._FILE_NAME_BUILD_APR_UTIL  )
                self._build(Program._FILE_NAME_BUILD_CRYPTOWRAPPER )
                self._build(Program._FILE_NAME_BUILD_MINIZIP   )
                self._build(Program._FILE_NAME_BUILD_LIBPNG    )
                self._build(Program._FILE_NAME_BUILD_LIBXML    )
                self._build(Program._FILE_NAME_BUILD_LIBTIFF   )
                self._build(Program._FILE_NAME_BUILD_LIBGEOTIFF)
                self._build(Program._FILE_NAME_BUILD_LOG4CXX   )
                self._build(Program._FILE_NAME_BUILD_PODOFO    )
                self._build(Program._FILE_NAME_BUILD_QUAZIP    )
                self._build(Program._FILE_NAME_BUILD_XERCES    )
                self._build(Program._FILE_NAME_BUILD_OGDI      )
                self._build(Program._FILE_NAME_BUILD_GDAL      )
                self._build(Program._FILE_NAME_BUILD_LIBLAS    )
                self._build(Program._FILE_NAME_BUILD_OSG       )
        #----------------------------------------------------------------------
        
    #--------------------------------------------------------------------------
    # private methods
    
        #----------------------------------------------------------------------
        # Builds a library using a specified build file.
        #
        # Parameters :
        #     self          : this program
        #     buildFileName : the name of the build file to use
        def _build( self          , \
                    buildFileName ) :
                    
            systemManager = SystemManager()
        
            if "firebird" in buildFileName:
                print("Skipping Firebird 32-bit debug build")
            else:
                # build 32-bit debug
                ret = os.system( ( "%s \"%s\" \"%s\" \"%s\"" % \
                           ( Program._FILE_NAME_PYTHON                            , \
                             os.path.join( systemManager.getCurrentPathName() , \
                                           buildFileName                      )   , \
                             BuildSettingSet.ARGUMENT_VALUE_BITNESS_X86           ,
                             BuildSettingSet.ARGUMENT_VALUE_CONFIGURATION_DEBUG   ) ) )
                if ret != 0:
                    print("Error building 32-bit debug")
                    sys.exit(-1)
             
            
            # build 32-bit release
            os.system( ( "%s \"%s\" \"%s\" \"%s\"" % \
                       ( Program._FILE_NAME_PYTHON                            , \
                         os.path.join( systemManager.getCurrentPathName() , \
                                       buildFileName                      )   , \
                         BuildSettingSet.ARGUMENT_VALUE_BITNESS_X86           ,
                         BuildSettingSet.ARGUMENT_VALUE_CONFIGURATION_RELEASE ) ) )
            if ret != 0:
                print("Error building 32-bit release")
                sys.exit(-1)

            # build 64-bit debug
            if "firebird" in buildFileName:
                print("Skipping Firebird 64-bit debug build")
            else:            
                os.system( ( "%s \"%s\" \"%s\" \"%s\"" % \
                           ( Program._FILE_NAME_PYTHON                            , \
                             os.path.join( systemManager.getCurrentPathName() , \
                                           buildFileName                      )   , \
                             BuildSettingSet.ARGUMENT_VALUE_BITNESS_X64           ,
                             BuildSettingSet.ARGUMENT_VALUE_CONFIGURATION_DEBUG   ) ) )
                if ret != 0:
                    print("Error building 64-bit debug")
                    sys.exit(-1)

            # build 64-bit release
            os.system( ( "%s \"%s\" \"%s\" \"%s\"" % \
                       ( Program._FILE_NAME_PYTHON                            , \
                         os.path.join( systemManager.getCurrentPathName() , \
                                       buildFileName                      )   , \
                         BuildSettingSet.ARGUMENT_VALUE_BITNESS_X64           ,
                         BuildSettingSet.ARGUMENT_VALUE_CONFIGURATION_RELEASE ) ) )
            if ret != 0:
                print("Error building 64-bit release")
                sys.exit(-1)

        #----------------------------------------------------------------------
        
    #--------------------------------------------------------------------------
    
#------------------------------------------------------------------------------
Program().main()
#------------------------------------------------------------------------------
