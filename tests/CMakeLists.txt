cmake_minimum_required(VERSION 3.22)

# Setup project
project(LwLibPROJECT)

# Set default compile flags for GCC
if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
endif(CMAKE_COMPILER_IS_GNUCXX)

enable_testing()
add_executable(${CMAKE_PROJECT_NAME})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/test_main.c
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/
)

# Include file that can add more sources and prepare lib parameters
include(${TEST_CMAKE_FILE_NAME})

# Add subdir with lwjson and link to project
add_subdirectory("../lwjson" lwjson)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC lwjson)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC lwjson_debug)
target_compile_definitions(lwjson PUBLIC LWJSON_DEV)

# Add test
add_test(NAME Test COMMAND $<TARGET_FILE:${CMAKE_PROJECT_NAME}>)
