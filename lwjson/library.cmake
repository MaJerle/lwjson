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