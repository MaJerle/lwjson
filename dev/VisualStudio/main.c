#include <stdio.h>
#include <string.h>
#include "lwjson/lwjson.h"

const char* json_input_text_simple = "{\"name\":\"Tilen\",\"last_name\":\"Madjerle\",\"age\":27,\"pass\":true,\"children_ages\":[1,2,3,4,5,6,7,8,9,0]}";

const char* json_input_weather_ljubljana = "{\"coord\":{\"lon\":14.51,\"lat\":46.05},\"weather\":[{\"id\":600,\"main\":\"Snow\",\"description\":\"light snow\",\"icon\":\"13d\"},{\"id\":600,\"main\":\"Snow\",\"description\":\"light snow\",\"icon\":\"13d\"},{\"id\":600,\"main\":\"Snow\",\"description\":\"light snow\",\"icon\":\"13d\"},{\"id\":600,\"main\":\"Snow\",\"description\":\"light snow\",\"icon\":\"13d\"}],\"base\":\"stations\",\"main\":{\"temp\":-0.85,\"feels_like\":-4.05,\"temp_min\":-2,\"temp_max\":0,\"pressure\":1018,\"humidity\":79},\"visibility\":10000,\"wind\":{\"speed\":1,\"deg\":0},\"snow\":{\"1h\":0.31},\"clouds\":{\"all\":90},\"dt\":1606892861,\"sys\":{\"type\":1,\"id\":6815,\"country\":\"SI\",\"sunrise\":1606890313,\"sunset\":1606922289},\"timezone\":3600,\"id\":3196359,\"name\":\"Ljubljana\",\"cod\":200}";

const char* json_input_text_advanced = ""
"{\n"
"\t\"_id\": \"5fc567f48d6bf08d4c29a664\",\n"
"\t\"index\" : 0,\n"
"\t\"guid\" : \"0924c82c - e162 - 4148 - 9c8d - 3ae1a5f1f291\",\n"
"\t\"isActive\" : false,\n"
"\t\"latitude\" : -13.290682,\n"
"\t\"tags\" : [\n"
"\t\t\"laboris\",\n"
"\t\t\"sint\",\n"
"\t\t\"nulla\"\n"
"\t] ,\n"
"\t\"friends\" : [\n"
"\t\t{\n"
"\t\t\t\"id\": 0,\n"
"\t\t\t\"name\" : \"Mccormick Henson\"\n"
"\t\t},\n"
"\t\t{\n"
"\t\t\t\"id\": 1,\n"
"\t\t\t\"name\" : \"Pamela Landry\"\n"
"\t\t}\n"
"\t]\n"
"}\n";

lwjson_token_t tokens[128];
lwjson_t lwjson;

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
        printf("\"%.*s\"", (int)token->u.v.token_value_len, token->u.v.token_value);
    } else if (token->type == LWJSON_TYPE_NUMBER) {
        printf("%.*s", (int)token->u.v.token_value_len, token->u.v.token_value);
    } else if (token->type == LWJSON_TYPE_BOOLEAN) {
        printf("%.*s", (int)token->u.v.token_value_len, token->u.v.token_value);
    } else if (token->type == LWJSON_TYPE_NULL) {
        printf("NULL");
    }
    if (token->next != NULL) {
        printf(",");
    }
    printf("\n");
}

int
main() {
    lwjson_token_t* t;
    size_t token_cnt = 0;

#if 1
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
    //lwjson_parse(&lwjson, json_generated);
    lwjson_parse(&lwjson, json_input_text_simple);
    dump(&lwjson.first_token);
#else 
    /* Reset tokens for demo purpose */
    memset(tokens, 0x00, sizeof(tokens));

    /* First token */
    lwjson.first_token.next = NULL;
    lwjson.first_token.token_name = NULL;
    lwjson.first_token.type = LWJSON_TYPE_OBJECT;
    lwjson.first_token.u.first_child = &tokens[0];

    /* Create one token */
    tokens[0].type = LWJSON_TYPE_STRING;
    tokens[0].token_name = "name";
    tokens[0].token_name_len = 4;
    tokens[0].u.v.token_value = "Tilen";
    tokens[0].u.v.token_value_len = 5;
    tokens[0].next = &tokens[1];

    /* Create second token */
    tokens[1].type = LWJSON_TYPE_STRING;
    tokens[1].token_name = "surname";
    tokens[1].token_name_len = 7;
    tokens[1].u.v.token_value = "Majerle";
    tokens[1].u.v.token_value_len = 7;
    tokens[1].next = &tokens[2];

    /* Create third token */
    tokens[2].type = LWJSON_TYPE_ARRAY;
    tokens[2].token_name = "ages";
    tokens[2].token_name_len = 4;
    tokens[2].u.first_child = &tokens[3];
    tokens[2].next = &tokens[8];

    /* Create forth token */
    tokens[3].type = LWJSON_TYPE_STRING;
    tokens[3].u.v.token_value = "25";
    tokens[3].u.v.token_value_len = 2;
    tokens[3].next = &tokens[4];

    /* Create fifth token */
    tokens[4].type = LWJSON_TYPE_NUMBER;
    tokens[4].u.v.token_value = "87342";
    tokens[4].u.v.token_value_len = 5;
    tokens[4].next = &tokens[5];

    /* Create sixth token */
    tokens[5].type = LWJSON_TYPE_BOOLEAN;
    tokens[5].u.v.token_value = "true";
    tokens[5].u.v.token_value_len = 4;
    tokens[5].next = &tokens[6];

    /* Create seventh token */
    tokens[6].type = LWJSON_TYPE_BOOLEAN;
    tokens[6].u.v.token_value = "false";
    tokens[6].u.v.token_value_len = 5;
    tokens[6].next = &tokens[7];

    /* Create eight token */
    tokens[7].type = LWJSON_TYPE_NULL;
    tokens[7].next = NULL;

    /* Create next token */
    tokens[8].type = LWJSON_TYPE_OBJECT;
    tokens[8].token_name = "sub_objects";
    tokens[8].token_name_len = 11;
    tokens[8].u.first_child = &tokens[9];
    tokens[8].next = NULL;

    /* Create next token */
    tokens[9].type = LWJSON_TYPE_STRING;
    tokens[9].token_name = "name";
    tokens[9].token_name_len = 4;
    tokens[9].u.v.token_value = "Tilen";
    tokens[9].u.v.token_value_len = 5;
    tokens[9].next = &tokens[10];

    /* Create next token */
    tokens[10].type = LWJSON_TYPE_STRING;
    tokens[10].token_name = "test";
    tokens[10].token_name_len = 4;
    tokens[10].u.v.token_value = "value";
    tokens[10].u.v.token_value_len = 5;
    tokens[10].next = NULL;


    dump(&lwjson.first_token);
#endif

    //printf("%s", json_input_text_simple);
    //printf("%s", json_input_text_advanced);
    return 0;
}
