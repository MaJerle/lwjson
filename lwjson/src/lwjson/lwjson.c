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
#include <stdio.h>
#include "lwjson/lwjson.h"

static lwjson_token_t*
prv_alloc_token(lwjson_t* lw) {
    if (lw->next_free_token_pos < lw->tokens_len) {
        return &lw->tokens[lw->next_free_token_pos++];
    }
    return NULL;
}

static lwjsonr_t
prv_skip_blank(const char** p) {
    const char* s = *p;
    while (s != NULL && *s != '\0') {
        if (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n' || *s == '\f') {
            ++s;
            continue;
        }
        *p = s;
        return lwjsonOK;
    }
    return lwjsonERR;
}

static lwjsonr_t
prv_parse_string(const char** p, const char** pout, size_t* poutlen) {
    const char* s;
    lwjsonr_t res = lwjsonOK;
    size_t len = 0;

    if ((res = prv_skip_blank(p)) != lwjsonOK) {
        return res;
    }
    s = *p;
    if (*s++ != '"') {
        return lwjsonERRJSON;
    }
    *pout = s;
    /* Parse string but take care of escape characters */
    for (char prev_ch = '\0';; ++s, ++len) {
        if (s == NULL || *s == '\0') {
            return lwjsonERRJSON;
        }
        /* Check end of string */
        if (*s == '"') {
            if (prev_ch != '\\') {
                ++s;
                break;
            }
        }
        prev_ch = *s;
    }
    *poutlen = len;
    if (*s == '"') {
        ++s;
    }
    if ((res = prv_skip_blank(&s)) != lwjsonOK) {
        return res;
    }
    *p = s;
    return res;
}

static lwjsonr_t
prv_parse_property_name(const char** p, lwjson_token_t* t) {
    const char* s = *p;
    lwjsonr_t res;

    if ((res = prv_parse_string(p, &t->token_name, &t->token_name_len)) != lwjsonOK) {
        return res;
    }
    s = *p;
    if (*s != ':') {
        return lwjsonERRJSON;
    }
    ++s;
    prv_skip_blank(&s);
    *p = s;
    return lwjsonOK;
}

lwjsonr_t
lwjson_init(lwjson_t* lw, lwjson_token_t* tokens, size_t tokens_len) {
    memset(lw, 0x00, sizeof(*lw));
    memset(tokens, 0x00, sizeof(*tokens) * tokens_len);
    lw->tokens = tokens;
    lw->tokens_len = tokens_len;
    lw->first_token.type = LWJSON_TYPE_OBJECT;
    return lwjsonOK;
}

lwjsonr_t
lwjson_parse(lwjson_t* lw, const char* json_str) {
    lwjsonr_t res = lwjsonOK;
    const char* p = json_str;
    lwjson_token_t* t, *to = &lw->first_token;
    uint8_t first_check = 0;

    /* Process all characters */
    while (p != NULL && *p != '\0') {
        /* Filter out blanks */
        if ((res = prv_skip_blank(&p)) != lwjsonOK) {
            goto ret;
        }
        if (!first_check) {
            first_check = 1;
            if (*p == '{') {
                to->type = LWJSON_TYPE_OBJECT;
            } else if (*p == '[') {
                to->type = LWJSON_TYPE_ARRAY;
            } else {
                res = lwjsonERRMEM;
                goto ret;
            }
            ++p;
            continue;
        }
        if (*p == ',') {
            ++p;
            continue;
        }

        /* Check if end of object or array*/
        if (*p == (to->type == LWJSON_TYPE_OBJECT ? '}' : ']')) {
            lwjson_token_t* parent = to->next;
            to->next = NULL;
            to = parent;
            ++p;

            /* End of string, check if properly terminated*/
            if (to == NULL) {
                res = (p == NULL || *p == '\0') ? lwjsonOK : lwjsonERR;
                goto ret;
            }
            continue;
        }

        /* Allocate new token */
        t = prv_alloc_token(lw);
        if (t == NULL) {
            res = lwjsonERRMEM;
            goto ret;
        }

        /* If object type is not array, first thing is property that starts with quotes */
        if (to->type != LWJSON_TYPE_ARRAY) {
            if (*p != '"') {
                res = lwjsonERRJSON;
                goto ret;
            }
            if ((res = prv_parse_property_name(&p, t)) != lwjsonOK) {
                goto ret;
            }
        }
        
        /* Add element to linked list */
        if (to->u.first_child == NULL) {
            to->u.first_child = t;
        } else {
            lwjson_token_t* c;
            for (c = to->u.first_child; c->next != NULL; c = c->next) {}
            c->next = t;
        }

        /* Check next character to process */
        switch (*p) {
            case '{':
            case '[':
                t->type = *p == '{' ? LWJSON_TYPE_OBJECT : LWJSON_TYPE_ARRAY;
                t->next = to;           /* Temporary saved as parent object */
                to = t;
                ++p;
                break;
            case '"':
                if (prv_parse_string(&p, &t->u.v.token_value, &t->u.v.token_value_len) == lwjsonOK) {
                    t->type = LWJSON_TYPE_STRING;
                }
                break;
            case 't':
                /* RFC is lower-case only */
                if (strncmp(p, "true", 4) == 0) {
                    t->type = LWJSON_TYPE_TRUE;
                    p += 4;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            case 'f':
                /* RFC is lower-case only */
                if (strncmp(p, "false", 5) == 0) {
                    t->type = LWJSON_TYPE_FALSE;
                    p += 5;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            case 'n':
                /* RFC is lower-case only */
                if (strncmp(p, "null", 4) == 0) {
                    t->type = LWJSON_TYPE_NULL;
                    p += 4;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            default:
                if (*p == '-' || (*p >= '0' && *p <= '9')) {
                    /* Valid number */
                    while (p != NULL && *p != '\0' && (*p != ',' && *p != ']' && *p != '}')) {
                        ++p;
                    }
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
        }

        /* Below code is used to check characters after valid tokens */
        if (t->type == LWJSON_TYPE_ARRAY || t->type == LWJSON_TYPE_OBJECT) {
            continue;
        }

        /*
         * Check what are values after the token value
         *
         * As per RFC, every token value may have one or more
         *  blank characters, followed by one of below options:
         *  - Comma separator for next token
         *  - End of array indication
         *  - End of object indication
         */
        if ((res = prv_skip_blank(&p)) != lwjsonOK) {
            goto ret;
        }
        /* Check if valid string is availabe after */
        if (p == NULL || *p == '\0' || (*p != ',' && *p != ']' && *p != '}')) {
            res = lwjsonERRJSON;
            goto ret;
        } else if (*p == ',') {                 /* Check to advance to next token immediatey */
            ++p;
        }
    }
    if (to != NULL) {
        to->token_name = NULL;
        to->token_name_len = 0;
    }
ret:
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
