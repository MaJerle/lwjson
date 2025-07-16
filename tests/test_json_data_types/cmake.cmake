# CMake include file

# Add more sources
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/test_json_data_types.c
)

# Options file
set(LWJSON_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/lwjson_opts.h)