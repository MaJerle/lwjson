# CMake include file

# Add more sources
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/test_parse_token_count.c
)

# Options file
set(LWJSON_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/lwjson_opts.h)