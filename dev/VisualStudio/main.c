#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "windows.h"
#include "lwjson/lwjson.h"

#define print_indent(indent_level)          printf("%.*s", (int)((indent_level) * 4), "                                                  ");

void
dump(lwjson_token_t* token) {
    static size_t indent = 0;

    if (token->token_name != NULL) {
        print_indent(indent); printf("\"%.*s\":", (int)token->token_name_len, token->token_name);
    } else {
        print_indent(indent);
    }

    if (token->type == LWJSON_TYPE_OBJECT) {
        printf("{\n");
        ++indent;
        for (lwjson_token_t* t = token->u.first_child; t != NULL; t = t->next) {
            dump(t);
        }
        --indent;
        print_indent(indent); printf("}");
    } else if (token->type == LWJSON_TYPE_ARRAY) {
        printf("[\n");
        ++indent;
        for (lwjson_token_t* t = token->u.first_child; t != NULL; t = t->next) {
            dump(t);
        }
        --indent;
        print_indent(indent); printf("]");
    } else if (token->type == LWJSON_TYPE_STRING) {
        printf("\"%.*s\"", (int)token->u.str.token_value_len, token->u.str.token_value);
    } else if (token->type == LWJSON_TYPE_NUM_INT) {
        printf("%lld", (long long)token->u.num_int);
    } else if (token->type == LWJSON_TYPE_NUM_REAL) {
        printf("%f", (float)token->u.num_real);
    } else if (token->type == LWJSON_TYPE_TRUE) {
        printf("true");
    } else if (token->type == LWJSON_TYPE_FALSE) {
        printf("false");
    } else if (token->type == LWJSON_TYPE_NULL) {
        printf("NULL");
    }
    if (token->next != NULL) {
        printf(",");
    }
    printf("\n");
}

lwjson_token_t tokens[4096];
lwjson_t lwjson;

int
main() {
    HANDLE f;
    DWORD file_size;
    size_t token_cnt = 0;
    char* json_text = NULL;
    const lwjson_token_t* tkn;

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
    dump(&lwjson.first_token);

    /* Find token if exists */
    if ((tkn = lwjson_find(&lwjson, "multi_array.#.#.key6")) != NULL) {
        printf("Found requested token path\r\n");
        dump(tkn);
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
