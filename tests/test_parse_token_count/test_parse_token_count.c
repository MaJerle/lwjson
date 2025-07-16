#include <stdio.h>
#include "lwjson/lwjson.h"

#define RUN_TEST(exp_token_count, json_str)                                                                            \
    do {                                                                                                               \
        if (lwjson_parse(&lwjson, (json_str)) == lwjsonOK) {                                                           \
            if ((lwjson.next_free_token_pos + 1) == exp_token_count) {                                                 \
                ++test_passed;                                                                                         \
            } else {                                                                                                   \
                ++test_failed;                                                                                         \
                printf("Test failed for JSON token count on input %s\r\n", (json_str));                                \
            }                                                                                                          \
            lwjson_free(&lwjson);                                                                                      \
        } else {                                                                                                       \
            ++test_failed;                                                                                             \
            printf("Test failed for JSON parse on input %s\r\n", (json_str));                                          \
        }                                                                                                              \
    } while (0)

/**
 * \brief           Path and type parse check
 */
typedef struct {
    const char* path;   /*!< Path in parsed JSON */
    lwjson_type_t type; /*!< expected data type in JSON */
} test_path_type_t;

/* LwJSON instance and tokens */
static lwjson_token_t tokens[4096];
static lwjson_t lwjson;

/**
 * \brief           Run all tests entry point
 */
int
test_run(void) {
    size_t test_failed = 0, test_passed = 0;

    printf("---\r\nTest JSON token count..\r\n");
    if (lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens)) != lwjsonOK) {
        printf("JSON init failed\r\n");
        return -1;
    }

    /* Run token count tests */
    RUN_TEST(2, "{\"k\":1}");
    RUN_TEST(3, "{\"k\":1,\"k\":2}");
    RUN_TEST(4, "{\"k\":1,\"k\":[1]}");
    RUN_TEST(5, "{\"k\":1,\"k\":[1,2]}");
    RUN_TEST(6, "{\"k\":1,\"k\":[1,2,5]}");
    RUN_TEST(12, "{\"k\":1,\"k\":[[1,2],[3,4],[5,6]]}");
    RUN_TEST(4, "{\"k\":{\"k\":{\"k\":[]}}}");
    RUN_TEST(6, "{\"k\":{\"k\":{\"k\":[[[]]]}}}");
    RUN_TEST(6, "{\"k\":[{\"k\":1},{\"k\":2}]}");

    /* Print results */
    printf("JSON token count test result pass/fail: %d/%d\r\n\r\n", (int)test_passed, (int)test_failed);
    return test_failed > 0 ? -1 : 0;
}
