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

    f = CreateFile(TEXT("test\\json\\weather_onecall.json"),
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
        if (lwjson_stream_parse(&stream_parser, *str) != lwjsonOK) {
            break;
        }
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
    /* Get current weather icon */
    if (type == LWJSON_STREAM_TYPE_STRING               /* Make sure current type is a string */
        && jsp->stack_pos >= 6

        /* Its previously parsed key has to be icon */
        && (jsp->stack[jsp->stack_pos - 1].type == LWJSON_STREAM_TYPE_KEY && strcmp(jsp->stack[jsp->stack_pos - 1].name, "icon") == 0)
        && (jsp->stack[jsp->stack_pos - 2].type == LWJSON_STREAM_TYPE_OBJECT)
        && (jsp->stack[jsp->stack_pos - 3].type == LWJSON_STREAM_TYPE_ARRAY)
        && (jsp->stack[jsp->stack_pos - 4].type == LWJSON_STREAM_TYPE_KEY && strcmp(jsp->stack[jsp->stack_pos - 4].name, "weather") == 0)
        && (jsp->stack[jsp->stack_pos - 5].type == LWJSON_STREAM_TYPE_OBJECT)
        && (jsp->stack[jsp->stack_pos - 6].type == LWJSON_STREAM_TYPE_KEY && strcmp(jsp->stack[jsp->stack_pos - 6].name, "current") == 0)) {
        
        printf("GOT ICON string for current weather: %s\r\n", jsp->data.str.buff);
    }

    /* Get hourly temperature */
    if (type == LWJSON_STREAM_TYPE_NUMBER
        && jsp->stack_pos >= 4

        /* Get chain of previously parsed entries */
        && (jsp->stack[jsp->stack_pos - 1].type == LWJSON_STREAM_TYPE_KEY && strcmp(jsp->stack[jsp->stack_pos - 1].name, "temp") == 0)
        && (jsp->stack[jsp->stack_pos - 2].type == LWJSON_STREAM_TYPE_OBJECT)
        && (jsp->stack[jsp->stack_pos - 3].type == LWJSON_STREAM_TYPE_ARRAY)
        && (jsp->stack[jsp->stack_pos - 4].type == LWJSON_STREAM_TYPE_KEY && strcmp(jsp->stack[jsp->stack_pos - 4].name, "hourly") == 0)) {
        
        printf("Hour %d forecast for temperature: %s\r\n", (int)jsp->stack[jsp->stack_pos - 3].index, jsp->data.str.buff);
    }

    /* Get daily temperatures */
    if (type == LWJSON_STREAM_TYPE_NUMBER
        && jsp->stack_pos >= 6

        /* Get chain of previously parsed entries */
        && (jsp->stack[jsp->stack_pos - 1].type == LWJSON_STREAM_TYPE_KEY)
        && (jsp->stack[jsp->stack_pos - 2].type == LWJSON_STREAM_TYPE_OBJECT)
        && (jsp->stack[jsp->stack_pos - 3].type == LWJSON_STREAM_TYPE_KEY && strcmp(jsp->stack[jsp->stack_pos - 3].name, "temp") == 0)
        && (jsp->stack[jsp->stack_pos - 4].type == LWJSON_STREAM_TYPE_OBJECT)
        && (jsp->stack[jsp->stack_pos - 5].type == LWJSON_STREAM_TYPE_ARRAY)
        && (jsp->stack[jsp->stack_pos - 6].type == LWJSON_STREAM_TYPE_KEY && strcmp(jsp->stack[jsp->stack_pos - 6].name, "daily") == 0)) {
        
        if (strcmp(jsp->stack[jsp->stack_pos - 1].name, "min") == 0) {
            printf("Day %d temp minimum: %s\r\n", (int)jsp->stack[jsp->stack_pos - 5].index, jsp->data.str.buff);
        } else if (strcmp(jsp->stack[jsp->stack_pos - 1].name, "max") == 0) {
            printf("Day %d temp maximum: %s\r\n", (int)jsp->stack[jsp->stack_pos - 5].index, jsp->data.str.buff);
        }
    }
}
