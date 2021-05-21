# Download and set up libfreenect2

include(cmake/Externals.cmake)

if(FAST_BUILD_ALL_DEPENDENCIES)
if(WIN32)
ExternalProject_Add(realsense
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/realsense
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/realsense
        GIT_REPOSITORY "https://github.com/IntelRealSense/librealsense.git"
        GIT_TAG "v2.40.0"
        CMAKE_ARGS
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_GRAPHICAL_EXAMPLES:BOOL=OFF
        -DBUILD_EASYLOGGINGPP:BOOL=OFF
        -DBUILD_WITH_TM2:BOOL=OFF
        -DBUILD_WITH_OPENMP:BOOL=ON
        CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=Release
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
        -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
        -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        )
else()
ExternalProject_Add(realsense
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/realsense
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/realsense
        GIT_REPOSITORY "https://github.com/IntelRealSense/librealsense.git"
        GIT_TAG "v2.40.0"
	INSTALL_COMMAND make install/strip
        CMAKE_ARGS
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_GRAPHICAL_EXAMPLES:BOOL=OFF
        -DBUILD_EASYLOGGINGPP:BOOL=OFF
        -DBUILD_WITH_TM2:BOOL=OFF
        -DBUILD_WITH_OPENMP:BOOL=ON
        CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=Release
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
        -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
        -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        )
endif()
else(FAST_BUILD_ALL_DEPENDENCIES)
if(WIN32)
else()
    ExternalProject_Add(realsense
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/realsense
            URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL}/linux/realsense_2.40.0_glibc2.27.tar.xz
            URL_HASH SHA256=824f19b032d8e64de3a32bbaf7258a7db69e646db217fe752e6ce7c25923ec3f
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            # On install: Copy contents of each subfolder to the build folder
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${FAST_EXTERNAL_INSTALL_DIR}/include COMMAND
                ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib ${FAST_EXTERNAL_INSTALL_DIR}/lib COMMAND
                ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/licences ${FAST_EXTERNAL_INSTALL_DIR}/licences
    )
endif()
endif(FAST_BUILD_ALL_DEPENDENCIES)

list(APPEND LIBRARIES ${CMAKE_SHARED_LIBRARY_PREFIX}realsense2${CMAKE_SHARED_LIBRARY_SUFFIX})
list(APPEND FAST_EXTERNAL_DEPENDENCIES realsense)
