/**
 * \file            lwjson.h
 * \brief           LwJSON - Lightweight JSON format parser
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
#ifndef LWJSON_HDR_H
#define LWJSON_HDR_H

#include <string.h>
#include <stdint.h>
#include "lwjson/lwjson_opt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWJSON Lightweight JSON format parser
 * \brief           LwJSON - Lightweight JSON format parser
 * \{
 */

/**
 * \brief           Get size of statically allocated array
 * \param[in]       x: Object to get array size of
 * \return          Number of elements in array
 */
#define LWJSON_ARRAYSIZE(x)         (sizeof(x) / sizeof((x)[0]))

/**
 * \brief           List of supported JSON types
 */
typedef enum {
    LWJSON_TYPE_STRING,                         /*!< String/Text format. Everything that has beginning and ending quote character */
    LWJSON_TYPE_NUMBER,                         /*!< Number type */
    LWJSON_TYPE_OBJECT,
    LWJSON_TYPE_ARRAY,
    LWJSON_TYPE_BOOLEAN,
    LWJSON_TYPE_NULL,
} lwjson_type_t;

/**
 * \brief           JSON token
 */
typedef struct lwjson_token {
    struct lwjson_token* next;                  /*!< Next token on a list */
    struct lwjson_token* parent;                /*!< Parent token (think about optimization and remove this one?) */
    lwjson_type_t type;                         /*!< Token type */
    const char* token_name;                     /*!< Token name (if exists) */
    size_t token_name_len;                      /*!< Length of token name (this is needed to support const input strings to parse) */
    union {
        struct {
            const char* token_value;            /*!< Value if type is not \ref LWJSON_TYPE_OBJECT or \ref LWJSON_TYPE_ARRAY */
            size_t token_value_len;             /*!< Length of token value (this is needed to support const input strings to parse) */
        } v;
        struct lwjson_token* first_child;       /*!< First children object */
    } u;
} lwjson_token_t;

/**
 * \brief           JSON result enumeration
 */
typedef enum {
    lwjsonOK = 0x00,                            /*!< Function returns successfully */
    lwjsonERR,
    lwjsonERRJSON,                              /*!< Error JSON format */
    lwjsonERRMEM,                               /*!< Memory error */
} lwjsonr_t;

/**
 * \brief           LwJSON instance
 */
typedef struct {
    lwjson_token_t* tokens;                     /*!< Pointer to array of tokens */
    size_t tokens_len;                          /*!< Size of all tokens */
    size_t next_free_token_pos;                 /*!< Position of next free token instance */
    lwjson_token_t first_token;                 /*!< First token on a list */
} lwjson_t;

lwjsonr_t   lwjson_init(lwjson_t* lw, lwjson_token_t* tokens, size_t tokens_len);
lwjsonr_t   lwjson_parse(lwjson_t* lw, const char* json_str);
lwjsonr_t   lwjson_reset(lwjson_t* lw);
lwjsonr_t   lwjson_free(lwjson_t* lw);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWJSON_HDR_H */
