#include <stdio.h>
#include "lwjson/lwjson.h"

#define RUN_TEST(exp_res, json_str)                                                                                    \
    if (lwjson_parse(&lwjson, (json_str)) == (exp_res)) {                                                              \
        ++test_passed;                                                                                                 \
    } else {                                                                                                           \
        ++test_failed;                                                                                                 \
        printf("Test failed for input %s on line %d\r\n", json_str, __LINE__);                                         \
    }
#define RUN_TEST_EX(exp_res, json_str, len)                                                                            \
    if (lwjson_parse_ex(&lwjson, (json_str), (len)) == (exp_res)) {                                                    \
        ++test_passed;                                                                                                 \
    } else {                                                                                                           \
        ++test_failed;                                                                                                 \
        printf("Test failed for input %s on line %d\r\n", json_str, __LINE__);                                         \
    }

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

    printf("JSON parse test\r\n");
    if (lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens)) != lwjsonOK) {
        printf("JSON init failed\r\n");
        return -1;
    }

    /* Run JSON parse tests that must succeed */
    RUN_TEST(lwjsonOK, "{}");
    RUN_TEST(lwjsonOK, "{ }");
    RUN_TEST(lwjsonOK, "{}\r\n");
    RUN_TEST(lwjsonOK, "{ }\r\n");
    RUN_TEST(lwjsonOK, "{\t}\r\n");
    RUN_TEST(lwjsonOK, "{\t }\r\n");
    RUN_TEST(lwjsonOK, "[1,2,3,4]");
    RUN_TEST(lwjsonOK, "{\"k\":[]}");
    RUN_TEST(lwjsonOK, "{\"k\":[1]}");
    RUN_TEST(lwjsonOK, "{\"k\":[1,2]}");
    RUN_TEST(lwjsonOK, "{\"k\":[1,]}");
    RUN_TEST(lwjsonOK, "{\"k\":[1,[1,2]]}");
    RUN_TEST(lwjsonOK, "{\"k\":false}");
    RUN_TEST(lwjsonOK, "{\"k\":true}");
    RUN_TEST(lwjsonOK, "{\"k\":null}");
    RUN_TEST(lwjsonOK, "{\"k\" :null}");
    RUN_TEST(lwjsonOK, "{\"k\" : null}");
    RUN_TEST(lwjsonOK, "{ \"k\": null }");
    RUN_TEST(lwjsonOK, "{ \"k\": null }");
    RUN_TEST(lwjsonOK, "{\"k\":\"Stringgg\"}");
    RUN_TEST(lwjsonOK, "{\"k\":\"Stri\\\"nggg with quote inside\"}");
    RUN_TEST(lwjsonOK, "{\"k\":{\"b\":1E5,\t\r\n\"c\":1.3E5\r\n}\r\n}");

    /* Arrays */
    RUN_TEST(lwjsonOK, "[]");
    RUN_TEST(lwjsonOK, "[ ]");
    RUN_TEST(lwjsonOK, "[[],[]]");
    RUN_TEST(lwjsonOK, "[[],[],{}]");
    RUN_TEST(lwjsonERRJSON, "[");
    RUN_TEST(lwjsonERRJSON, "[\"abc\":\"test\"]");
    RUN_TEST(lwjsonERRJSON, "]");
    RUN_TEST(lwjsonERRJSON, "[[,[]]");
    RUN_TEST(lwjsonERRJSON, "[,[]]");
    RUN_TEST(lwjsonERRJSON, "[[],[,{}]");
    RUN_TEST(lwjsonERRJSON, "{[0,1,2]}");
    RUN_TEST(lwjsonERRJSON, "{1,2}");

    /* Check specials */
    RUN_TEST(lwjsonOK, "{\"k\":\"\\t\"}");
    RUN_TEST(lwjsonOK, "{\"k\":\"\\b\"}");
    RUN_TEST(lwjsonOK, "{\"k\":\"\\r\"}");
    RUN_TEST(lwjsonOK, "{\"k\":\"\\n\"}");
    RUN_TEST(lwjsonOK, "{\"k\":\"\\f\"}");
    RUN_TEST(lwjsonOK, "{\"k\":\"\\\\\"}");
    RUN_TEST(lwjsonOK, "{\"k\":\"\\u1234\"}");
    RUN_TEST(lwjsonOK, "{\"k\":\"\\uabcd\"}");
    RUN_TEST(lwjsonOK, "{\"k\":\"\\uAbCd\"}");
    RUN_TEST(lwjsonOK, "{\"k\":\"\\u1abc\"}");
    RUN_TEST(lwjsonERRJSON, "{\"k\":\"\\u1aGc\"}");
    RUN_TEST(lwjsonERRJSON, "{\"k\":\"\\u\t\n\n\n\"}");
    RUN_TEST(lwjsonERRJSON, "{\"k\":\"\\u\"}");
    RUN_TEST(lwjsonERRJSON, "{\"k\":\"\\u1\"}");
    RUN_TEST(lwjsonERRJSON, "{\"k\":\"\\u12\"}");
    RUN_TEST(lwjsonERRJSON, "{\"k\":\"\\u123\"}");
    RUN_TEST(lwjsonERRJSON, "{\"k\":\"\\a\"}");

    /* Run JSON tests to fail */
    RUN_TEST(lwjsonERRPAR, "");
    RUN_TEST(lwjsonERRJSON, "{[]}");          /* Array without key inside object */
    RUN_TEST(lwjsonERRJSON, "{\"k\":False}"); /* False value must be all lowercase */
    RUN_TEST(lwjsonERRJSON, "{\"k\":True}");  /* True value must be all lowercase */
    RUN_TEST(lwjsonERRJSON, "{\"k\":nUll}");  /* Null value must be all lowercase */
    RUN_TEST(lwjsonERRJSON, "{\"k\"1}");      /* Missing separator */
    RUN_TEST(lwjsonERRJSON, "{\"k\"1}");      /* Missing separator */
    RUN_TEST(lwjsonERRJSON, "{k:1}");         /* Property name must be string */
    RUN_TEST(lwjsonERRJSON, "{k:0.}");        /* Wrong number format */

    /* Tests with custom len */
    RUN_TEST_EX(lwjsonOK, "[1,2,3,4]abc", 9);   /* Limit input len to JSON-only */
    RUN_TEST_EX(lwjsonERR, "[1,2,3,4]abc", 10); /* Too long input for JSON string.. */
    RUN_TEST_EX(lwjsonOK, "[1,2,3,4]",
                15); /* String ends earlier than what is input data len indicating = OK if JSON is valid */

    /* Print results */
    printf("JSON parse test result pass/fail: %d/%d\r\n\r\n", (int)test_passed, (int)test_failed);
    return test_failed > 0 ? -1 : 0;
}
