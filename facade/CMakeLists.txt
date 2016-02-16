cmake_minimum_required( VERSION 3.1.0 )

file( GLOB SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ../include/[^.]*.h
    [^.]*.h
    [^.]*.cpp
    )

add_library( ${PROJECT_NAME} SHARED ${SOURCE_FILES} )
set_target_properties( ${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON )
set_target_properties( ${PROJECT_NAME} PROPERTIES PREFIX "" )
set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,--output-def,${PROJECT_NAME}.def -static" )
target_include_directories( ${PROJECT_NAME} PUBLIC "../include" )
target_include_directories( ${PROJECT_NAME} PUBLIC "../deps/libvlc-sdk/include" )
target_include_directories( ${PROJECT_NAME} PRIVATE "../deps/libvlc-sdk/include/vlc/plugins" )

find_library( LIBVLCCORE_LIBRARY NAMES vlccore libvlccore
    HINTS "$ENV{LIBVLC_LIBRARY_PATH}"
          "${CMAKE_CURRENT_SOURCE_DIR}/deps/libvlc-sdk/lib/"
    PATHS  "/Applications/VLC.app/Contents/MacOS/lib"
    )

target_link_libraries( ${PROJECT_NAME} ${LIBVLCCORE_LIBRARY} )
