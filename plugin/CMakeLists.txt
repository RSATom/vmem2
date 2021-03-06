cmake_minimum_required( VERSION 3.1.0 )

set( PLUGIN_NAME ${PROJECT_NAME}_plugin )

add_definitions( -DHAVE_CONFIG_H -DMODULE_STRING="${PROJECT_NAME}" -D__PLUGIN__ )

file( GLOB SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ../include/[^.]*.h
    [^.]*.h
    [^.]*.cpp
    )

add_library( ${PLUGIN_NAME} SHARED ${SOURCE_FILES} )
target_compile_features( ${PLUGIN_NAME} PRIVATE cxx_nullptr )
set_target_properties( ${PLUGIN_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON )
set_target_properties( ${PLUGIN_NAME} PROPERTIES LINK_FLAGS "-static")
target_include_directories( ${PLUGIN_NAME} PRIVATE "../include" )
target_include_directories( ${PLUGIN_NAME} PRIVATE "../deps/libvlc-sdk/include/vlc/plugins" )

target_link_libraries( ${PLUGIN_NAME} ${LIBVLCCORE_LIBRARY} )

install( TARGETS ${PLUGIN_NAME}
    RUNTIME DESTINATION bin/plugins/video_output
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )
