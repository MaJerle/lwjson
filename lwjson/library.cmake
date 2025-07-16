#
# LIB_PREFIX: LWJSON
#
# This file provides set of variables for end user
# and also generates one (or more) libraries, that can be added to the project using target_link_libraries(...)
#
# Before this file is included to the root CMakeLists file (using include() function), user can set some variables:
#
# LWJSON_OPTS_FILE: If defined, it is the path to the user options file. If not defined, one will be generated for you automatically
# LWJSON_COMPILE_OPTIONS: If defined, it provide compiler options for generated library.
# LWJSON_COMPILE_DEFINITIONS: If defined, it provides "-D" definitions to the library build
#

# Custom include directory
set(LWJSON_CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib_inc)

# Setup generic source files
set(lwjson_core_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/lwjson/lwjson.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwjson/lwjson_stream.c
)

# Debug sources
set(lwjson_debug_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/lwjson/lwjson_debug.c
)

# Setup include directories
set(lwjson_include_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    ${LWJSON_CUSTOM_INC_DIR}
)

# Register core library to the system
add_library(lwjson)
target_sources(lwjson PRIVATE ${lwjson_core_SRCS})
target_include_directories(lwjson PUBLIC ${lwjson_include_DIRS})
target_compile_options(lwjson PRIVATE ${LWJSON_COMPILE_OPTIONS})
target_compile_definitions(lwjson PRIVATE ${LWJSON_COMPILE_DEFINITIONS})

# Register lwjson debug module
add_library(lwjson_debug)
target_sources(lwjson_debug PRIVATE ${lwjson_debug_SRCS})
target_include_directories(lwjson_debug PUBLIC ${lwjson_include_DIRS})
target_compile_options(lwjson_debug PRIVATE ${LWJSON_COMPILE_OPTIONS})
target_compile_definitions(lwjson_debug PRIVATE ${LWJSON_COMPILE_DEFINITIONS})
target_link_libraries(lwjson_debug PUBLIC lwjson)

# Create config file if user didn't provide one info himself
if(NOT LWJSON_OPTS_FILE)
    message(STATUS "Using default lwjson_opts.h file")
    set(LWJSON_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/src/include/lwjson/lwjson_opts_template.h)
else()
    message(STATUS "Using custom lwjson_opts.h file from ${LWJSON_OPTS_FILE}")
endif()

configure_file(${LWJSON_OPTS_FILE} ${LWJSON_CUSTOM_INC_DIR}/lwjson_opts.h COPYONLY)
