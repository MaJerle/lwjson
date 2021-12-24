cmake_minimum_required(VERSION 3.13)

# Debug message
message("LWJSON: Entering lib source CMakeLists.txt")

# Set library name
set(LWJSON_LIB_NAME "lwjson")

# Register library to the system
add_library(${LWJSON_LIB_NAME} INTERFACE)

# Setup generic source files
target_sources(${LWJSON_LIB_NAME} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/lwjson/lwjson.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwjson/lwjson.c
    )

# Setup include directories
target_include_directories(${LWJSON_LIB_NAME} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    )
