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
extern void example_stream_run(void);

static void jsp_stream_callback(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type);

int
main() {
    HANDLE f;
    DWORD file_size;
    size_t token_cnt = 0;
    char* json_text = NULL;
    const lwjson_token_t* tkn;

    test_run();
    example_minimal_run();
    example_traverse_run();
    example_stream_run();
    return 0;

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
#if 0
    if (jsp->stack_pos >= 4
        && (type == LWJSON_STREAM_TYPE_STRING || type == LWJSON_STREAM_TYPE_NUMBER)
        && jsp->stack[0].type == LWJSON_STREAM_TYPE_OBJECT
        && jsp->stack[1].type == LWJSON_STREAM_TYPE_KEY
        && jsp->stack[2].type == LWJSON_STREAM_TYPE_ARRAY
        && jsp->stack[3].type == LWJSON_STREAM_TYPE_ARRAY
        && strcmp(jsp->stack[1].meta.name, "array_in_array") == 0) {
        printf("Index: %u, Index: %u, value: %s\r\n",
            (int)jsp->stack[2].meta.index, (int)jsp->stack[3].meta.index,
            jsp->data.str.buff
        );
    }
    return;
#endif

    /* 
     * Take care of key-value pairs immediately at the start of object
     *
     * Values such as latitude and longitude are parsed here
     */
    if (jsp->stack_pos == 2
        && jsp->stack[0].type == LWJSON_STREAM_TYPE_OBJECT
        && jsp->stack[1].type == LWJSON_STREAM_TYPE_KEY) {

        if (strcmp(jsp->stack[1].meta.name, "lat") == 0) {
            printf("Latitude weather: %f\r\n", strtod(jsp->data.str.buff, NULL));
        } else if (strcmp(jsp->stack[1].meta.name, "lon") == 0) {
            printf("Longitude weather: %f\r\n", strtod(jsp->data.str.buff, NULL));
        } else if (strcmp(jsp->stack[1].meta.name, "timezone_offset") == 0) {
            printf("Timezone offset: %u\r\n", (unsigned)atoll(jsp->data.str.buff));
        } else if (strcmp(jsp->stack[1].meta.name, "timezone") == 0) {
            printf("Timezone: %s\r\n", jsp->data.str.buff);
        }
    }

    /*
     * Handle current object - single object with multiple key-value pairs
     */
    if (jsp->stack_pos >= 4
        && jsp->stack[0].type == LWJSON_STREAM_TYPE_OBJECT
        && jsp->stack[1].type == LWJSON_STREAM_TYPE_KEY
        && jsp->stack[2].type == LWJSON_STREAM_TYPE_OBJECT
        && jsp->stack[3].type == LWJSON_STREAM_TYPE_KEY) {
        /* Check for current weather */
        if (strcmp(jsp->stack[1].meta.name, "current") == 0) {
            /*
             * Process the "weather part"
             */
            if (jsp->stack_pos >= 7
                && jsp->stack[4].type == LWJSON_STREAM_TYPE_ARRAY
                && jsp->stack[5].type == LWJSON_STREAM_TYPE_OBJECT
                && jsp->stack[6].type == LWJSON_STREAM_TYPE_KEY
                && strcmp(jsp->stack[3].meta.name, "weather") == 0) {
                
                if (strcmp(jsp->stack[6].meta.name, "id") == 0) {
                    printf("Current weather %d id: %u\r\n", (int)jsp->stack[4].meta.index, (unsigned)atoll(jsp->data.str.buff));
                } else if (strcmp(jsp->stack[6].meta.name, "main") == 0) {
                    printf("Current weather %d main: %s\r\n", (int)jsp->stack[4].meta.index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[6].meta.name, "description") == 0) {
                    printf("Current weather %d description: %s\r\n", (int)jsp->stack[4].meta.index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[6].meta.name, "icon") == 0) {
                    printf("Current weather %d icon: %s\r\n", (int)jsp->stack[4].meta.index, jsp->data.str.buff);
                }
            } else if (strcmp(jsp->stack[3].meta.name, "dt") == 0) {
                printf("Current weather dt: %u\r\n", (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[3].meta.name, "sunrise") == 0) {
                printf("Current weather sunrise: %u\r\n", (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[3].meta.name, "sunset") == 0) {
                printf("Current weather sunset: %u\r\n", (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[3].meta.name, "temp") == 0) {
                printf("Current weather temp: %f\r\n", strtod(jsp->data.str.buff, NULL));
            } else if (strcmp(jsp->stack[3].meta.name, "feels_like") == 0) {
                printf("Current weather feels_like: %f\r\n", strtod(jsp->data.str.buff, NULL));
            } else if (strcmp(jsp->stack[3].meta.name, "pressure") == 0) {
                printf("Current weather pressure: %u\r\n", (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[3].meta.name, "humidity") == 0) {
                printf("Current weather humidity: %u\r\n", (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[3].meta.name, "uvi") == 0) {
                printf("Current weather uvi: %u\r\n", (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[3].meta.name, "clouds") == 0) {
                printf("Current weather clouds: %u\r\n", (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[3].meta.name, "visibility") == 0) {
                printf("Current weather visibility: %u\r\n", (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[3].meta.name, "wind_speed") == 0) {
                printf("Current weather wind_speed: %f\r\n", strtod(jsp->data.str.buff, NULL));
            } else if (strcmp(jsp->stack[3].meta.name, "wind_deg") == 0) {
                printf("Current weather wind_deg: %u\r\n", (unsigned)atoll(jsp->data.str.buff));
            }
        }
    }

    /* 
     * Process the various object in specific JSON order
     */
    if (jsp->stack_pos >= 5
        /* First build the order... */
        && jsp->stack[0].type == LWJSON_STREAM_TYPE_OBJECT
        && jsp->stack[1].type == LWJSON_STREAM_TYPE_KEY
        && jsp->stack[2].type == LWJSON_STREAM_TYPE_ARRAY
        && jsp->stack[3].type == LWJSON_STREAM_TYPE_OBJECT
        && jsp->stack[4].type == LWJSON_STREAM_TYPE_KEY) {
        
        /*
         * Handle daily forecast objects
         */
        if (strcmp(jsp->stack[1].meta.name, "daily") == 0) {
            /* Analyze objects for temp and feels like object */
            if (jsp->stack_pos >= 7
                && jsp->stack[5].type == LWJSON_STREAM_TYPE_OBJECT
                && jsp->stack[6].type == LWJSON_STREAM_TYPE_KEY) {
                if (strcmp(jsp->stack[4].meta.name, "temp") == 0) {
                    /* Parsing of temp object */    
                    if (strcmp(jsp->stack[6].meta.name, "min") == 0) {
                        printf("Day %2d temp min: %s\r\n", (int)jsp->stack[2].meta.index, jsp->data.str.buff);
                    } else if (strcmp(jsp->stack[6].meta.name, "max") == 0) {
                        printf("Day %2d temp max: %s\r\n", (int)jsp->stack[2].meta.index, jsp->data.str.buff);
                    } else if (strcmp(jsp->stack[6].meta.name, "day") == 0) {
                        printf("Day %2d temp day: %s\r\n", (int)jsp->stack[2].meta.index, jsp->data.str.buff);
                    } else if (strcmp(jsp->stack[6].meta.name, "night") == 0) {
                        printf("Day %2d temp night: %s\r\n", (int)jsp->stack[2].meta.index, jsp->data.str.buff);
                    } else if (strcmp(jsp->stack[6].meta.name, "eve") == 0) {
                        printf("Day %2d temp eve: %s\r\n", (int)jsp->stack[2].meta.index, jsp->data.str.buff);
                    } else if (strcmp(jsp->stack[6].meta.name, "morn") == 0) {
                        printf("Day %2d temp morn: %s\r\n", (int)jsp->stack[2].meta.index, jsp->data.str.buff);
                    }
                } else if (strcmp(jsp->stack[4].meta.name, "feels_like") == 0) {
                    /* Parsing of feels-like objects */
                    if (strcmp(jsp->stack[6].meta.name, "day") == 0) {
                        printf("Day %2d temp_feels_like day: %s\r\n", (int)jsp->stack[2].meta.index, jsp->data.str.buff);
                    } else if (strcmp(jsp->stack[6].meta.name, "night") == 0) {
                        printf("Day %2d temp_feels_like night: %s\r\n", (int)jsp->stack[2].meta.index, jsp->data.str.buff);
                    } else if (strcmp(jsp->stack[6].meta.name, "eve") == 0) {
                        printf("Day %2d temp_feels_like eve: %s\r\n", (int)jsp->stack[2].meta.index, jsp->data.str.buff);
                    } else if (strcmp(jsp->stack[6].meta.name, "morn") == 0) {
                        printf("Day %2d temp_feels_like morn: %s\r\n", (int)jsp->stack[2].meta.index, jsp->data.str.buff);
                    }
                }
            } else if (jsp->stack_pos >= 8
                && jsp->stack[5].type == LWJSON_STREAM_TYPE_ARRAY
                && jsp->stack[6].type == LWJSON_STREAM_TYPE_OBJECT
                && jsp->stack[7].type == LWJSON_STREAM_TYPE_KEY
                && strcmp(jsp->stack[4].meta.name, "weather") == 0) {
                
                if (strcmp(jsp->stack[7].meta.name, "id") == 0) {
                    printf("Day %2d weather %2d id: %s\r\n", (int)jsp->stack[2].meta.index, (int)jsp->stack[5].meta.index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[7].meta.name, "main") == 0) {
                    printf("Day %2d weather %2d main: %s\r\n", (int)jsp->stack[2].meta.index, (int)jsp->stack[5].meta.index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[7].meta.name, "description") == 0) {
                    printf("Day %2d weather %2d description: %s\r\n", (int)jsp->stack[2].meta.index, (int)jsp->stack[5].meta.index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[7].meta.name, "icon") == 0) {
                    printf("Day %2d weather %2d icon: %s\r\n", (int)jsp->stack[2].meta.index, (int)jsp->stack[5].meta.index, jsp->data.str.buff);
                }
            } else if (strcmp(jsp->stack[jsp->stack_pos - 1].meta.name, "dt") == 0) {
                printf("Day %2d dt: %u\r\n", (int)jsp->stack[2].meta.index, (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[jsp->stack_pos - 1].meta.name, "pressure") == 0) {
                printf("Day %2d pressure: %u\r\n", (int)jsp->stack[2].meta.index, (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[jsp->stack_pos - 1].meta.name, "humidity") == 0) {
                printf("Day %2d humidity: %u\r\n", (int)jsp->stack[2].meta.index, (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[jsp->stack_pos - 1].meta.name, "clouds") == 0) {
                printf("Day %2d clouds: %u\r\n", (int)jsp->stack[2].meta.index, (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[jsp->stack_pos - 1].meta.name, "snow") == 0) {
                printf("Day %2d snow: %f\r\n", (int)jsp->stack[2].meta.index, strtod(jsp->data.str.buff, NULL));
            } else if (strcmp(jsp->stack[jsp->stack_pos - 1].meta.name, "rain") == 0) {
                printf("Day %2d rain: %f\r\n", (int)jsp->stack[2].meta.index, strtod(jsp->data.str.buff, NULL));
            } else if (strcmp(jsp->stack[jsp->stack_pos - 1].meta.name, "uvi") == 0) {
                printf("Day %2d uvi: %f\r\n", (int)jsp->stack[2].meta.index, strtod(jsp->data.str.buff, NULL));
            }
        } else if (strcmp(jsp->stack[1].meta.name, "hourly") == 0) {
            if (jsp->stack_pos >= 8
                && jsp->stack[5].type == LWJSON_STREAM_TYPE_ARRAY
                && jsp->stack[6].type == LWJSON_STREAM_TYPE_OBJECT
                && jsp->stack[7].type == LWJSON_STREAM_TYPE_KEY
                && strcmp(jsp->stack[4].meta.name, "weather") == 0) {
                
                if (strcmp(jsp->stack[7].meta.name, "id") == 0) {
                    printf("Hour %2d weather %2d id: %s\r\n", (int)jsp->stack[2].meta.index, (int)jsp->stack[5].meta.index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[7].meta.name, "main") == 0) {
                    printf("Hour %2d weather %2d main: %s\r\n", (int)jsp->stack[2].meta.index, (int)jsp->stack[5].meta.index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[7].meta.name, "description") == 0) {
                    printf("Hour %2d weather %2d description: %s\r\n", (int)jsp->stack[2].meta.index, (int)jsp->stack[5].meta.index, jsp->data.str.buff);
                } else if (strcmp(jsp->stack[7].meta.name, "icon") == 0) {
                    printf("Hour %2d weather %2d icon: %s\r\n", (int)jsp->stack[2].meta.index, (int)jsp->stack[5].meta.index, jsp->data.str.buff);
                }
            }else if (strcmp(jsp->stack[4].meta.name, "dt") == 0) {
                printf("Hour %2d forecast for dt: %u\r\n", (int)jsp->stack[jsp->stack_pos - 3].meta.index, (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[4].meta.name, "temp") == 0) {
                printf("Hour %2d forecast for temp: %f\r\n", (int)jsp->stack[jsp->stack_pos - 3].meta.index, strtod(jsp->data.str.buff, NULL));
            } else if (strcmp(jsp->stack[4].meta.name, "feels_like") == 0) {
                printf("Hour %2d forecast for feels_like: %f\r\n", (int)jsp->stack[jsp->stack_pos - 3].meta.index, strtod(jsp->data.str.buff, NULL));
            } else if (strcmp(jsp->stack[4].meta.name, "pressure") == 0) {
                printf("Hour %2d forecast for pressure: %d\r\n", (int)jsp->stack[jsp->stack_pos - 3].meta.index, (int)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[4].meta.name, "humidity") == 0) {
                printf("Hour %2d forecast for humidity: %d\r\n", (int)jsp->stack[jsp->stack_pos - 3].meta.index, (int)atoll(jsp->data.str.buff));
            }
        } else if (strcmp(jsp->stack[1].meta.name, "minutely") == 0) {
            if (strcmp(jsp->stack[4].meta.name, "dt") == 0) {
                printf("Minute %2d forecast for dt: %u\r\n", (int)jsp->stack[jsp->stack_pos - 3].meta.index, (unsigned)atoll(jsp->data.str.buff));
            } else if (strcmp(jsp->stack[4].meta.name, "precipitation") == 0) {
                printf("Minute %2d forecast for precipitation: %u\r\n", (int)jsp->stack[jsp->stack_pos - 3].meta.index, (unsigned)atoll(jsp->data.str.buff));
            }
        }
    }
}
