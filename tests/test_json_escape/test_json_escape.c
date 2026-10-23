#include <stdio.h>
#include <string.h>
#include "lwjson/lwjson.h"
#include "lwjson/lwjson_utils.h"
#include "test.h"

/*
 * Escape sequence test for both, non-stream and stream API.
 *
 * Parsed string values (and keys) are expected to be unescaped,
 * the opposite of what the serializer does when it escapes them on output.
 * See https://github.com/MaJerle/lwjson/discussions/40
 *
 * - Non-stream API keeps token values as raw pointers to JSON input,
 *   values are unescaped with lwjson_utils_unescape_string, keys are matched unescaped by lwjson_find
 * - Stream API delivers unescaped values and keys directly
 *
 * Key names are kept shorter than LWJSON_CFG_STREAM_STRING_MAX_LEN,
 * as stream parser stores only the last chunk of the key name to the stack.
 */

#define RUN_TEST(exp_res_must_be_true)                                                                                 \
    if ((exp_res_must_be_true)) {                                                                                      \
        ++test_passed;                                                                                                 \
    } else {                                                                                                           \
        ++test_failed;                                                                                                 \
        printf("Test failed on line %d\r\n", __LINE__);                                                                \
    }

/**
 * \brief           Expected value for a JSON path
 */
typedef struct {
    const char* path;     /*!< Path in parsed JSON, with unescaped key names */
    const char* expected; /*!< Expected unescaped string value */
} test_escape_t;

/* Input JSON string, every escape sequence is written as JSON source text */
static const char* json_input = ""
                                "{"
                                "\"plain\":\"Hello world, nothing special here\","
                                "\"newline\":\"line1\\nline2\","
                                "\"crlf\":\"first\\r\\nsecond\\r\\n\","
                                "\"tab\":\"col1\\tcol2\\tcol3\","
                                "\"quote\":\"She said \\\"hi\\\" and left\","
                                "\"bslash\":\"C:\\\\path\\\\to\\\\file.txt\","
                                "\"slash\":\"http:\\/\\/example.com\\/a\","
                                "\"ctrl\":\"x\\by\\fz\","
                                "\"unicode\":\"snow \\u2603, e-acute \\u00E9\","
                                "\"emoji\":\"smile \\uD83D\\uDE00!\","
                                "\"mixed\":\"Lorem\\tipsum\\r\\ndolor \\\"sit\\\" amet\\\\\\n\","
                                "\"onlyesc\":\"\\n\\r\\t\\\"\\\\\","
                                "\"trailbs\":\"ends with backslash\\\\\","
                                "\"bound\":\"1234567\\n1234567\\\\\\\"x\","
                                "\"arr\":[\"a\\nb\",\"\\\"q\\\"\",\"\\u0041\"],"
                                "\"k\\tey\":\"escaped key\""
                                "}";

/* Expected unescaped values */
static const test_escape_t escape_tests[] = {
    {"plain", "Hello world, nothing special here"},
    {"newline", "line1\nline2"},
    {"crlf", "first\r\nsecond\r\n"},
    {"tab", "col1\tcol2\tcol3"},
    {"quote", "She said \"hi\" and left"},
    {"bslash", "C:\\path\\to\\file.txt"},
    {"slash", "http://example.com/a"},
    {"ctrl", "x\by\fz"},
    {"unicode", "snow \xE2\x98\x83, e-acute \xC3\xA9"}, /* U+2603 and U+00E9 in UTF-8 */
    {"emoji", "smile \xF0\x9F\x98\x80!"},                /* Surrogate pair U+1F600 in UTF-8 */
    {"mixed", "Lorem\tipsum\r\ndolor \"sit\" amet\\\n"},
    {"onlyesc", "\n\r\t\"\\"},
    {"trailbs", "ends with backslash\\"},
    {"bound", "1234567\n1234567\\\"x"},
    {"arr.#0", "a\nb"},
    {"arr.#1", "\"q\""},
    {"arr.#2", "A"},
    {"k\tey", "escaped key"},
};

/* Invalid escape sequences, must be rejected by both parsers */
static const char* invalid_escape_tests[] = {
    "{\"a\":\"bad\\q\"}",  /* Unknown escape character */
    "{\"a\":\"\\u12G4\"}", /* Non-hex digit in unicode escape */
    "{\"a\":\"\\u12\"}",   /* Too short unicode escape */
    "{\"a\":\"\\x41\"}",   /* Hex escape is not valid JSON */
    "{\"a\":\"\\uDE00\"}", /* Low surrogate without high one */
    "{\"a\":\"\\uD83Dx\"}", /* High surrogate without low one */
    "{\"a\":\"\\uD83D\\u0041\"}", /* High surrogate followed by non-surrogate */
};

/* LwJSON instance and tokens */
static lwjson_token_t tokens[128];
static lwjson_t lwjson;
static lwjson_stream_parser_t parser;

/* Reassembled values from the stream parser, one per escape_tests entry */
static struct {
    char buff[64];
    size_t len;
    uint8_t found;
} stream_values[LWJSON_ARRAYSIZE(escape_tests)];

/**
 * \brief           Print string with non-printable characters as hex, for failure reports
 */
static void
prv_print_escaped(const char* str, size_t len) {
    printf("\"");
    for (size_t idx = 0; idx < len; ++idx) {
        unsigned char chr = (unsigned char)str[idx];
        if (chr >= 0x20 && chr < 0x7F && chr != '\\') {
            printf("%c", (char)chr);
        } else {
            printf("\\x%02X", (unsigned)chr);
        }
    }
    printf("\"");
}

/**
 * \brief           Compare parsed value against expected one, print both on mismatch
 * \return          `1` on match, `0` otherwise
 */
static uint8_t
prv_check_value(const char* api, const char* path, const char* value, size_t value_len, const char* expected) {
    size_t expected_len = strlen(expected);
    if (value_len == expected_len && memcmp(value, expected, expected_len) == 0) {
        return 1;
    }
    printf("%s: mismatch for path ", api);
    prv_print_escaped(path, strlen(path));
    printf("\r\n  expected: ");
    prv_print_escaped(expected, expected_len);
    printf("\r\n  received: ");
    prv_print_escaped(value, value_len);
    printf("\r\n");
    return 0;
}

/**
 * \brief           Stream parser callback, reassembles string chunks per path
 */
static void
prv_stream_callback(struct lwjson_stream_parser* jsp, lwjson_stream_type_t type) {
    char path[LWJSON_CFG_STREAM_KEY_MAX_LEN + 16];

    if (type != LWJSON_STREAM_TYPE_STRING) {
        return;
    }

    /* Build the path in the same format as used by the non-stream find function */
    if (jsp->stack_pos == 2 && lwjson_stack_seq_2(jsp, 0, OBJECT, KEY)) {
        snprintf(path, sizeof(path), "%s", jsp->stack[1].meta.name);
    } else if (jsp->stack_pos == 3 && lwjson_stack_seq_3(jsp, 0, OBJECT, KEY, ARRAY)) {
        snprintf(path, sizeof(path), "%s.#%u", jsp->stack[1].meta.name, (unsigned)jsp->stack[2].meta.index);
    } else {
        return;
    }

    for (size_t idx = 0; idx < LWJSON_ARRAYSIZE(escape_tests); ++idx) {
        if (strcmp(escape_tests[idx].path, path) == 0) {
            if (stream_values[idx].len + jsp->data.str.buff_pos < sizeof(stream_values[idx].buff)) {
                memcpy(&stream_values[idx].buff[stream_values[idx].len], jsp->data.str.buff, jsp->data.str.buff_pos);
                stream_values[idx].len += jsp->data.str.buff_pos;
            }
            stream_values[idx].found = 1;
            return;
        }
    }
    printf("Stream: unexpected string for path ");
    prv_print_escaped(path, strlen(path));
    printf("\r\n");
}

/**
 * \brief           Feed the complete string to the stream parser
 * \return          Last parser result
 */
static lwjsonr_t
prv_stream_parse_all(const char* json, lwjson_stream_parser_callback_fn cb) {
    lwjsonr_t res = lwjsonSTREAMINPROG;

    lwjson_stream_init(&parser, cb);
    for (const char* ptr = json; *ptr != '\0' && res == lwjsonSTREAMINPROG; ++ptr) {
        res = lwjson_stream_parse(&parser, *ptr);
    }
    return res;
}

int
test_run(void) {
    size_t test_failed = 0, test_passed = 0;
    const lwjson_token_t* tkn;
    lwjsonr_t res;

    printf("---\r\nTest JSON escape sequences\r\n");

    /* Non-stream API */
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
    res = lwjson_parse(&lwjson, json_input);
    RUN_TEST(res == lwjsonOK);
    if (res == lwjsonOK) {
        for (size_t idx = 0; idx < LWJSON_ARRAYSIZE(escape_tests); ++idx) {
            const char* value;
            char unescaped[64];
            size_t value_len = 0, unescaped_len = 0;

            tkn = lwjson_find(&lwjson, escape_tests[idx].path);
            if (tkn == NULL || tkn->type != LWJSON_TYPE_STRING) {
                printf("Non-stream: string not found for path ");
                prv_print_escaped(escape_tests[idx].path, strlen(escape_tests[idx].path));
                printf("\r\n");
                RUN_TEST(0);
                continue;
            }
            /* Token value points to raw JSON input, unescape it to user buffer */
            value = lwjson_get_val_string(tkn, &value_len);
            res = lwjson_utils_unescape_string(value, value_len, unescaped, sizeof(unescaped), &unescaped_len);
            RUN_TEST(res == lwjsonOK);
            RUN_TEST(prv_check_value("Non-stream", escape_tests[idx].path, unescaped, unescaped_len,
                                     escape_tests[idx].expected));
        }
    }
    lwjson_free(&lwjson);

    /* Stream API */
    memset(stream_values, 0x00, sizeof(stream_values));
    RUN_TEST(prv_stream_parse_all(json_input, prv_stream_callback) == lwjsonSTREAMDONE);
    for (size_t idx = 0; idx < LWJSON_ARRAYSIZE(escape_tests); ++idx) {
        if (!stream_values[idx].found) {
            printf("Stream: string not received for path ");
            prv_print_escaped(escape_tests[idx].path, strlen(escape_tests[idx].path));
            printf("\r\n");
            RUN_TEST(0);
            continue;
        }
        RUN_TEST(prv_check_value("Stream", escape_tests[idx].path, stream_values[idx].buff, stream_values[idx].len,
                                 escape_tests[idx].expected));
    }

    /* Invalid escape sequences must fail in both parsers */
    for (size_t idx = 0; idx < LWJSON_ARRAYSIZE(invalid_escape_tests); ++idx) {
        lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
        res = lwjson_parse(&lwjson, invalid_escape_tests[idx]);
        lwjson_free(&lwjson);
        if (res == lwjsonOK) {
            printf("Non-stream: invalid escape accepted: %s\r\n", invalid_escape_tests[idx]);
        }
        RUN_TEST(res != lwjsonOK);

        res = prv_stream_parse_all(invalid_escape_tests[idx], NULL);
        if (res == lwjsonSTREAMDONE || res == lwjsonSTREAMINPROG) {
            printf("Stream: invalid escape accepted: %s\r\n", invalid_escape_tests[idx]);
        }
        RUN_TEST(res != lwjsonSTREAMDONE && res != lwjsonSTREAMINPROG);
    }

    printf("Escape test result pass/fail: %d/%d\r\n\r\n", (int)test_passed, (int)test_failed);
    return test_failed > 0 ? -1 : 0;
}
