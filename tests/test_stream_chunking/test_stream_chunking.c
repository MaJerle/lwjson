#include <stdio.h>
#include <string.h>
#include "lwjson/lwjson.h"
#include "test.h"

/*
 * 12-character value. With LWJSON_CFG_STREAM_STRING_MAX_LEN == 8 (see lwjson_opts.h),
 * the internal string buffer can hold 8 characters per chunk before it must be flushed
 * to the user callback. This value therefore arrives in exactly 2 chunks:
 *   chunk 1: "ABCDEFGH" (8 bytes, is_last = 0)
 *   chunk 2: "IJKL"     (4 bytes, is_last = 1)
 * The final chunk is shorter than the first one - this is the exact pattern that used to
 * expose stale bytes left over from the first (longer) chunk when the buffer wasn't
 * re-terminated before being handed to the user.
 */
#define CHUNK_TEST_VALUE "ABCDEFGHIJKL"

static const char* json_input = "{\"key\":\"" CHUNK_TEST_VALUE "\"}";

typedef struct {
    char reassembled[64];
    size_t string_events;
    size_t termination_failures;
} parsed_data_t;

static parsed_data_t parsed_data;

static void
prv_parser_callback(struct lwjson_stream_parser* jsp, lwjson_stream_type_t type) {
    if (type == LWJSON_STREAM_TYPE_STRING) {
        ++parsed_data.string_events;

        /*
         * Regression check for the NUL-termination fix: the buffer must be terminated
         * exactly at buff_pos. If a shorter chunk exposed stale bytes from an earlier,
         * longer chunk, strlen() would read past buff_pos into that leftover data.
         * This is the only check in this test that can catch that regression - the
         * reassembly below is length-bounded on purpose and would hide the bug.
         */
        if (strlen(jsp->data.str.buff) != jsp->data.str.buff_pos) {
            printf("Chunk not properly terminated: buff_pos=%u, strlen=%u, buff=\"%s\"\r\n",
                   (unsigned)jsp->data.str.buff_pos, (unsigned)strlen(jsp->data.str.buff), jsp->data.str.buff);
            ++parsed_data.termination_failures;
        }

        /* Reassemble using the length-bounded pattern (not relying on NUL-termination alone) */
        strncat(parsed_data.reassembled, jsp->data.str.buff, jsp->data.str.buff_pos);
    }
}

int
test_run(void) {
    printf("---\r\nTest JSON stream chunking\r\n");

    memset(&parsed_data, 0x00, sizeof(parsed_data));

    lwjson_stream_parser_t parser;
    lwjson_stream_init(&parser, prv_parser_callback);

    lwjsonr_t res = lwjsonSTREAMINPROG;
    for (const char* p = json_input; *p != '\0' && res == lwjsonSTREAMINPROG; ++p) {
        res = lwjson_stream_parse(&parser, *p);
    }

    TEST_ASSERT(res == lwjsonSTREAMDONE);

    /* Value must have needed more than one chunk to reach this test at all */
    TEST_ASSERT(parsed_data.string_events == 2);

    /* No chunk exposed stale bytes past its own length */
    TEST_ASSERT(parsed_data.termination_failures == 0);

    /* Every chunk read back correctly, with no stale trailing bytes */
    TEST_ASSERT(strcmp(parsed_data.reassembled, CHUNK_TEST_VALUE) == 0);

    printf("Test JSON stream chunking passed\r\n");
    return 0;
}
