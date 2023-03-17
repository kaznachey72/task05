cmake_minimum_required(VERSION 3.14)

set(ENTITY_NAME mywavsrc)

find_package(PkgConfig)
pkg_check_modules(GST REQUIRED 
    gstreamer-1.0>=1.4
    gstreamer-sdp-1.0>=1.4
    gstreamer-video-1.0>=1.4
    gstreamer-app-1.0>=1.4
)

add_library(${ENTITY_NAME} SHARED
    gstmywavsrc.c
)

set_property(TARGET ${ENTITY_NAME} PROPERTY C_STANDARD 11)
target_compile_options(${ENTITY_NAME} PUBLIC -Wall -Wextra -Wpedantic)
target_include_directories(${ENTITY_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${ENTITY_NAME} PUBLIC "${GST_INCLUDE_DIRS}")
set_target_properties(${ENTITY_NAME} PROPERTIES COMPILE_FLAGS ${GST_CFLAGS} )
target_link_libraries(${ENTITY_NAME} ${GST_LIBRARIES})
