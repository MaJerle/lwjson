#include <stdio.h>
#include "lwjson/lwjson.h"

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
    if (lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens)) != lwjsonOK) {
        printf("JSON init failed\r\n");
        return -1;
    }

    /* First parse JSON */
    if (lwjson_parse(&lwjson, json_complete) != lwjsonOK) {
        printf("Could not parse LwJSON data types..\r\n");
        return -1;
    }

    /* Now that it is parsed, check all input keys */
    for (size_t idx = 0; idx < LWJSON_ARRAYSIZE(paths_types); ++idx) {
        t = lwjson_find(&lwjson, paths_types[idx].path);
        if (t == NULL) {
            printf("IDX = %u: Could not find entry for path \"%s\"\r\n", (unsigned)idx, paths_types[idx].path);
            ++test_failed;
            continue;
        }
        if (t->type == paths_types[idx].type) {
            ++test_passed;
        } else {
            printf("IDX = %u: Type mismatch for path \"%s\"\r\n", (unsigned)idx, paths_types[idx].path);
            ++test_failed;
        }
    }

    /* Call this once JSON usage is finished */
    lwjson_free(&lwjson);

    /* Print results */
    printf("Data type test result pass/fail: %d/%d\r\n\r\n", (int)test_passed, (int)test_failed);
    return test_failed > 0 ? -1 : 0;
}
