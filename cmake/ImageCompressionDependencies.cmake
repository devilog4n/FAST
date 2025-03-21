# Download dependencies required for various image compression
if(WIN32)
    fast_download_dependency(jpegxl
        0.11.0
        51aec03201152d99ca9c6a561b3c28e0e1c1043d9488c9bd7b590b03412347e0
        jxl.lib jxl_threads.lib
    )
elseif(APPLE)
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        fast_download_dependency(jpegxl
            0.11.0
            2da83a5bcba122116cdadf98af9f4cecfeb45595dd2989a4cc95281a88f40d57
            jxl.dylib jxl_threads.dylib
        )
    else()
        fast_download_dependency(jpegxl
            0.11.0
            0828fc48b0b9262e0db6ae3602c0afcd55626f0f388826a38c245c6f8911f38e
            jxl.dylib jxl_threads.dylib
        )
    endif()
else()
    fast_download_dependency(jpegxl
        0.11.0
        a9bdb10ceddceb57892e9f6526cd8f62ad4a469e9f6941a687f8af6eb425b172
        libjxl.so libjxl_threads.so
    )
endif()
