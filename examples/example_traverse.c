#include <stdio.h>
#include "lwjson/lwjson.h"

/* LwJSON instance and tokens */
static lwjson_token_t tokens[128];
static lwjson_t lwjson;

/* Parse JSON */
void
example_traverse_run(void) {
    /* Initialize and pass statically allocated tokens */
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));

    /* Try to parse input string */
    if (lwjson_parse(&lwjson, "{\"mykey\":\"myvalue\",\"num\":1,\"obj\":{},\"arr\":[1,2,3,4]}") == lwjsonOK) {
        lwjson_token_t* t;
        printf("JSON parsed..\r\n");

        /* Get very first token as top object */
        t = lwjson_get_first_token(&lwjson);
        if (t->type == LWJSON_TYPE_ARRAY) {
            printf("JSON starts with array..\r\n");
        } else if (t->type == LWJSON_TYPE_OBJECT) {
            printf("JSON starts with object..\r\n");
        } else {
            printf("This should never happen..\r\n");
        }

        /* Now print all keys in the object */
        for (const lwjson_token_t* tkn = lwjson_get_first_child(t); tkn != NULL; tkn = tkn->next) {
            printf("Token: %.*s", (int)tkn->token_name_len, tkn->token_name);
            if (tkn->type == LWJSON_TYPE_ARRAY || tkn->type == LWJSON_TYPE_OBJECT) {
                printf(": Token is array or object...check children tokens if any, in recursive mode..");
                /* Get first child of token */
                //lwjson_get_first_child(tkn);
            }
            printf("\n");
        }

        /* Call this when not used anymore */
        lwjson_free(&lwjson);
    }
}
