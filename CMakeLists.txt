cmake_minimum_required(VERSION 3.22)

# Setup project
project(LwLibPROJECT)

if(NOT PROJECT_IS_TOP_LEVEL)
    add_subdirectory(lwjson)
else()
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # Set as executable
    add_executable(${PROJECT_NAME})

    # Add key executable block
    target_sources(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/dev/main.c
        ${CMAKE_CURRENT_LIST_DIR}/test/test.c
        ${CMAKE_CURRENT_LIST_DIR}/examples/example_minimal.c
        ${CMAKE_CURRENT_LIST_DIR}/examples/example_traverse.c
        ${CMAKE_CURRENT_LIST_DIR}/examples/example_stream.c
        ${CMAKE_CURRENT_LIST_DIR}/trial_env/trial_run.c
    )

    # Add key include paths
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/dev
    )

    # Compilation definition information
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        WIN32
        _DEBUG
        CONSOLE
    )

    # Compiler options
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    # Add subdir with lwjson and link to project
    set(LWJSON_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/dev/lwjson_opts.h)
    add_subdirectory(lwjson)
    target_link_libraries(${PROJECT_NAME} lwjson)
    target_link_libraries(${PROJECT_NAME} lwjson_debug)
endif()