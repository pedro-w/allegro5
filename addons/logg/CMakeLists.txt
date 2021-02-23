#-----------------------------------------------------------------------------#
#
# logg
#

find_package(Vorbis)
if(NOT VORBIS_FOUND)
    return()
endif()

include_directories(SYSTEM ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})

set(LOGG_SOURCES
    logg.c
    )

set(LOGG_HEADERS
    logg.h
    )

set_source_files_properties(${LOGG_HEADERS}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Headers
    )

add_our_library(logg ${ADDON_LINKAGE}
    ${LOGG_SOURCES}
    ${LOGG_HEADERS}
    )

if (BUILD_SHARED_LIBS)
    set (logg_dynamic "-DLOGG_DYNAMIC")
endif(BUILD_SHARED_LIBS)

set_target_properties(logg
    PROPERTIES
    COMPILE_FLAGS "-DLOGG_SRC_BUILD ${logg_dynamic}"
    LINK_FLAGS "${ALLEGRO_LINK_FLAGS}"
    VERSION ${ALLEGRO_VERSION}
    SOVERSION ${ALLEGRO_SOVERSION}
    )

set_our_framework_properties(logg logg)

target_link_libraries(logg allegro ${VORBIS_LIBRARIES})

sanitize_cmake_link_flags(static_link_with ${VORBIS_LIBRARIES})
set_target_properties(logg
    PROPERTIES
    static_link_with "${static_link_with}"
    )

install_our_library(logg)
if(NOT WANT_FRAMEWORKS)
    install_our_headers(${LOGG_HEADERS})
endif(NOT WANT_FRAMEWORKS)

#-----------------------------------------------------------------------------#
#
# Examples
#

if(WANT_EXAMPLES)
    add_executable(play_ogg play_ogg.c)
    target_link_libraries(play_ogg logg)
    set_target_properties(play_ogg
      PROPERTIES
      COMPILE_FLAGS " ${logg_dynamic}")
    add_executable(stream_ogg stream_ogg.c)
    target_link_libraries(stream_ogg logg)
    set_target_properties(stream_ogg
      PROPERTIES
      COMPILE_FLAGS " ${logg_dynamic}")
endif(WANT_EXAMPLES)

#-----------------------------------------------------------------------------#
# vim: set sts=4 sw=4 et:
