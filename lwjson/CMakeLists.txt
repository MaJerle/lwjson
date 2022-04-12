cmake_minimum_required(VERSION 3.22)

#Debug message
message("Entering ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")

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

# Register lwjson debug module
add_library(lwjson_debug INTERFACE)
target_sources(lwjson_debug PUBLIC ${lwjson_debug_SRCS})
target_include_directories(lwjson_debug INTERFACE ${lwjson_include_DIRS})

#Debug message
message("Exiting ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")