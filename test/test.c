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

static void
test_token_count(size_t exp_token_count, const char* json_str) {
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

/* Test if JSON is properly parsed.. */
static void
test_parse(lwjsonr_t exp_result, const char* json_str) {
    if (lwjson_parse(&lwjson, json_str) == exp_result) {
        printf("Parse test passed..\r\n");
    } else {
        printf("Parse test passed failed..\r\n");
    }
}

static void
test_json_data_types(void) {
    const lwjson_token_t* t;

    printf("...\r\nParsing one JSON for data types..\r\n");
    if (lwjson_parse(&lwjson, json_complete) != lwjsonOK) {
        printf("Could not parse LwJSON data types..\r\n");
        return;
    }
    printf("JSON parsed..\r\n");

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

void
test_run(void) {
    /* Init LwJSON */
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));

    /* Run JSON parse tests that must succeed */
    test_parse(lwjsonOK, "{}");
    test_parse(lwjsonOK, "{\"k\":[]}");
    test_parse(lwjsonOK, "{\"k\":[1]}");
    test_parse(lwjsonOK, "{\"k\":[1,2]}");
    test_parse(lwjsonOK, "{\"k\":[1,]}");
    test_parse(lwjsonOK, "{\"k\":[1,[1,2]]}");
    test_parse(lwjsonOK, "{\"k\":false}");
    test_parse(lwjsonOK, "{\"k\":true}");
    test_parse(lwjsonOK, "{\"k\":null}");
    test_parse(lwjsonOK, "{\"k\":\"Stringgg\"}");
    test_parse(lwjsonOK, "{\"k\":\"Stri\\\"nggg with quote inside\"}");
    test_parse(lwjsonOK, "{\"k\":{\"b\":1E5,\t\r\n\"c\":1.3E5\r\n}\r\n}");

    /* Run JSON tests to fail */
    test_parse(lwjsonERRJSON, "");
    test_parse(lwjsonERRJSON, "{[]}");          /* Array without key inside object */
    test_parse(lwjsonERRJSON, "{\"k\":False}"); /* False value must be all lowercase */
    test_parse(lwjsonERRJSON, "{\"k\":True}");  /* True value must be all lowercase */
    test_parse(lwjsonERRJSON, "{\"k\":nUll}");  /* Null value must be all lowercase */
    test_parse(lwjsonERRJSON, "{\"k\"1}");      /* Missing separator */
    test_parse(lwjsonERRJSON, "{\"k\"1}");      /* Missing separator */
    test_parse(lwjsonERRJSON, "{k:1}");         /* Property name must be string */
    test_parse(lwjsonERRJSON, "{k:0.}");        /* Wrong number format */

    /* Run token count tests */
    test_token_count(2, "{\"k\":1}");
    test_token_count(3, "{\"k\":1,\"k\":2}");
    test_token_count(4, "{\"k\":1,\"k\":[1]}");
    test_token_count(5, "{\"k\":1,\"k\":[1,2]}");
    test_token_count(6, "{\"k\":1,\"k\":[1,2,5]}");
    test_token_count(12, "{\"k\":1,\"k\":[[1,2],[3,4],[5,6]]}");
    test_token_count(4, "{\"k\":{\"k\":{\"k\":[]}}}");
    test_token_count(6, "{\"k\":{\"k\":{\"k\":[[[]]]}}}");
    test_token_count(6, "{\"k\":[{\"k\":1},{\"k\":2}]}");

    /* Parse input text and compare against expected data types */
    test_json_data_types();
}