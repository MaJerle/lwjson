/**
 * \file            lwjson_stream.c
 * \brief           Lightweight JSON format parser
 */

/*
 * Copyright (c) 2022 Tilen MAJERLE
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
 * Version:         v1.5.0
 */
#include <string.h>
#include "lwjson/lwjson.h"

#if defined(LWJSON_DEV)
#include <stdio.h>
#define DEBUG_STRING_PREFIX_SPACES          "                                                                                                "
#define LWJSON_DEBUG(jsp, ...)              do {    \
    if ((jsp) != NULL) {                            \
        printf("%.*s", (int)(4 * (jsp)->stack_pos), DEBUG_STRING_PREFIX_SPACES); \
    }   \
    printf(__VA_ARGS__);    \
} while (0)

/* Strings for debug */
static const char* type_strings[] = {
    [LWJSON_STREAM_TYPE_NONE]       = "none",
    [LWJSON_STREAM_TYPE_OBJECT]     = "object",
    [LWJSON_STREAM_TYPE_OBJECT_END] = "object_end",
    [LWJSON_STREAM_TYPE_ARRAY]      = "array",
    [LWJSON_STREAM_TYPE_ARRAY_END]  = "array_end",
    [LWJSON_STREAM_TYPE_KEY]        = "key",
    [LWJSON_STREAM_TYPE_STRING]     = "string",
    [LWJSON_STREAM_TYPE_TRUE]       = "true",
    [LWJSON_STREAM_TYPE_FALSE]      = "false",
    [LWJSON_STREAM_TYPE_NULL]       = "null",
    [LWJSON_STREAM_TYPE_INT]        = "int",
    [LWJSON_STREAM_TYPE_REAL]       = "real",
};
#else
#define LWJSON_DEBUG(jsp, ...)              
#endif /* defined(LWJSON_DEV) */

uint8_t
prv_stack_push(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type) {
    if (jsp->stack_pos < LWJSON_ARRAYSIZE(jsp->stack)) {
        jsp->stack[jsp->stack_pos].type = type;
        LWJSON_DEBUG(jsp, "Pushed to stack: %s\r\n", type_strings[type]);
        jsp->stack_pos++;
        /* TODO: Copy name in case of non-array object */
        return 1;
    }
    return 0;
}

lwjson_stream_type_t
prv_stack_pop(lwjson_stream_parser_t* jsp) {
    if (jsp->stack_pos > 0) {
        jsp->stack_pos--;
        LWJSON_DEBUG(jsp, "Popped from stack: %s\r\n", type_strings[jsp->stack[jsp->stack_pos].type]);
        return jsp->stack[jsp->stack_pos].type;
    }
    return LWJSON_STREAM_TYPE_NONE;
}

lwjson_stream_type_t
prv_stack_get_top(lwjson_stream_parser_t* jsp) {
    if (jsp->stack_pos > 0) {
        return jsp->stack[jsp->stack_pos - 1].type;
    }
    return LWJSON_STREAM_TYPE_NONE;
}

/**
 * \brief           Initialize LwJSON stream object before parsing takes place
 * \param[in,out]   jsp: Stream JSON structure 
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_stream_init(lwjson_stream_parser_t* jsp) {
    jsp->parse_state = LWJSON_STREAM_STATE_WAITINGFIRSTCHAR;
    return lwjsonOK;
}

/**
 * \brief           Parse JSON string in streaming mode
 * \param[in,out]   jsp: Stream JSON structure 
 * \param[in]       c: Character to parse
 * \return          lwjsonr_t
 */
lwjsonr_t
lwjson_stream_parse(lwjson_stream_parser_t* jsp, char c) {
    /* Get first character first */
    if (jsp->parse_state == LWJSON_STREAM_STATE_WAITINGFIRSTCHAR
        && c != '{' && c != '[') {
        return lwjsonOK;
    }

    /*
     * Determine what to do from parsing state
     */
    switch (jsp->parse_state) {

        /*
         * Waiting for very first valid characters,
         * that is used to indicate start of JSON stream
         */
        case LWJSON_STREAM_STATE_WAITINGFIRSTCHAR:
        case LWJSON_STREAM_STATE_PARSING: {
            /* Determine start or object or an array */
            if (c == '{' || c == '[') {
                if (!prv_stack_push(jsp, c == '{' ? LWJSON_STREAM_TYPE_OBJECT : LWJSON_STREAM_TYPE_ARRAY)) {
                    return lwjsonERRMEM;
                }
                jsp->parse_state = LWJSON_STREAM_STATE_PARSING;

            /* Determine end or object or an array */
            } else if (c == '}' || c == ']') {
                if (!prv_stack_pop(jsp)) {
                    return lwjsonERR;
                }
                if (prv_stack_get_top(jsp) == LWJSON_STREAM_TYPE_KEY) {
                    prv_stack_pop(jsp);
                }

            /* Determine start of string - can be key or regular string (in array or after key) */
            } else if (c == '"') {
#if defined(LWJSON_DEV)
                lwjson_stream_type_t t = prv_stack_get_top(jsp);
                if (t == LWJSON_STREAM_TYPE_OBJECT) {
                    LWJSON_DEBUG(jsp, "Start of string parsing - expected key name in an object\r\n");
                } else if (t == LWJSON_STREAM_TYPE_KEY) {
                    LWJSON_DEBUG(jsp, "Start of string parsing - string value associated to previous key in an object\r\n");
                } else if (t == LWJSON_STREAM_TYPE_ARRAY) {
                    LWJSON_DEBUG(jsp, "Start of string parsing - string entry in an array\r\n");
                } 
#endif /* defined(LWJSON_DEV) */

                jsp->parse_state = LWJSON_STREAM_STATE_PARSING_STRING;
                memset(&jsp->data.str, 0x00, sizeof(jsp->data.str));

            /* Check for end of key character */
            } else if (c == ':') {
                lwjson_stream_type_t t = prv_stack_get_top(jsp);

                /*
                 * Color character means end of key.
                 *
                 * Valid key can only happen if its parent (top of current stack)
                 * is an object - otherwise trigger and error
                 */
                if (t == LWJSON_STREAM_TYPE_OBJECT) {
                    if (!prv_stack_push(jsp, LWJSON_STREAM_TYPE_KEY)) {
                        return lwjsonERRMEM;
                    }
                } else {
                   LWJSON_DEBUG(jsp, "Error - wrong ':' character\r\n");
                }
            /* Check if this is start of number */
            } else if (c == '-' || (c >= '0' && c <= '9')) {

            /* Check if it is start of either "true", "false" or "null" */
            } else if (c == 't' || c == 'f' || c == 'n') {

            }
            break;
        }

        /*
         * Parse any type of string in a sequence
         *
         * It is used for key or string in an object or an array
         */
        case LWJSON_STREAM_STATE_PARSING_STRING: {
            /* 
             * Quote character may trigger end of string, 
             * or if backslasled before - it is part of string
             */
            if (c == '"') {
                lwjson_stream_type_t t = prv_stack_get_top(jsp);
                if (t == LWJSON_STREAM_TYPE_OBJECT) {
                    LWJSON_DEBUG(jsp, "End of string parsing - object key name: \"%s\"\r\n", jsp->data.str.buff);
                } else if (t == LWJSON_STREAM_TYPE_KEY) {
                    LWJSON_DEBUG(jsp, "End of string parsing - string value associated to previous key in an object: \"%s\"\r\n", jsp->data.str.buff);
                    prv_stack_pop(jsp);
                } else if (t == LWJSON_STREAM_TYPE_ARRAY) {
                    LWJSON_DEBUG(jsp, "End of string parsing - an array string entry: \"%s\"\r\n", jsp->data.str.buff);
                }
                jsp->parse_state = LWJSON_STREAM_STATE_PARSING;
            } else {
                jsp->data.str.buff[jsp->data.str.buff_pos++] = c;
                /* TODO: Add 0 if necessary */
            }
            break;
        }

        /* TODO: Add other case statements */
        default:
            break;
    }
}
