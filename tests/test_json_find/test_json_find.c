#include <stdio.h>
#include "lwjson/lwjson.h"

#define RUN_TEST(c)                                                                                                    \
    if ((c)) {                                                                                                         \
        ++test_passed;                                                                                                 \
    } else {                                                                                                           \
        printf("Test failed on line %d\r\n", __LINE__);                                                                \
        ++test_failed;                                                                                                 \
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
    const lwjson_token_t* token;
    const char* json_str = "\
    {\
        \"my_arr\":[\
            {\"num\":1,\"str\":\"first_entry\"},\
            {\"num\":2,\"str\":\"second_entry\"},\
            {\"num\":3,\"str\":\"third_entry\"},\
            [\"abc\", \"def\"],\
            [true, false, null],\
            [123, -123, 987]\
        ],\
        \"my_obj\": {\
            \"key_true\": true,\
            \"key_true\": false,\
            \"arr\": [\
                [1, 2, 3],\
                [true, false, null],\
                [{\"my_key\":\"my_text\"}]\
            ],\
            \"ustr\":\"\\t\\u1234abc\"\
        }\
    }\
    ";

    printf("---\r\nTest JSON find..\r\n");
    if (lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens)) != lwjsonOK) {
        printf("JSON init failed\r\n");
        return -1;
    }

    /* First parse JSON */
    if (lwjson_parse(&lwjson, json_str) != lwjsonOK) {
        printf("Could not parse JSON string..\r\n");
        return -1;
    }

    /* Run all tests */
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr")) != NULL && token->type == LWJSON_TYPE_ARRAY);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#")) == NULL);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.")) == NULL);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#0")) != NULL && token->type == LWJSON_TYPE_OBJECT);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#1")) != NULL && token->type == LWJSON_TYPE_OBJECT);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#2")) != NULL && token->type == LWJSON_TYPE_OBJECT);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#0.str")) != NULL && token->type == LWJSON_TYPE_STRING
             && strncmp(token->u.str.token_value, "first_entry", 11) == 0);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#1.str")) != NULL && token->type == LWJSON_TYPE_STRING
             && strncmp(token->u.str.token_value, "second_entry", 12) == 0);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#2.str")) != NULL && token->type == LWJSON_TYPE_STRING
             && strncmp(token->u.str.token_value, "third_entry", 11) == 0);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#3")) != NULL && token->type == LWJSON_TYPE_ARRAY);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#3.#1")) != NULL && token->type == LWJSON_TYPE_STRING
             && strncmp(token->u.str.token_value, "def", 3) == 0);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#3.#0")) != NULL && token->type == LWJSON_TYPE_STRING
             && strncmp(token->u.str.token_value, "abc", 3) == 0);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#3.#")) == NULL);

    /* Use EX version to search for tokens */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj")) != NULL /* First search for my_obj in root JSON */
             && (token = lwjson_find_ex(&lwjson, token, "key_true"))
                    != NULL /* Use token for relative search and search for key_true only */
             && token->type == LWJSON_TYPE_TRUE);

    /* Deep search */

    /* Search for first match in any array */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#.#.my_key")) != NULL
             && token->type == LWJSON_TYPE_STRING && strncmp(token->u.str.token_value, "my_text", 7) == 0);

    /* Search for match in specific array keys */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#2.#0.my_key")) != NULL
             && token->type == LWJSON_TYPE_STRING && strncmp(token->u.str.token_value, "my_text", 7) == 0);

    /* Search for match in specific array keys = must return NULL, no index = 1 in second array */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#2.#1.my_key")) == NULL);

    /* Use partial searches */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj")) != NULL
             && (token = lwjson_find_ex(&lwjson, token, "arr")) != NULL
             && (token = lwjson_find_ex(&lwjson, token, "#2")) != NULL
             && (token = lwjson_find_ex(&lwjson, token, "#0")) != NULL
             && (token = lwjson_find_ex(&lwjson, token, "my_key")) != NULL && token->type == LWJSON_TYPE_STRING
             && strncmp(token->u.str.token_value, "my_text", 7) == 0);

    /* Search for match in specific array keys and check for string length field */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.ustr")) != NULL && token->type == LWJSON_TYPE_STRING
             && lwjson_get_val_string_length(token) == 11
             && strncmp(token->u.str.token_value, "\\t\\u1234abc", 11) == 0);

    /* Check string compare */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#.#.my_key")) != NULL
             && lwjson_string_compare(token, "my_text"));
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#.#.my_key")) != NULL
             && lwjson_string_compare_n(token, "my_text", 3));
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#.#.my_key")) != NULL
             && !lwjson_string_compare_n(token, "my_stext", 4)); /* Must be a fail */

    /* Call this once JSON usage is finished */
    lwjson_free(&lwjson);

    /* Print results */
    printf("Find function test result pass/fail: %d/%d\r\n\r\n", (int)test_passed, (int)test_failed);
    return test_failed > 0 ? -1 : 0;
}
