#include <stdio.h>
#include "lwjson/lwjson.h"

/**
 * \brief           Path and type parse check
 */
typedef struct {
    const char* path;                           /*!< Path in parsed JSON */
    lwjson_type_t type;                         /*!< expected data type in JSON */
} test_path_type_t;

/* LwJSON instance and tokens */
static lwjson_token_t tokens[4096];
static lwjson_t lwjson;

/* Test JSON parsing */
static void
test_json_parse(void) {
    size_t test_failed = 0, test_passed = 0;

    printf("JSON parse test\r\n");

#define RUN_TEST(exp_res, json_str)             if (lwjson_parse(&lwjson, (json_str)) == (exp_res)) { ++test_passed; } else { ++test_failed; printf("Test failed for input %s on line %d\r\n", json_str, __LINE__); }
#define RUN_TEST_EX(exp_res, json_str, len)     if (lwjson_parse_ex(&lwjson, (json_str), (len)) == (exp_res)) { ++test_passed; } else { ++test_failed; printf("Test failed for input %s on line %d\r\n", json_str, __LINE__); }

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
    RUN_TEST(lwjsonERRJSON, "{[]}");            /* Array without key inside object */
    RUN_TEST(lwjsonERRJSON, "{\"k\":False}");   /* False value must be all lowercase */
    RUN_TEST(lwjsonERRJSON, "{\"k\":True}");    /* True value must be all lowercase */
    RUN_TEST(lwjsonERRJSON, "{\"k\":nUll}");    /* Null value must be all lowercase */
    RUN_TEST(lwjsonERRJSON, "{\"k\"1}");        /* Missing separator */
    RUN_TEST(lwjsonERRJSON, "{\"k\"1}");        /* Missing separator */
    RUN_TEST(lwjsonERRJSON, "{k:1}");           /* Property name must be string */
    RUN_TEST(lwjsonERRJSON, "{k:0.}");          /* Wrong number format */

    /* Tests with custom len */
    RUN_TEST_EX(lwjsonOK, "[1,2,3,4]abc", 9);   /* Limit input len to JSON-only */
    RUN_TEST_EX(lwjsonERR, "[1,2,3,4]abc", 10); /* Too long input for JSON string.. */
    RUN_TEST_EX(lwjsonOK, "[1,2,3,4]", 15);     /* String ends earlier than what is input data len indicating = OK if JSON is valid */

#undef RUN_TEST
#undef RUN_TEST_EX

    /* Print results */
    printf("JSON parse test result pass/fail: %d/%d\r\n\r\n",
        (int)test_passed, (int)test_failed);
}

/* Test number of tokens necessary for parsing */
static void
test_parse_token_count(void) {
    size_t test_failed = 0, test_passed = 0;

    printf("---\r\nTest JSON token count..\r\n");

#define RUN_TEST(exp_token_count, json_str)     do {                \
        if (lwjson_parse(&lwjson, (json_str)) == lwjsonOK) {        \
            if ((lwjson.next_free_token_pos + 1) == exp_token_count) {  \
                ++test_passed;                                      \
            } else {                                                \
                ++test_failed;                                      \
                printf("Test failed for JSON token count on input %s\r\n", (json_str));\
            }                                                       \
            lwjson_free(&lwjson);                                   \
        } else {                                                    \
            ++test_failed;                                          \
            printf("Test failed for JSON parse on input %s\r\n", (json_str));\
        }                                                           \
    } while (0)

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

#undef RUN_TEST

    /* Print results */
    printf("JSON token count test result pass/fail: %d/%d\r\n\r\n",
        (int)test_passed, (int)test_failed);
}

/* Test JSON data types */
static void
test_json_data_types(void) {
    const lwjson_token_t* t;
    size_t test_failed = 0, test_passed = 0;

    /* Input JSON string */
    const char* json_complete = ""
        "{"
        "   \"int\": {"
        "       \"num1\": 1234,"
        "       \"num2\": -1234,"
        "       \"num3\": 0"
        "   },"
#if LWJSON_CFG_COMMENTS
        " /* This is my comment... */"
#endif /* LWJSON_CFG_COMMENTS */
        "   \"real\": {"
        "       \"num1\":123.4,"
        "       \"num2\":-123.4,"
        "       \"num3\":123E3,"
        "       \"num4\":123e4,"
        "       \"num5\":-123E3,"
        "       \"num6\":-123e4,"
        "       \"num7\":123E-3,"
        "       \"num8\":123e-4,"
        "       \"num9\":-123E-3,"
        "       \"num10\":-123e-4,"
        "       \"num11\":123.12E3,"
        "       \"num12\":123.1e4,"
        "       \"num13\":-123.0E3,"
        "       \"num14\":-123.1e4,"
        "       \"num15\":123.1E-3,"
        "       \"num16\":123.1235e-4,"
        "       \"num17\":-123.324342E-3,"
        "       \"num18\":-123.3232e-4,"
        "   },"
        "   \"obj\": {"
        "       \"obj1\":{},"
        "       \"obj2\":[],"
        "       \"obj3\":{"
        "           \"key1\":[],"
        "           \"key2\":\"string\","
        "        },"
        "    },"
        "   \"bool\": {"
        "       \"true\":true,"
        "       \"false\":false"
        "   },"
        "   \"null\":null,"
        "   \"array\":[],"
        "}";

    /* JSON paths */
    const test_path_type_t paths_types[] = {
        /* Integer types */
        {"int", LWJSON_TYPE_OBJECT},
        {"int.num1", LWJSON_TYPE_NUM_INT},
        {"int.num2", LWJSON_TYPE_NUM_INT},
        {"int.num3", LWJSON_TYPE_NUM_INT},

        /* Real types */
        {"real", LWJSON_TYPE_OBJECT},
        {"real.num1", LWJSON_TYPE_NUM_REAL},
        {"real.num2", LWJSON_TYPE_NUM_REAL},
        {"real.num3", LWJSON_TYPE_NUM_REAL},
        {"real.num4", LWJSON_TYPE_NUM_REAL},
        {"real.num5", LWJSON_TYPE_NUM_REAL},
        {"real.num6", LWJSON_TYPE_NUM_REAL},
        {"real.num7", LWJSON_TYPE_NUM_REAL},
        {"real.num8", LWJSON_TYPE_NUM_REAL},
        {"real.num9", LWJSON_TYPE_NUM_REAL},
        {"real.num10", LWJSON_TYPE_NUM_REAL},
        {"real.num11", LWJSON_TYPE_NUM_REAL},
        {"real.num12", LWJSON_TYPE_NUM_REAL},
        {"real.num13", LWJSON_TYPE_NUM_REAL},
        {"real.num14", LWJSON_TYPE_NUM_REAL},
        {"real.num15", LWJSON_TYPE_NUM_REAL},
        {"real.num16", LWJSON_TYPE_NUM_REAL},
        {"real.num17", LWJSON_TYPE_NUM_REAL},
        {"real.num18", LWJSON_TYPE_NUM_REAL},

        /* Object */
        {"obj", LWJSON_TYPE_OBJECT},
        {"obj.obj1", LWJSON_TYPE_OBJECT},
        {"obj.obj2", LWJSON_TYPE_ARRAY},
        {"obj.obj3", LWJSON_TYPE_OBJECT},
        {"obj.obj3.key1", LWJSON_TYPE_ARRAY},
        {"obj.obj3.key2", LWJSON_TYPE_STRING},

        /* Boolean */
        {"bool", LWJSON_TYPE_OBJECT},
        {"bool.true", LWJSON_TYPE_TRUE},
        {"bool.false", LWJSON_TYPE_FALSE},

        /* Null check */
        {"null", LWJSON_TYPE_NULL},

        /* Array check */
        {"array", LWJSON_TYPE_ARRAY},
    };

    printf("---\r\nTest JSON data types..\r\n");
    
    /* First parse JSON */
    if (lwjson_parse(&lwjson, json_complete) != lwjsonOK) {
        printf("Could not parse LwJSON data types..\r\n");
        return;
    }

    /* Now that it is parsed, check all input keys */
    for (size_t i = 0; i < LWJSON_ARRAYSIZE(paths_types); ++i) {
        t = lwjson_find(&lwjson, paths_types[i].path);
        if (t == NULL) {
            printf("Could not find entry for path \"%s\"\r\n", paths_types[i].path);
            ++test_failed;
            continue;
        }
        if (t->type == paths_types[i].type) {
            ++test_passed;
        } else {
            printf("Type missmatch for path \"%s\"\r\n", paths_types[i].path);
            ++test_failed;
        }
    }

    /* Call this once JSON usage is finished */
    lwjson_free(&lwjson);

    /* Print results */
    printf("Data type test result pass/fail: %d/%d\r\n\r\n",
        (int)test_passed, (int)test_failed);
}

/* Test find function */
static void
test_find_function(void) {
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
    
    /* First parse JSON */
    if (lwjson_parse(&lwjson, json_str) != lwjsonOK) {
        printf("Could not parse JSON string..\r\n");
        return;
    }

 #define RUN_TEST(c)     if ((c)) { ++test_passed; } else { ++test_failed; printf("Test failed on line %d\r\n", __LINE__); }

    /* Run all tests */
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr")) != NULL
                && token->type == LWJSON_TYPE_ARRAY);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#")) == NULL);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.")) == NULL);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#0")) != NULL
                && token->type == LWJSON_TYPE_OBJECT);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#1")) != NULL
                && token->type == LWJSON_TYPE_OBJECT);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#2")) != NULL
                && token->type == LWJSON_TYPE_OBJECT);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#0.str")) != NULL
                && token->type == LWJSON_TYPE_STRING
                && strncmp(token->u.str.token_value, "first_entry", 11) == 0);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#1.str")) != NULL
                && token->type == LWJSON_TYPE_STRING
                && strncmp(token->u.str.token_value, "second_entry", 12) == 0);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#2.str")) != NULL
                && token->type == LWJSON_TYPE_STRING
                && strncmp(token->u.str.token_value, "third_entry", 11) == 0);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#3")) != NULL
                && token->type == LWJSON_TYPE_ARRAY);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#3.#1")) != NULL
                && token->type == LWJSON_TYPE_STRING
                && strncmp(token->u.str.token_value, "def", 3) == 0);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#3.#0")) != NULL
                && token->type == LWJSON_TYPE_STRING
                && strncmp(token->u.str.token_value, "abc", 3) == 0);
    RUN_TEST((token = lwjson_find(&lwjson, "my_arr.#3.#")) == NULL);

    /* Use EX version to search for tokens */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj")) != NULL              /* First search for my_obj in root JSON */
                && (token = lwjson_find_ex(&lwjson, token, "key_true")) != NULL     /* Use token for relative search and search for key_true only */
                && token->type == LWJSON_TYPE_TRUE);

    /* Deep search */

    /* Search for first match in any array */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#.#.my_key")) != NULL
                && token->type == LWJSON_TYPE_STRING
                && strncmp(token->u.str.token_value, "my_text", 7) == 0);

    /* Search for match in specific array keys */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#2.#0.my_key")) != NULL
                && token->type == LWJSON_TYPE_STRING
                && strncmp(token->u.str.token_value, "my_text", 7) == 0);

    /* Search for match in specific array keys = must return NULL, no index = 1 in second array */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#2.#1.my_key")) == NULL);

    /* Use partial searches */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj")) != NULL
                && (token = lwjson_find_ex(&lwjson, token, "arr")) != NULL
                && (token = lwjson_find_ex(&lwjson, token, "#2")) != NULL
                && (token = lwjson_find_ex(&lwjson, token, "#0")) != NULL
                && (token = lwjson_find_ex(&lwjson, token, "my_key")) != NULL
                && token->type == LWJSON_TYPE_STRING
                && strncmp(token->u.str.token_value, "my_text", 7) == 0);

    /* Search for match in specific array keys and check for string length field */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.ustr")) != NULL
        && token->type == LWJSON_TYPE_STRING
        && lwjson_get_val_string_length(token) == 11
        && strncmp(token->u.str.token_value, "\\t\\u1234abc", 11) == 0);

    /* Check string compare */
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#.#.my_key")) != NULL
        && lwjson_string_compare(token, "my_text"));
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#.#.my_key")) != NULL
        && lwjson_string_compare_n(token, "my_text", 3));
    RUN_TEST((token = lwjson_find_ex(&lwjson, NULL, "my_obj.arr.#.#.my_key")) != NULL
        && !lwjson_string_compare_n(token, "my_stext", 4)); /* Must be a fail */

#undef RUN_TEST

    /* Call this once JSON usage is finished */
    lwjson_free(&lwjson);

    /* Print results */
    printf("Find function test result pass/fail: %d/%d\r\n\r\n",
            (int)test_passed, (int)test_failed);
}

/**
 * \brief           Run all tests entry point
 */
void
test_run(void) {
    /* Init LwJSON */
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
	
	/* Test JSON parse */
	test_json_parse();

    /* Test find function */
    test_find_function();

    /* Parse input text and compare against expected data types */
    test_json_data_types();
}
