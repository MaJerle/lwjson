#include <stdio.h>
#include "lwjson/lwjson.h"
#include <string.h>
typedef struct example_data_struct_t {
    uint8_t k1[10];
    char *k2;
    int k2_len;
    int k2_pos;
} example_data_struct_t;

/* Test string to parser */
static const char* json_str = "{\"k1\":\"v1\",\"k2\":[true, false, true]}";

/* LwJSON stream parser */
static lwjson_stream_parser_t stream_parser;

/**
 * \brief           Callback function for various events
 * \param           jsp: JSON stream parser object
 * \param           type: Event type
 */
void
prv_example_callback_func(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type) {
    /* Get a value corresponsing to "k1" key */
    example_data_struct_t* data = (example_data_struct_t *)jsp->user_data;

    if (jsp->stack_pos >= 2                                /* Number of stack entries must be high */
        && jsp->stack[0].type == LWJSON_STREAM_TYPE_OBJECT /* First must be object */
        && jsp->stack[1].type == LWJSON_STREAM_TYPE_KEY    /* We need key to be before */
        && strcmp(jsp->stack[1].meta.name, "k1") == 0) {
        printf("Got key '%s' with value '%s'\r\n", jsp->stack[1].meta.name, jsp->data.str.buff);
        strncpy((char *)data->k1, jsp->data.str.buff, sizeof(data->k1) - 1);
    }
    if (jsp->stack_pos >= 2                                /* Number of stack entries must be high */
        && jsp->stack[0].type == LWJSON_STREAM_TYPE_OBJECT /* First must be object */
        && jsp->stack[1].type == LWJSON_STREAM_TYPE_KEY    /* We need key to be before */
        && strcmp(jsp->stack[1].meta.name, "k2") == 0) {
        printf("Got key '%s' with value '%s'\r\n", jsp->stack[1].meta.name, jsp->data.str.buff);
        if (jsp->stack_pos >= 3
            && jsp->stack[2].type == LWJSON_STREAM_TYPE_ARRAY && jsp->stack[2].meta.index < data->k2_len) {
            printf("Got array value '%s' index = %d \r\n", jsp->data.str.buff, jsp->stack[2].meta.index);
            data->k2[jsp->stack[2].meta.index] = (strncmp(jsp->data.str.buff, "true", 4) == 0);
            data->k2_pos = jsp->stack[2].meta.index + 1;
        }
    }
    (void)type;
}

/* Parse JSON */
void
example_stream_run(void) {
    lwjsonr_t res;
    printf("\r\n\r\nParsing stream\r\n");
    example_data_struct_t data;
    char k2_buff[10];
    data.k2 = k2_buff;
    data.k2_len = sizeof(k2_buff);
    data.k2_pos = 0;
    lwjson_stream_init(&stream_parser, prv_example_callback_func);
    lwjson_stream_set_user_data(&stream_parser, &data);
    /* Demonstrate as stream inputs */
    for (const char* c = json_str; *c != '\0'; ++c) {
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
    printf("Parsing completed\r\n");
    printf("data: k1 = '%s'\r\n", data.k1);
    for(int i = 0; i < data.k2_pos; i++) {
        printf("data: k2[%d] = %d\r\n", i, data.k2[i]);
    }
}

int main(void) {
    example_stream_run();
    return 0;
}