#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "windows.h"
#include "lwjson/lwjson.h"

static lwjson_token_t tokens[4096];
static lwjson_t lwjson;

extern void test_run(void);
extern void example_minimal_run(void);
extern void example_traverse_run(void);

int
main() {
    HANDLE f;
    DWORD file_size;
    size_t token_cnt = 0;
    char* json_text = NULL;
    const lwjson_token_t* tkn;

    test_run();
    //example_minimal_run();
    //example_traverse_run();
    return 0;

    printf("\n---\n");
    /* Init JSON */
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));

    f = CreateFile(TEXT("..\\..\\test\\json\\custom.json"),
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
    file_size = GetFileSize(f, NULL);
    if (file_size == INVALID_FILE_SIZE) {
        printf("Invalid file size..\r\n");
        goto exit;
    } else if (file_size == 0) {
        printf("File is empty..\r\n");
        goto exit;
    }
    json_text = calloc((size_t)(file_size + 1), sizeof(*json_text));
    if (json_text == NULL) {
        printf("Could not allocate memory..\r\n");
        goto exit;
    }
    if (ReadFile(f, json_text, file_size, NULL, NULL) == 0) {
        printf("Could not read full file..\r\n");
        goto exit;
    }

    /* Start parsing */
    if (lwjson_parse(&lwjson, json_text) != lwjsonOK) {
        printf("Could not parse input json\r\n");
        goto exit;
    }

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
