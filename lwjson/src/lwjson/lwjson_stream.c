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

lwjson_stream_type_t
prv_stack_push(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type) {
    if (jsp->stack_pos < LWJSON_ARRAYSIZE(jsp->stack)) {
        jsp->stack[jsp->stack_pos++].type = type;
        /* TODO: Copy name in case of non-array object */
        return 1;
    }
    return 0;
}

lwjson_stream_type_t
prv_stack_pop(lwjson_stream_parser_t* jsp) {
    if (jsp->stack_pos > 0) {
        /* TODO: Check the return values */
        return jsp->stack[jsp->stack_pos--].type;
    }
    /* TODO: Replace 0 with define */
    return 0;
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
    switch (jsp->parse_state) {

        /*
         * Waiting for very first valid characters,
         * that is used to indicate start of JSON stream
         */
        case LWJSON_STREAM_STATE_WAITINGFIRSTCHAR: {
            if (c == '{' || c == '[') {

            }
            break;
        }
    }
}
