#include <stdio.h>
#include <string.h>
#include "lwjson/lwjson.h"

const char* json_input_text_simple = "{\n\t\"name\":\"Tilen\",\n\t\"age\":27,\n\t\"pass\":true\n}\r\n\r\n";

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

int
main() {
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
    lwjson_parse(&lwjson, json_input_text_simple);

    printf("%s", json_input_text_simple);
    printf("%s", json_input_text_advanced);
    return 0;
}
