#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lwjson/lwjson.h"
#include "lwjson/lwjson_serializer.h"
#include "lwjson/lwjson_utils.h"
#include "test.h"

#define RUN_TEST(exp_res_must_be_true)                                                                                 \
    if ((exp_res_must_be_true)) {                                                                                      \
        ++test_passed;                                                                                                 \
    } else {                                                                                                           \
        ++test_failed;                                                                                                 \
        printf("Test failed on line %d\r\n", __LINE__);                                                                \
    }

/**
 * \brief           Test data structure to serialize
 */
typedef struct {
    int numbers_array[3];
    struct {
        int num1;
        int num2;
    } numbers_obj;
    int numbers_array_array[2][3];
    lwjson_real_t number_real;
    char long_string[128];
    uint8_t bool_value;
} test_data_t;

/* Test data to serialize */
static test_data_t test_data = {
    .numbers_array = {123, -123, 987},
    .numbers_obj = {
        .num1 = 123,
        .num2 = 456
    },
    .numbers_array_array = {
        {1, 2, 3},
        {4, 5, 6}
    },
    .number_real = 3.5f,
    .long_string = "this is a test string for serialization",
    .bool_value = 1
};

/**
 * \brief           Serialize test data to JSON string using lwjson_serializer
 * \param[out]      json_str: Output buffer for JSON string
 * \param[in]       max_len: Maximum length of output buffer
 * \return          Length of serialized JSON or -1 on error
 */
static int
serialize_test_data(char* json_str, size_t max_len) {
    lwjson_serializer_t serializer;
    lwjsonr_t res;
    size_t outputBytes = 0;

    /* Initialize serializer with output buffer */
    res = lwjson_serializer_init(&serializer, json_str, max_len);
    if (res != lwjsonOK) {
        printf("Failed to initialize serializer: %d\r\n", res);
        return -1;
    }

    /* Start root object */
    res = lwjson_serializer_start_object(&serializer, NULL, 0);
    if (res != lwjsonOK) {
        printf("Failed to start object: %d\r\n", res);
        return -1;
    }

    /* Serialize numbers_array */
    res = lwjson_serializer_start_array(&serializer, "numbers_array", 13);
    if (res != lwjsonOK) {
        printf("Failed to start numbers_array: %d\r\n", res);
        return -1;
    }

    for (int i = 0; i < 3; i++) {
        res = lwjson_serializer_add_int(&serializer, NULL, 0, test_data.numbers_array[i]);
        if (res != lwjsonOK) {
            printf("Failed to add numbers_array[%d]: %d\r\n", i, res);
            return -1;
        }
    }

    res = lwjson_serializer_end_array(&serializer);
    if (res != lwjsonOK) {
        printf("Failed to end numbers_array: %d\r\n", res);
        return -1;
    }

    /* Serialize numbers_obj */
    res = lwjson_serializer_start_object(&serializer, "numbers_obj", 11);
    if (res != lwjsonOK) {
        printf("Failed to start numbers_obj: %d\r\n", res);
        return -1;
    }

    res = lwjson_serializer_add_int(&serializer, "num1", 4, test_data.numbers_obj.num1);
    if (res != lwjsonOK) {
        printf("Failed to add num1: %d\r\n", res);
        return -1;
    }

    res = lwjson_serializer_add_int(&serializer, "num2", 4, test_data.numbers_obj.num2);
    if (res != lwjsonOK) {
        printf("Failed to add num2: %d\r\n", res);
        return -1;
    }

    res = lwjson_serializer_end_object(&serializer);
    if (res != lwjsonOK) {
        printf("Failed to end numbers_obj: %d\r\n", res);
        return -1;
    }

    /* Serialize numbers_array_array */
    res = lwjson_serializer_start_array(&serializer, "numbers_array_array", 19);
    if (res != lwjsonOK) {
        printf("Failed to start numbers_array_array: %d\r\n", res);
        return -1;
    }

    for (int i = 0; i < 2; i++) {
        res = lwjson_serializer_start_array(&serializer, NULL, 0);
        if (res != lwjsonOK) {
            printf("Failed to start inner array[%d]: %d\r\n", i, res);
            return -1;
        }

        for (int j = 0; j < 3; j++) {
            res = lwjson_serializer_add_int(&serializer, NULL, 0, test_data.numbers_array_array[i][j]);
            if (res != lwjsonOK) {
                printf("Failed to add numbers_array_array[%d][%d]: %d\r\n", i, j, res);
                return -1;
            }
        }

        res = lwjson_serializer_end_array(&serializer);
        if (res != lwjsonOK) {
            printf("Failed to end inner array[%d]: %d\r\n", i, res);
            return -1;
        }
    }

    res = lwjson_serializer_end_array(&serializer);
    if (res != lwjsonOK) {
        printf("Failed to end numbers_array_array: %d\r\n", res);
        return -1;
    }

    /* Serialize number_real */
    res = lwjson_serializer_add_float(&serializer, "number_real", 11, test_data.number_real);
    if (res != lwjsonOK) {
        printf("Failed to add number_real: %d\r\n", res);
        return -1;
    }

    /* Serialize long_string */
    res = lwjson_serializer_add_string(&serializer, "long_string", 11, test_data.long_string, strlen(test_data.long_string));
    if (res != lwjsonOK) {
        printf("Failed to add long_string: %d\r\n", res);
        return -1;
    }

    /* Serialize bool_value */
    res = lwjson_serializer_add_bool(&serializer, "bool_value", 10, test_data.bool_value);
    if (res != lwjsonOK) {
        printf("Failed to add bool_value: %d\r\n", res);
        return -1;
    }

    /* End root object */
    res = lwjson_serializer_end_object(&serializer);
    if (res != lwjsonOK) {
        printf("Failed to end object: %d\r\n", res);
        return -1;
    }

    res = lwjson_serializer_finalize(&serializer, &outputBytes);
    if (res != lwjsonOK) {
        printf("Failed to finalize serializer: %d\r\n", res);
        return -1;
    }
    return outputBytes;
}

/**
 * \brief           Test lwjson_utils string escaping functions
 * \return          0 on success, -1 on error
 */
static int
test_string_utils(void) {
    char input[] = "Hello \"World\"\t\n\r\\Test - 中文 ČĞ\xc5\xbe";
    char output[256];
    size_t bytes_written = 0;
    lwjsonr_t res;
    char unescaped[256];
    size_t unescaped_len = 0;

    printf("Testing string escape utilities...\r\n");
    printf("Input string: '%s'\r\n", input);

    /* Test string escaping */
    res = lwjson_utils_escape_string(input, strlen(input), output, sizeof(output), &bytes_written);
    if (res != lwjsonOK) {
        printf("Failed to escape string: %d\r\n", res);
        return -1;
    }

    printf("Escaped string: '%.*s' (length: %lu)\r\n", (int)bytes_written, output, (unsigned long)bytes_written);

    /* Test string unescaping */
    res = lwjson_utils_unescape_string(output, bytes_written, unescaped, sizeof(unescaped) - 1, &unescaped_len);
    if (res != lwjsonOK) {
        printf("Failed to unescape string: %d\r\n", res);
        return -1;
    }

    unescaped[unescaped_len] = '\0'; /* Null terminate for comparison */
    printf("Unescaped string: '%s' (length: %lu)\r\n", unescaped, (unsigned long)unescaped_len);

    /* Verify round-trip correctness */
    if (strlen(input) == unescaped_len && memcmp(input, unescaped, unescaped_len) == 0) {
        printf("String escape/unescape round-trip test PASSED\r\n");
        return 0;
    } else {
        printf("String escape/unescape round-trip test FAILED\r\n");
        return -1;
    }
}

int
test_run(void) {
    size_t test_failed = 0, test_passed = 0;
    char serialized_json[1024];
    int json_len;
    lwjson_token_t tokens[256];
    lwjson_t lwj;
    lwjsonr_t res;
    const lwjson_token_t* token;
    size_t str_len = 0;

    printf("---\r\nTest JSON serialization and utilities\r\n");

    /* Test string utilities first */
    printf("\r\n=== Testing lwjson_utils functions ===\r\n");
    RUN_TEST(test_string_utils() == 0);

    /* Test Unicode unescaping (emoji, euro, chinese) */
    {
        struct {
            const char* input;
            const char* expected_utf8;
            const char* description;
        } unicode_tests[] = {
            {"\u20AC", "\xe2\x82\xac", "Euro sign"},
            {"\u4E2D\u6587", "\xe4\xb8\xad\xe6\x96\x87", "Chinese: 中文"},
            {"\u010C\u011E\u0160\u017E", "\xc4\x8c\xc4\x9e\xc5\xa0\xc5\xbe", "Czech: ČĚŠŽ"}
        };
        char outbuf[16];
        size_t outlen;
        for (size_t i = 0; i < sizeof(unicode_tests)/sizeof(unicode_tests[0]); ++i) {
            memset(outbuf, 0, sizeof(outbuf));
            res = lwjson_utils_unescape_string(unicode_tests[i].input, strlen(unicode_tests[i].input), outbuf, sizeof(outbuf), &outlen);
            int match = (res == lwjsonOK && outlen == strlen(unicode_tests[i].expected_utf8) && memcmp(outbuf, unicode_tests[i].expected_utf8, outlen) == 0);
            printf("Unicode test '%s': %s\r\n", unicode_tests[i].description, match ? "PASSED" : "FAILED");
            RUN_TEST(match);
        }
    }

    printf("\r\n=== Testing lwjson_serializer functions ===\r\n");

    /* Initialize lwjson */
    if (lwjson_init(&lwj, tokens, LWJSON_ARRAYSIZE(tokens)) != lwjsonOK) {
        printf("JSON init failed\r\n");
        return -1;
    }

    /* Test serialization */
    json_len = serialize_test_data(serialized_json, sizeof(serialized_json));
    if (json_len > 0 && json_len < (int)sizeof(serialized_json)) {
        serialized_json[json_len] = '\0'; /* Ensure null termination */
        printf("Serialized JSON (%d chars): %s\r\n", json_len, serialized_json);
    } else {
        printf("ERROR: Invalid serialization length: %d\r\n", json_len);
        return -1;
    }

    RUN_TEST(json_len > 0);

    /* Test if we can parse the serialized JSON back */
    res = lwjson_parse(&lwj, serialized_json);
    RUN_TEST(res == lwjsonOK);

    if (res == lwjsonOK) {
        /* Verify array values */
        token = lwjson_find(&lwj, "numbers_array.#0");
        RUN_TEST(token != NULL && lwjson_get_val_int(token) == 123);

        token = lwjson_find(&lwj, "numbers_array.#1");
        RUN_TEST(token != NULL && lwjson_get_val_int(token) == -123);

        token = lwjson_find(&lwj, "numbers_array.#2");
        RUN_TEST(token != NULL && lwjson_get_val_int(token) == 987);

        /* Verify object values */
        token = lwjson_find(&lwj, "numbers_obj.num1");
        RUN_TEST(token != NULL && lwjson_get_val_int(token) == 123);

        token = lwjson_find(&lwj, "numbers_obj.num2");
        RUN_TEST(token != NULL && lwjson_get_val_int(token) == 456);

        /* Verify 2D array values */
        token = lwjson_find(&lwj, "numbers_array_array.#0.#0");
        RUN_TEST(token != NULL && lwjson_get_val_int(token) == 1);

        token = lwjson_find(&lwj, "numbers_array_array.#0.#1");
        RUN_TEST(token != NULL && lwjson_get_val_int(token) == 2);

        token = lwjson_find(&lwj, "numbers_array_array.#1.#2");
        RUN_TEST(token != NULL && lwjson_get_val_int(token) == 6);

        /* Verify real number */
        token = lwjson_find(&lwj, "number_real");
        RUN_TEST(token != NULL && lwjson_get_val_real(token) == 3.5);

        /* Verify string */
        token = lwjson_find(&lwj, "long_string");
        RUN_TEST(token != NULL);
        const char* str_val = lwjson_get_val_string(token, &str_len);
        RUN_TEST(str_len == strlen("this is a test string for serialization")
                 && strncmp(str_val, "this is a test string for serialization", str_len) == 0);

        /* Verify boolean */
        token = lwjson_find(&lwj, "bool_value");
        RUN_TEST(token != NULL && token->type == LWJSON_TYPE_TRUE);

        /* Free JSON */
        RUN_TEST(lwjson_free(&lwj) == lwjsonOK);
    }

    printf("Serialization tests completed: %zu passed, %zu failed\r\n", test_passed, test_failed);

    return test_failed > 0 ? -1 : 0;
}
