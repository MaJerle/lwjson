/**
 * \file            lwjson_serializer.h
 * \brief           JSON serializer for building JSON strings incrementally
 */

/*
 * Copyright (c) 2025 Tilen MAJERLE
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
 * Version:         v1.9.0
 */
#ifndef LWJSON_SERIALIZER_HDR_H
#define LWJSON_SERIALIZER_HDR_H

/* Include system headers */
#include <stddef.h>
#include <stdint.h>

/* Include lwjson library */
#include "lwjson.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup        LWJSON_SERIALIZER JSON serializer
 * \brief           JSON serializer
 * \{
 */

/**
 * \brief           Stack element types for tracking nesting
 */
typedef enum {
    LWJSON_SERIALIZER_TYPE_OBJECT = 0, /*!< Object type on stack */
    LWJSON_SERIALIZER_TYPE_ARRAY,      /*!< Array type on stack */
} lwjson_serializer_stack_type_t;

/**
 * \brief           Callback function type for serialization
 * \param[in]       ctx: User context passed to callback
 * \param[in]       str: String to serialize
 * \param[in]       str_len: Length in bytes of string to serialize
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
typedef lwjsonr_t (*lwjson_serializer_callback_fn)(void* ctx, const char* str, size_t str_len);

/**
 * \brief           Context structure for default buffered output
 */
typedef struct {
    char* buffer;    /*!< Pointer to the buffer for serialized output */
    size_t capacity; /*!< Maximum capacity of the buffer */
    size_t length;   /*!< Current length of the serialized output */
} lwjson_serializer_default_ctx_t;

/**
 * \brief           JSON serializer structure
 */
typedef struct {
    lwjson_serializer_stack_type_t stack[LWJSON_CFG_SERIALIZER_MAX_STACK_DEPTH]; /*!< Stack for tracking nesting */
    int32_t top;                                                                 /*!< Current top of stack */
    uint8_t need_comma;                          /*!< Flag indicating if comma is needed before next element */
    lwjson_serializer_callback_fn callback;      /*!< Callback function for serialization */
    void* ctx;                                   /*!< User context passed to callback */
    lwjson_serializer_default_ctx_t default_ctx; /*!< Default context for buffered output */
} lwjson_serializer_t;

/**
 * \brief           Unsigned integer type used by serializer integer APIs
 */
#if LWJSON_CFG_SERIALIZER_USE_64BIT
typedef uint64_t lwjson_serializer_uint_t;
#else
typedef uint32_t lwjson_serializer_uint_t;
#endif

/**
 * \brief           Signed integer type used by serializer integer APIs
 */
#if LWJSON_CFG_SERIALIZER_USE_64BIT
typedef int64_t lwjson_serializer_int_t;
#else
typedef int32_t lwjson_serializer_int_t;
#endif

lwjsonr_t lwjson_serializer_init(lwjson_serializer_t* serializer, char* user_buffer, size_t buffer_size);
lwjsonr_t lwjson_serializer_finalize(lwjson_serializer_t* serializer, size_t* total_length);
lwjsonr_t lwjson_serializer_init_callback(lwjson_serializer_t* serializer, lwjson_serializer_callback_fn callback,
                                          void* ctx);
lwjsonr_t lwjson_serializer_start_object(lwjson_serializer_t* serializer, const char* key, size_t key_len);
lwjsonr_t lwjson_serializer_start_array(lwjson_serializer_t* serializer, const char* key, size_t key_len);
lwjsonr_t lwjson_serializer_end_object(lwjson_serializer_t* serializer);
lwjsonr_t lwjson_serializer_end_array(lwjson_serializer_t* serializer);
lwjsonr_t lwjson_serializer_add_string(lwjson_serializer_t* serializer, const char* key, size_t key_len,
                                       const char* value, size_t value_len);
lwjsonr_t lwjson_serializer_add_uint(lwjson_serializer_t* serializer, const char* key, size_t key_len,
                                     lwjson_serializer_uint_t value);
lwjsonr_t lwjson_serializer_add_int(lwjson_serializer_t* serializer, const char* key, size_t key_len,
                                    lwjson_serializer_int_t value);
lwjsonr_t lwjson_serializer_add_float(lwjson_serializer_t* serializer, const char* key, size_t key_len, double value);
lwjsonr_t lwjson_serializer_add_bool(lwjson_serializer_t* serializer, const char* key, size_t key_len, uint8_t value);
lwjsonr_t lwjson_serializer_add_null(lwjson_serializer_t* serializer, const char* key, size_t key_len);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif

#endif /* LWJSON_SERIALIZER_HDR_H */
