cmake_minimum_required( VERSION 3.1.0 )

project( vmem2 )

if( MINGW )
    if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
        find_library( LIBVLCCORE_LIBRARY NAMES vlccore.x64 libvlccore.x64
            HINTS "$ENV{LIBVLC_LIBRARY_PATH}"
                  "${CMAKE_CURRENT_SOURCE_DIR}/deps/libvlc-sdk/lib/mingw"
            NO_SYSTEM_ENVIRONMENT_PATH
            )
    else()
        find_library( LIBVLCCORE_LIBRARY NAMES vlccore libvlccore
            HINTS "$ENV{LIBVLC_LIBRARY_PATH}"
                  "${CMAKE_CURRENT_SOURCE_DIR}/deps/libvlc-sdk/lib/msvc"
            NO_SYSTEM_ENVIRONMENT_PATH
            )
    endif()
else()
    message( FATAL_ERROR "Only MINGW supported at the moment." )
endif()

set( CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE PATH "install to" FORCE )

add_subdirectory( plugin )
add_subdirectory( facade )
