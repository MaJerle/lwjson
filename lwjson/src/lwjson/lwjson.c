/**
 * \file            lwjson.c
 * \brief           Lightweight JSON format parser
 */

/*
 * Copyright (c) 2020 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwJSON - Lightweight JSON format parser.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         $_version_$
 */
#include <string.h>
#include "lwjson/lwjson.h"

lwjson_token_t*
prv_alloc_token(lwjson_t* lw) {
    if (lw->next_free_token_pos < lw->tokens_len) {
        return &lw->tokens[lw->next_free_token_pos++];
    }
    return NULL;
}

lwjsonr_t
lwjson_init(lwjson_t* lw, lwjson_token_t* tokens, size_t tokens_len) {
    memset(lw, 0x00, sizeof(*lw));

    lw->tokens = tokens;
    lw->tokens_len = tokens_len;
    return lwjsonOK;
}

lwjsonr_t
lwjson_parse(lwjson_t* lw, const char* json_str) {
    lwjsonr_t res = lwjsonOK;
    const char* p = json_str;
    lwjson_token_t* t;

    /* Process all characters */
    while (p != NULL && *p != '\0') {
        /* Filter out blanks */
        if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || *p == '\f') {
            ++p;
            continue;
        }

        /* Allocate new token */
        t = prv_alloc_token(lw);
        if (t == NULL) {
            res = lwjsonERRMEM;
            break;
        }

        /* Check next character to process */
        switch (*p) {
            case '{':
            case '[':
                t->type = *p == '{' ? LWJSON_TYPE_OBJECT : LWJSON_TYPE_ARRAY;
        }
    }
    return res;
}

lwjsonr_t
lwjson_reset(lwjson_t* lw) {
    return lwjsonOK;
}

lwjsonr_t
lwjson_free(lwjson_t* lw) {
    return lwjsonOK;
}
