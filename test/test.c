#include <stdio.h>
#include "lwjson/lwjson.h"

/**
 * \brief           Path and type parse check
 */
typedef struct {
    const char* path;                           /*!< Path in parsed JSON */
    lwjson_type_t type;                         /*!< expected data type in JSON */
} test_path_type_t;

/**
 * \brief       JSON text with many different set of values
 */
const char*
json_complete = ""
"{"
"   \"int\": {"
"       \"num1\": 1234,"
"       \"num2\": -1234,"
"       \"num3\": 0"
"   },"
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

/**
 * \brief           List of paths and expected data types
 */
static test_path_type_t
paths_types[] = {
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

/* LwJSON instance and tokens */
static lwjson_token_t tokens[4096];
static lwjson_t lwjson;

/* Test number of tokens in parsed JSON string */
static void
run_token_count(size_t exp_token_count, const char* json_str) {
    if (lwjson_parse(&lwjson, json_str) != lwjsonOK) {
        printf("Could not print input JSON text: \"%s\"\r\n", json_str);
        return;
    }
    if (lwjson.next_free_token_pos + 1 == exp_token_count) {
        printf("Token count test pass..\r\n");
    } else {
        printf("Token count test failed..expected: %d, actual: %d\r\n",
                (int)exp_token_count, (int)(lwjson.next_free_token_pos + 1));
    }
}

/* Test if JSON input is properly parsed */
static void
run_parse(lwjsonr_t exp_result, const char* json_str) {
    if (lwjson_parse(&lwjson, json_str) == exp_result) {
        printf("Parse test passed..\r\n");
    } else {
        printf("Parse test passed failed.. %s\r\n", json_str);
    }
}

/**********************************************************/
/*                                                        */
/*                                                        */
/*               Actual API parsing functions             */
/*                                                        */
/*                                                        */
/**********************************************************/

/* Test JSON parsing */
static void
test_json_parse(void) {
    /* Run JSON parse tests that must succeed */
    run_parse(lwjsonOK, "{}");
    run_parse(lwjsonOK, "{ }");
    run_parse(lwjsonOK, "{}\r\n");
    run_parse(lwjsonOK, "{ }\r\n");
    run_parse(lwjsonOK, "{\t}\r\n");
    run_parse(lwjsonOK, "{\t }\r\n");
    run_parse(lwjsonOK, "[1,2,3,4]");
    run_parse(lwjsonOK, "{\"k\":[]}");
    run_parse(lwjsonOK, "{\"k\":[1]}");
    run_parse(lwjsonOK, "{\"k\":[1,2]}");
    run_parse(lwjsonOK, "{\"k\":[1,]}");
    run_parse(lwjsonOK, "{\"k\":[1,[1,2]]}");
    run_parse(lwjsonOK, "{\"k\":false}");
    run_parse(lwjsonOK, "{\"k\":true}");
    run_parse(lwjsonOK, "{\"k\":null}");
    run_parse(lwjsonOK, "{\"k\" :null}");
    run_parse(lwjsonOK, "{\"k\" : null}");
    run_parse(lwjsonOK, "{ \"k\": null }");
    run_parse(lwjsonOK, "{ \"k\": null }");
    run_parse(lwjsonOK, "{\"k\":\"Stringgg\"}");
    run_parse(lwjsonOK, "{\"k\":\"Stri\\\"nggg with quote inside\"}");
    run_parse(lwjsonOK, "{\"k\":{\"b\":1E5,\t\r\n\"c\":1.3E5\r\n}\r\n}");

    /* Arrays */
    run_parse(lwjsonOK, "[]");
    run_parse(lwjsonOK, "[ ]");
    run_parse(lwjsonOK, "[[],[]]");
    run_parse(lwjsonOK, "[[],[],{}]");
    run_parse(lwjsonERRJSON, "[");
    run_parse(lwjsonERRJSON, "[\"abc\":\"test\"]");
    run_parse(lwjsonERRJSON, "]");
    run_parse(lwjsonERRJSON, "[[,[]]");
    run_parse(lwjsonERRJSON, "[,[]]");
    run_parse(lwjsonERRJSON, "[[],[,{}]");

    /* Check specials */
    run_parse(lwjsonOK, "{\"k\":\"\\t\"}");
    run_parse(lwjsonOK, "{\"k\":\"\\b\"}");
    run_parse(lwjsonOK, "{\"k\":\"\\r\"}");
    run_parse(lwjsonOK, "{\"k\":\"\\n\"}");
    run_parse(lwjsonOK, "{\"k\":\"\\f\"}");
    run_parse(lwjsonOK, "{\"k\":\"\\\\\"}");
    run_parse(lwjsonOK, "{\"k\":\"\\u1234\"}");
    run_parse(lwjsonOK, "{\"k\":\"\\uabcd\"}");
    run_parse(lwjsonOK, "{\"k\":\"\\uAbCd\"}");
    run_parse(lwjsonOK, "{\"k\":\"\\u1abc\"}");
    run_parse(lwjsonERRJSON, "{\"k\":\"\\u1aGc\"}");
    run_parse(lwjsonERRJSON, "{\"k\":\"\\u\t\n\n\n\"}");
    run_parse(lwjsonERRJSON, "{\"k\":\"\\u\"}");
    run_parse(lwjsonERRJSON, "{\"k\":\"\\u1\"}");
    run_parse(lwjsonERRJSON, "{\"k\":\"\\u12\"}");
    run_parse(lwjsonERRJSON, "{\"k\":\"\\u123\"}");
    run_parse(lwjsonERRJSON, "{\"k\":\"\\a\"}");

    /* Run JSON tests to fail */
    run_parse(lwjsonERRJSON, "");
    run_parse(lwjsonERRJSON, "{[]}");           /* Array without key inside object */
    run_parse(lwjsonERRJSON, "{\"k\":False}");  /* False value must be all lowercase */
    run_parse(lwjsonERRJSON, "{\"k\":True}");   /* True value must be all lowercase */
    run_parse(lwjsonERRJSON, "{\"k\":nUll}");   /* Null value must be all lowercase */
    run_parse(lwjsonERRJSON, "{\"k\"1}");       /* Missing separator */
    run_parse(lwjsonERRJSON, "{\"k\"1}");       /* Missing separator */
    run_parse(lwjsonERRJSON, "{k:1}");          /* Property name must be string */
    run_parse(lwjsonERRJSON, "{k:0.}");         /* Wrong number format */
}

/* Test number of tokens necessary for parsing */
static void
test_parse_token_count(void) {
    /* Run token count tests */
    run_token_count(2, "{\"k\":1}");
    run_token_count(3, "{\"k\":1,\"k\":2}");
    run_token_count(4, "{\"k\":1,\"k\":[1]}");
    run_token_count(5, "{\"k\":1,\"k\":[1,2]}");
    run_token_count(6, "{\"k\":1,\"k\":[1,2,5]}");
    run_token_count(12, "{\"k\":1,\"k\":[[1,2],[3,4],[5,6]]}");
    run_token_count(4, "{\"k\":{\"k\":{\"k\":[]}}}");
    run_token_count(6, "{\"k\":{\"k\":{\"k\":[[[]]]}}}");
    run_token_count(6, "{\"k\":[{\"k\":1},{\"k\":2}]}");
}

/* Test JSON data types */
static void
test_json_data_types(void) {
    const lwjson_token_t* t;

    printf("Testing JSON data types..\r\n");
    if (lwjson_parse(&lwjson, json_complete) != lwjsonOK) {
        printf("Could not parse LwJSON data types..\r\n");
        return;
    }

    /* Now that it is parsed, check all input keys */
    for (size_t i = 0; i < LWJSON_ARRAYSIZE(paths_types); ++i) {
        t = lwjson_find(&lwjson, paths_types[i].path);
        if (t == NULL) {
            printf("Could not find entry for path \"%s\"\r\n", paths_types[i].path);
            continue;
        }
        if (t->type == paths_types[i].type) {
            printf("Type match for path \"%s\"\r\n", paths_types[i].path);
        } else {
            printf("Type missmatch for path \"%s\"\r\n", paths_types[i].path);
        }
    }
}

/* Test find function */
static void
test_find_function(void) {
    size_t test_failed = 0, test_passed = 0;
    const lwjson_token_t* token;
    const char* json_str = ""
    "{"
        "\"my_arr\":["
            "{\"num\":1,\"str\":\"first_entry\"},"
            "{\"num\":2,\"str\":\"second_entry\"},"
            "{\"num\":3,\"str\":\"third_entry\"},"
            "[\"abc\", \"def\"]"
        "]"
    "}";

 #define RUN_TEST(c)     if ((c)) { ++test_passed; } else { ++test_failed; printf("Test failed on line %d\r\n", __LINE__); }

    lwjson_parse(&lwjson, json_str);

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

    /* Print results */
    printf("Find function test result\r\nTests passed/failed: %d/%d\r\n",
            (int)test_passed, (int)test_failed);

#undef RUN_TEST
}

void
test(void) {
    /* Init LwJSON */
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
	
	/* Test JSON parse */
	test_json_parse();

    /* Test find function */
    test_find_function();

    /* Parse input text and compare against expected data types */
    test_json_data_types();
}
