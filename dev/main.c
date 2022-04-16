#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "windows.h"
#include "lwjson/lwjson.h"

/* Classic parser */
static lwjson_token_t tokens[4096];
static lwjson_t lwjson;

/* Stream parser */
static lwjson_stream_parser_t stream_parser;

extern void test_run(void);
extern void example_minimal_run(void);
extern void example_traverse_run(void);

static void jsp_stream_callback(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type);

int
main() {
    HANDLE f;
    DWORD file_size;
    size_t token_cnt = 0;
    char* json_text = NULL;
    const lwjson_token_t* tkn;

    //test_run();
    //example_minimal_run();
    //example_traverse_run();
    //return 0;

    printf("\n---\n");
    /* Init JSON */
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));

    f = CreateFile(TEXT("test\\json\\custom_stream.json"),
        GENERIC_READ,             // open for reading
        0,                        // do not share
        NULL,                     // no security
        OPEN_EXISTING,            // existing file only
        FILE_ATTRIBUTE_NORMAL,    // normal file
        NULL);                    // no attr. template

    if (f == INVALID_HANDLE_VALUE) {
        printf("Could not open file..\r\n");
        goto exit;
    }
    if ((file_size = GetFileSize(f, NULL)) == INVALID_FILE_SIZE) {
        printf("Invalid file size..\r\n");
        goto exit;
    } else if (file_size == 0) {
        printf("File is empty..\r\n");
        goto exit;
    }
    if ((json_text = calloc((size_t)(file_size + 1), sizeof(*json_text))) == NULL) {
        printf("Could not allocate memory..\r\n");
        goto exit;
    }
    if (ReadFile(f, json_text, file_size, NULL, NULL) == 0) {
        printf("Could not read full file..\r\n");
        goto exit;
    }

    /* Now parse as a stream */
    lwjson_stream_init(&stream_parser, jsp_stream_callback);
    for (const char* str = json_text; str != NULL && *str != '\0'; ++str) {
        lwjson_stream_parse(&stream_parser, *str);
    }
    return 0;

    /* Start parsing */
    printf("Parsing JSON with full text\r\n");
    if (lwjson_parse(&lwjson, json_text) != lwjsonOK) {
        printf("Could not parse input JSON\r\n");
        goto exit;
    }
    printf("Full JSON parsed\r\n");

    /* Dump result */
    lwjson_print_json(&lwjson);

    /* Find token if exists */
    if ((tkn = lwjson_find(&lwjson, "obj.obj2.key1")) != NULL) {
        printf("Found requested token path\r\n");
        lwjson_print_token(tkn);
    } else {
        printf("Could not find requested token path..\r\n");
    }
exit:
    if (json_text != NULL) {
        free(json_text);
        json_text = NULL;
    }
    return 0;
}

static void
jsp_stream_callback(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type) {

}
