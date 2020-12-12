#include <stdio.h>
#include "lwjson/lwjson.h"

/* LwJSON instance and tokens */
static lwjson_token_t tokens[128];
static lwjson_t lwjson;

/* Parse JSON */
void
example_minimal_run(void) {
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
    if (lwjson_parse(&lwjson, "{\"mykey\":\"myvalue\"}") == lwjsonOK) {
        const lwjson_token_t* t;
        printf("JSON parsed..\r\n");

        /* Find custom key in JSON */
        if ((t = lwjson_find(&lwjson, "mykey")) != NULL) {
            printf("Key found with data type: %d\r\n", (int)t->type);
        }

        /* Call this when not used anymore */
        lwjson_free(&lwjson);
    }
}
