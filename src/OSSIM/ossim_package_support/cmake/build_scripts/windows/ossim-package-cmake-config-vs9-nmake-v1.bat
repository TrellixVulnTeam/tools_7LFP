set common_dir="C:/Projects/SURGE/DigitalGlobe/Projects/Tools/build/OSSIM"
set build_dir="C:/Projects/SURGE/DigitalGlobe/Projects/Tools/build/OSSIM"
set pkg_dir="C:/Projects/SURGE/DigitalGlobe/Projects/Tools/src/OSSIM/ossim_package_support"

cmake -G "NMake Makefiles" ^
-DWIN32_USE_MP=OFF ^
-DBUILD_CSMAPI=OFF ^
-DBUILD_LIBRARY_DIR=lib ^
-DBUILD_OMS=OFF ^
-DBUILD_OSSIM=ON ^
-DBUILD_OSSIM_PLUGIN=ON ^
-DBUILD_OSSIMCONTRIB_PLUGIN=OFF ^
-DBUILD_OSSIMCSM_PLUGIN=OFF ^
-DBUILD_OSSIMGDAL_PLUGIN=ON ^
-DBUILD_OSSIMKAKADU_PLUGIN=ON ^
-DBUILD_OSSIMLIBRAW_PLUGIN=ON ^
-DBUILD_OSSIMMRSID_PLUGIN=ON ^
-DBUILD_OSSIMNDF_PLUGIN=OFF ^
-DBUILD_OSSIMNUI_PLUGIN=OFF ^
-DBUILD_OSSIMPNG_PLUGIN=ON ^
-DBUILD_OSSIMREGISTRATION_PLUGIN=ON ^
-DBUILD_OSSIMQT4=OFF ^
-DBUILD_OSSIM_MPI_SUPPORT=0 ^
-DBUILD_OSSIMPLANET=OFF ^
-DBUILD_OSSIMPLANETQT=OFF ^
-DBUILD_OSSIMPREDATOR=OFF ^
-DBUILD_OSSIM_TEST_APPS=1 ^
-DBUILD_RUNTIME_DIR=bin ^
-DBUILD_SHARED_LIBS=ON ^
-DBUILD_WMS=off ^
-DCMAKE_BUILD_TYPE=Release ^
-DCMAKE_INCLUDE_PATH=%build_dir%/local/include ^
-DCMAKE_INSTALL_PREFIX=%build_dir%/local ^
-DCMAKE_LIBRARY_PATH=%build_dir%/local/lib ^
-DCMAKE_MODULE_PATH=%pkg_dir%/cmake/CMakeModules ^
-DKAKADU_ROOT_SRC=%common_dir%/kakadu/v6_2_1-00367C ^
-DKAKADU_LIBRARY=%build_dir%/local/lib/kdu_v62R.lib ^
-DMRSID_LIBRARY=%build_dir%/local/lib/lti_dsdk_dll.lib ^
-DOPENTHREADS_LIBRARY=%build_dir%/local/lib/OpenThreads.lib ^
-DPNG_LIBRARY=%build_dir%/local/lib/libpng.lib ^
-DTIFF_LIBRARY=%build_dir%/local/lib/libtiff_i.lib ^
-DGEOTIFF_LIBRARY=%build_dir%/local/lib/geotiff_i.lib ^
-DFFTW3_LIBRARY=%build_dir%/local/lib/libfftw3-3.lib ^
-DMRSID_INCLUDE_DIR=%common_dir%/mrsid/Geo_DSDK-7.0.0.2181.win32-vc9/Geo_DSDK-7.0.0.2181/include ^
-DOSSIM_COMPILE_WITH_FULL_WARNING=ON ^
-DOSSIM_DEPENDENCIES=%build_dir%/local ^
-DOSSIM_DEV_HOME=%build_dir% ^
-DBUILD_SHARED_LIBS=ON ^
%pkg_dir%/cmake/
