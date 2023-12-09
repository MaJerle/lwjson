# 
# This file provides set of variables for end user
# and also generates one (or more) libraries, that can be added to the project using target_link_libraries(...)
#
# Before this file is included to the root CMakeLists file (using include() function), user can set some variables:
#
# LWJSON_OPTS_DIR: If defined, it should set the folder path where options file shall be generated.
# LWJSON_COMPILE_OPTIONS: If defined, it provide compiler options for generated library.
# LWJSON_COMPILE_DEFINITIONS: If defined, it provides "-D" definitions to the library build
#

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
)

# Register core library to the system
add_library(lwjson INTERFACE)
target_sources(lwjson PUBLIC ${lwjson_core_SRCS})
target_include_directories(lwjson INTERFACE ${lwjson_include_DIRS})
target_compile_options(lwjson PRIVATE ${LWJSON_COMPILE_OPTIONS})
target_compile_definitions(lwjson PRIVATE ${LWJSON_COMPILE_DEFINITIONS})

# Register lwjson debug module
add_library(lwjson_debug INTERFACE)
target_sources(lwjson_debug PUBLIC ${lwjson_debug_SRCS})
target_include_directories(lwjson_debug INTERFACE ${lwjson_include_DIRS})
target_compile_options(lwjson_debug PRIVATE ${LWJSON_COMPILE_OPTIONS})
target_compile_definitions(lwjson_debug PRIVATE ${LWJSON_COMPILE_DEFINITIONS})

# Create config file
if(DEFINED LWJSON_OPTS_DIR AND NOT EXISTS ${LWJSON_OPTS_DIR}/lwjson_opts.h)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/src/include/lwjson/lwjson_opts_template.h ${LWJSON_OPTS_DIR}/lwjson_opts.h COPYONLY)
endif()