#include <stdio.h>
#include "lwjson/lwjson.h"
#include "windows.h"

/* LwJSON stream parser */
static lwjson_stream_parser_t stream_parser;

/**
 * \brief           Callback function for various events
 * \param           jsp: JSON stream parser object
 * \param           type: Event type
 */
static void
prv_example_callback_func(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type) {
    /* IO device part of the parsing goes here */
    if (jsp->stack_pos == 4 && lwjson_stack_seq_4(jsp, 0, OBJECT, KEY, OBJECT, KEY)) {
    } else if (jsp->stack_pos == 7 && lwjson_stack_seq_7(jsp, 0, OBJECT, KEY, OBJECT, KEY, ARRAY, OBJECT, KEY)) {
    }

    (void)type;
    /* ... */
}

/* Parse JSON */
void
trial_stream_run(void) {
    lwjsonr_t res;
    HANDLE f;
    DWORD file_size;
    char* json_text = NULL;

    printf("\r\n\r\nTrial parsing test stream\r\n");
    lwjson_stream_init(&stream_parser, prv_example_callback_func);

    f = CreateFile(TEXT("trial_env/trial_run.json"),
                   GENERIC_READ,          // open for reading
                   0,                     // do not share
                   NULL,                  // no security
                   OPEN_EXISTING,         // existing file only
                   FILE_ATTRIBUTE_NORMAL, // normal file
                   NULL);                 // no attr. template

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

    /* Demonstrate as stream inputs */
    for (const char* c = json_text; *c != '\0'; ++c) {
        res = lwjson_stream_parse(&stream_parser, *c);
        if (res == lwjsonSTREAMINPROG) {
        } else if (res == lwjsonSTREAMWAITFIRSTCHAR) {
            printf("Waiting first character\r\n");
        } else if (res == lwjsonSTREAMDONE) {
            printf("Done\r\n");
        } else {
            printf("Error\r\n");
            break;
        }
    }
exit:
    free(json_text);
    printf("Parsing completed\r\n");
}
