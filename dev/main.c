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

    /*
     * Chcek for hourly forecast
     *
     * Define the stack sequence which gives us to this point
     */
    if (jsp->stack_pos >= 5
        /* First build the order... */
        && jsp->stack[0].type == LWJSON_STREAM_TYPE_OBJECT
        && jsp->stack[1].type == LWJSON_STREAM_TYPE_KEY
        && jsp->stack[2].type == LWJSON_STREAM_TYPE_ARRAY
        && jsp->stack[3].type == LWJSON_STREAM_TYPE_OBJECT
        && jsp->stack[4].type == LWJSON_STREAM_TYPE_KEY

        /* .. then analyze the strings */
        && strcmp(jsp->stack[1].name, "hourly") == 0) {

        if (strcmp(jsp->stack[4].name, "dt") == 0) {
            printf("Hour %d forecast for dt: %u\r\n", (int)jsp->stack[jsp->stack_pos - 3].index, (unsigned)atoll(jsp->data.str.buff));
        } else if (strcmp(jsp->stack[4].name, "temp") == 0) {
            printf("Hour %d forecast for temp: %f\r\n", (int)jsp->stack[jsp->stack_pos - 3].index, strtod(jsp->data.str.buff, NULL));
        } else if (strcmp(jsp->stack[4].name, "feels_like") == 0) {
            printf("Hour %d forecast for feels_like: %f\r\n", (int)jsp->stack[jsp->stack_pos - 3].index, strtod(jsp->data.str.buff, NULL));
        } else if (strcmp(jsp->stack[4].name, "pressure") == 0) {
            printf("Hour %d forecast for pressure: %d\r\n", (int)jsp->stack[jsp->stack_pos - 3].index, (int)atoll(jsp->data.str.buff));
        } else if (strcmp(jsp->stack[4].name, "humidity") == 0) {
            printf("Hour %d forecast for humidity: %d\r\n", (int)jsp->stack[jsp->stack_pos - 3].index, (int)atoll(jsp->data.str.buff));
        }
    }

    /* Get daily temperatures */
    if (jsp->stack_pos >= 5
        /* First build the order... */
        && jsp->stack[0].type == LWJSON_STREAM_TYPE_OBJECT
        && jsp->stack[1].type == LWJSON_STREAM_TYPE_KEY
        && jsp->stack[2].type == LWJSON_STREAM_TYPE_ARRAY
        && jsp->stack[3].type == LWJSON_STREAM_TYPE_OBJECT
        && jsp->stack[4].type == LWJSON_STREAM_TYPE_KEY

        /* .. then analyze the strings */
        && strcmp(jsp->stack[1].name, "daily") == 0) {
        
        /* Analyze objects for temp and feels like object */
        if (jsp->stack_pos >= 7
            && jsp->stack[5].type == LWJSON_STREAM_TYPE_OBJECT
            && jsp->stack[6].type == LWJSON_STREAM_TYPE_KEY) {
            if (strcmp(jsp->stack[4].name, "temp") == 0) {
                /* Parsing of temp object */    
                if (strcmp(jsp->stack[6].name, "min") == 0) {
                    printf("Day %d temp min: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[6].name, "max") == 0) {
                    printf("Day %d temp max: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[6].name, "day") == 0) {
                    printf("Day %d temp day: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[6].name, "night") == 0) {
                    printf("Day %d temp night: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[6].name, "eve") == 0) {
                    printf("Day %d temp eve: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[6].name, "morn") == 0) {
                    printf("Day %d temp morn: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
                }
            } else if (strcmp(jsp->stack[4].name, "feels_like") == 0) {
                /* Parsing of feels-like objects */
                if (strcmp(jsp->stack[6].name, "day") == 0) {
                    printf("Day %d temp_feels_like day: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[6].name, "night") == 0) {
                    printf("Day %d temp_feels_like night: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[6].name, "eve") == 0) {
                    printf("Day %d temp_feels_like eve: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[6].name, "morn") == 0) {
                    printf("Day %d temp_feels_like morn: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
                }
            }
        } else if (strcmp(jsp->stack[4].name, "dt") == 0) {
            printf("Day %d dt: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
        } else if (strcmp(jsp->stack[4].name, "pressure") == 0) {
            printf("Day %d pressure: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
        } else if (strcmp(jsp->stack[4].name, "humidity") == 0) {
            printf("Day %d humidity: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
        } else if (strcmp(jsp->stack[4].name, "clouds") == 0) {
            printf("Day %d clouds: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
        } else if (strcmp(jsp->stack[4].name, "snow") == 0) {
            printf("Day %d snow: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
        } else if (strcmp(jsp->stack[4].name, "uvi") == 0) {
            printf("Day %d uvi: %s\r\n", (int)jsp->stack[2].index, jsp->data.str.buff);
        }
    }
}
