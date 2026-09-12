/**
 * \file            lwjson_serializer.c
 * \brief           JSON serializer for building JSON strings incrementally.
 *                  This module allows creating JSON strings by adding elements one by one,
 *                  handling nested objects and arrays, and ensuring proper formatting.
 *                  It supports both direct buffer output and callback-based streaming.
 *
 */

/*
 * Copyright (c) 2026 Tilen MAJERLE
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
/* Include system headers */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/* Include lwjson headers */
#include "lwjson/lwjson_serializer.h"
#include "lwjson/lwjson_utils.h"

/**
 * \brief           Append single character to serializer output
 * \param[in,out]   serializer: Pointer to serializer structure
 * \param[in]       ch: Character to append
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_append_char(lwjson_serializer_t* serializer, char ch) {
    return serializer->callback(serializer->ctx, &ch, 1U);
}

/**
 * \brief           Append string to serializer output
 * \param[in,out]   serializer: Pointer to serializer structure
 * \param[in]       str: String to append
 * \param[in]       str_len: Length of string to append
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_append_string(lwjson_serializer_t* serializer, const char* str, size_t str_len) {
    return serializer->callback(serializer->ctx, str, str_len);
}

/**
 * \brief           Add comma if needed before next element
 * \param[in,out]   serializer: Pointer to serializer structure
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_add_comma_if_needed(lwjson_serializer_t* serializer) {
    if (serializer->need_comma) {
        lwjsonr_t res = prv_append_char(serializer, ',');
        if (res != lwjsonOK) {
            return res;
        }
        serializer->need_comma = 0;
    }
    return lwjsonOK;
}

/**
 * \brief           Add key with quotes and colon
 * \param[in,out]   serializer: Pointer to serializer structure
 * \param[in]       key: Key string
 * \param[in]       key_len: Length of key string
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_add_key(lwjson_serializer_t* serializer, const char* key, size_t key_len) {
    lwjsonr_t res;

    if (key == NULL || key_len == 0) {
        return lwjsonOK; /* No key for array elements */
    }

    /* Add opening quote */
    res = prv_append_char(serializer, '"');
    if (res != lwjsonOK) {
        return res;
    }

    /* Escape key using callback */
    res = lwjson_utils_escape_string_cb(key, key_len, serializer->callback, serializer->ctx);
    if (res != lwjsonOK) {
        return res;
    }

    /* Add closing quote */
    res = prv_append_char(serializer, '"');
    if (res != lwjsonOK) {
        return res;
    }

    /* Add colon */
    res = prv_append_char(serializer, ':');
    if (res != lwjsonOK) {
        return res;
    }

    return lwjsonOK;
}

/**
 * \brief           Push element type onto stack
 * \param[in,out]   serializer: Pointer to serializer structure
 * \param[in]       type: Type to push onto stack
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_stack_push(lwjson_serializer_t* serializer, lwjson_serializer_stack_type_t type) {
    if (serializer->top >= LWJSON_CFG_SERIALIZER_MAX_STACK_DEPTH - 1) {
        return lwjsonERRINVAL; /* Stack overflow */
    }

    ++serializer->top;
    serializer->stack[serializer->top] = type;
    return lwjsonOK;
}

/**
 * \brief           Add string value to serializer
 * \param[in,out]   serializer: Pointer to serializer structure
 * \param[in]       key: Key string
 * \param[in]       key_len: Length of key string
 * \param[in]       value: Value string
 * \param[in]       value_len: Length of value string
 * \param[in]       type: Type of the value
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_add_element(lwjson_serializer_t* serializer, const char* key, size_t key_len, const char* value, size_t value_len,
                lwjson_type_t type) {
    lwjsonr_t res;

    /* Parameter validation */
    if (serializer == NULL || value == NULL) {
        return lwjsonERRNULL;
    }

    /* Add comma if needed */
    res = prv_add_comma_if_needed(serializer);
    if (res != lwjsonOK) {
        return res;
    }

    /* Add key if provided */
    res = prv_add_key(serializer, key, key_len);
    if (res != lwjsonOK) {
        return res;
    }

    if (type == LWJSON_TYPE_STRING) {

        /* Add opening quote */
        res = prv_append_char(serializer, '"');
        if (res != lwjsonOK) {
            return res;
        }

        /* Escape value */
        res = lwjson_utils_escape_string_cb(value, value_len, serializer->callback, serializer->ctx);
        if (res != lwjsonOK) {
            return res;
        }

        /* Add closing quote */
        res = prv_append_char(serializer, '"');
        if (res != lwjsonOK) {
            return res;
        }
    } else {
        /* Add value */
        res = prv_append_string(serializer, value, value_len);
        if (res != lwjsonOK) {
            return res;
        }
    }

    if (type == LWJSON_TYPE_OBJECT) {
        /* Push object type onto stack */
        res = prv_stack_push(serializer, LWJSON_SERIALIZER_TYPE_OBJECT);
    } else if (type == LWJSON_TYPE_ARRAY) {
        /* Push array type onto stack */
        res = prv_stack_push(serializer, LWJSON_SERIALIZER_TYPE_ARRAY);
    } else {
        serializer->need_comma = 1;
    }

    return res;
}

/**
 * \brief           Pop element type from stack
 * \param[in,out]   serializer: Pointer to serializer structure
 * \param[out]      type: Pointer to store popped type
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_stack_pop(lwjson_serializer_t* serializer, lwjson_serializer_stack_type_t* type) {
    if (serializer->top < 0) {
        return lwjsonERRINVAL; /* Stack underflow */
    }

    if (type != NULL) {
        *type = serializer->stack[serializer->top];
    }
    --serializer->top;
    return lwjsonOK;
}

/**
 * \brief           End a container (object/array) in the serializer
 * \param[in,out]   serializer: Pointer to serializer structure
 * \param[in]       type: Type of container to end
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_end_container(lwjson_serializer_t* serializer, lwjson_serializer_stack_type_t type) {
    lwjsonr_t res;
    lwjson_serializer_stack_type_t stack_type;

    /* Parameter validation */
    if (serializer == NULL) {
        return lwjsonERRNULL;
    }

    /* Pop container type from stack */
    res = prv_stack_pop(serializer, &stack_type);
    if (res != lwjsonOK) {
        return res;
    }

    /* Check for type mismatch */
    if (stack_type != type) {
        return lwjsonERRJSON; /* Type mismatch */
    }

    /* Add closing bracket for object/array */
    if (type == LWJSON_SERIALIZER_TYPE_OBJECT) {
        res = prv_append_string(serializer, "}", 1);
    } else {
        res = prv_append_string(serializer, "]", 1);
    }
    if (res != lwjsonOK) {
        return res;
    }

    serializer->need_comma = 1;
    return lwjsonOK;
}

/**
 * \brief           Default callback function for serialization to buffer
 * \param[in]       ctx: User context (lwjson_serializer_default_ctx_t)
 * \param[in]       str: String chunk to serialize
 * \param[in]       str_len: Length of the string chunk
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_default_callback(void* ctx, const char* str, size_t str_len) {
    lwjson_serializer_default_ctx_t* default_ctx = ctx;
    if ((default_ctx->length + str_len) > default_ctx->capacity) {
        return lwjsonERRBUF;
    }
    LWJSON_MEMCPY(&default_ctx->buffer[default_ctx->length], str, str_len);
    default_ctx->length += str_len;
    return lwjsonOK;
}

/**
 * \brief           Initialize JSON serializer with user buffer
 * \param[in,out]   serializer: Pointer to serializer structure
 * \param[in]       user_buffer: User-provided buffer for JSON output
 * \param[in]       buffer_size: Size of user buffer
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_init(lwjson_serializer_t* serializer, char* user_buffer, size_t buffer_size) {
    /* Parameter validation */
    if (serializer == NULL || user_buffer == NULL) {
        return lwjsonERRNULL;
    }

    if (buffer_size == 0) {
        return lwjsonERRINVAL;
    }

    /* Initialize serializer structure */
    serializer->top = -1;
    serializer->need_comma = 0;
    serializer->callback = prv_default_callback;
    serializer->default_ctx.buffer = user_buffer;
    serializer->default_ctx.capacity = buffer_size;
    serializer->default_ctx.length = 0;
    serializer->ctx = &serializer->default_ctx;

    return lwjsonOK;
}

/**
 * \brief           Finalize serialization and get total length of serialized data
 * \param[in,out]   serializer: Pointer to serializer structure
 * \param[out]      total_length: Pointer to store total length of serialized data
 *                  (only valid when using default buffer callback, can be NULL)
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_finalize(lwjson_serializer_t* serializer, size_t* total_length) {
    /* Parameter validation */
    if (serializer == NULL) {
        return lwjsonERRNULL;
    }

    if (serializer->top != -1) {
        return lwjsonERRINVAL; /* Not all objects/arrays are closed */
    }

    /* Return length only if total_length is requested */
    if (total_length != NULL) {
        *total_length = serializer->default_ctx.length;
    }
    return lwjsonOK;
}

/**
 * \brief           Initialize serializer with callback function
 * \param[in,out]   serializer: Pointer to serializer structure
 * \param[in]       callback: Callback function to execute for output
 * \param[in]       ctx: User context to pass to callback
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_init_callback(lwjson_serializer_t* serializer, lwjson_serializer_callback_fn callback, void* ctx) {
    /* Parameter validation */
    if (serializer == NULL || callback == NULL) {
        return lwjsonERRNULL;
    }

    serializer->top = -1;
    serializer->need_comma = 0;
    serializer->callback = callback;
    serializer->ctx = ctx;

    return lwjsonOK;
}

/**
 * \brief           Start a new JSON object
 *
 * Creates a new JSON object in the serializer output. The object can be a root object (key is `NULL`),
 * a named object within another object (key provided), or an unnamed object within an array (key is `NULL`).
 *
 * \param[in,out]   serializer: Pointer to serializer structure. Must not be `NULL`.
 * \param[in]       key: Object key name. Use `NULL` for root object or array element.
 * \param[in]       key_len: Length of key string in bytes. Ignored if key is `NULL`.
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_start_object(lwjson_serializer_t* serializer, const char* key, size_t key_len) {
    return prv_add_element(serializer, key, key_len, "{", 1U, LWJSON_TYPE_OBJECT);
}

/**
 * \brief           Start a new JSON array
 *
 * Creates a new JSON array in the serializer output. The array can be a root array (key is `NULL`),
 * a named array within an object (key provided), or an unnamed array within another array (key is `NULL`).
 *
 * \param[in,out]   serializer: Pointer to serializer structure. Must not be `NULL`.
 * \param[in]       key: Array key name. Use `NULL` for root array or array element.
 * \param[in]       key_len: Length of key string in bytes. Ignored if key is `NULL`.
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_start_array(lwjson_serializer_t* serializer, const char* key, size_t key_len) {
    return prv_add_element(serializer, key, key_len, "[", 1U, LWJSON_TYPE_ARRAY);
}

/**
 * \brief           End current JSON object
 *
 * Closes the current JSON object by writing the closing brace `}`. Must be called after
 * all object members have been added. The number of end_object calls must match the number
 * of start_object calls.
 *
 * \param[in,out]   serializer: Pointer to serializer structure. Must not be `NULL`.
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_end_object(lwjson_serializer_t* serializer) {
    return prv_end_container(serializer, LWJSON_SERIALIZER_TYPE_OBJECT);
}

/**
 * \brief           End current JSON array
 *
 * Closes the current JSON array by writing the closing bracket `]`. Must be called after
 * all array elements have been added. The number of end_array calls must match the number
 * of start_array calls.
 *
 * \param[in,out]   serializer: Pointer to serializer structure. Must not be `NULL`.
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_end_array(lwjson_serializer_t* serializer) {
    return prv_end_container(serializer, LWJSON_SERIALIZER_TYPE_ARRAY);
}

/**
 * \brief           Add string value to JSON
 *
 * Adds a string value to the JSON output. The string is properly escaped
 * according to JSON specification. For object members, provide a key.
 * For array elements, set key to `NULL`.
 *
 * \param[in,out]   serializer: Pointer to serializer structure. Must not be `NULL`.
 * \param[in]       key: Key name for object member. Use `NULL` for array element.
 * \param[in]       key_len: Length of key string in bytes. Ignored if key is `NULL`.
 * \param[in]       value: String value to add. Must not be `NULL`.
 * \param[in]       value_len: Length of value string in bytes.
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_add_string(lwjson_serializer_t* serializer, const char* key, size_t key_len, const char* value,
                             size_t value_len) {
    return prv_add_element(serializer, key, key_len, value, value_len, LWJSON_TYPE_STRING);
}

/**
 * \brief           Add unsigned integer value to JSON
 *
 * Adds an unsigned 64-bit integer to the JSON output. The number is converted to string
 * representation without quotes.
 *
 * \param[in,out]   serializer: Pointer to serializer structure. Must not be `NULL`.
 * \param[in]       key: Key name for object member. Use `NULL` for array element.
 * \param[in]       key_len: Length of key string in bytes. Ignored if key is `NULL`.
 * \param[in]       value: Unsigned integer value to add (0 to 18446744073709551615).
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_add_uint(lwjson_serializer_t* serializer, const char* key, size_t key_len,
                           lwjson_serializer_uint_t value) {
    char number_str[32]; /* Large enough for a 64-bit unsigned integer */
    int32_t num_len;

    /* Convert number to string */
#if LWJSON_CFG_SERIALIZER_USE_64BIT
    num_len = snprintf(number_str, sizeof(number_str), "%" PRIu64, value);
#else
    num_len = snprintf(number_str, sizeof(number_str), "%" PRIu32, value);
#endif
    if (num_len < 0) {
        return lwjsonERRINVAL;
    }

    return prv_add_element(serializer, key, key_len, number_str, (size_t)num_len, LWJSON_TYPE_NUM_INT);
}

/**
 * \brief           Add signed integer value to JSON
 *
 * Adds a signed 64-bit integer to the JSON output. The number is converted to string
 * representation without quotes. Negative numbers include a minus sign.
 *
 * \param[in,out]   serializer: Pointer to serializer structure. Must not be `NULL`.
 * \param[in]       key: Key name for object member. Use `NULL` for array element.
 * \param[in]       key_len: Length of key string in bytes. Ignored if key is `NULL`.
 * \param[in]       value: Signed integer value to add (-9223372036854775808 to 9223372036854775807).
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_add_int(lwjson_serializer_t* serializer, const char* key, size_t key_len,
                          lwjson_serializer_int_t value) {
    char number_str[32]; /* Large enough for a 64-bit signed integer */
    int32_t num_len;

    /* Convert number to string */
#if LWJSON_CFG_SERIALIZER_USE_64BIT
    num_len = snprintf(number_str, sizeof(number_str), "%" PRId64, value);
#else
    num_len = snprintf(number_str, sizeof(number_str), "%" PRId32, value);
#endif
    if (num_len < 0) {
        return lwjsonERRINVAL;
    }

    return prv_add_element(serializer, key, key_len, number_str, (size_t)num_len, LWJSON_TYPE_NUM_INT);
}

/**
 * \brief           Add floating point value to JSON
 *
 * Adds a double-precision floating point number to the JSON output. The number is converted
 * to string representation using %.15g format (15 significant digits) without quotes.
 *
 * \param[in,out]   serializer: Pointer to serializer structure. Must not be `NULL`.
 * \param[in]       key: Key name for object member. Use `NULL` for array element.
 * \param[in]       key_len: Length of key string in bytes. Ignored if key is `NULL`.
 * \param[in]       value: Floating point value to add (IEEE 754 double precision).
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_add_float(lwjson_serializer_t* serializer, const char* key, size_t key_len, double value) {
    char number_str[64]; /* Large enough for double */
    int32_t num_len;

    /* Convert number to string */
    num_len = snprintf(number_str, sizeof(number_str), "%.15g", value);
    if (num_len < 0) {
        return lwjsonERRINVAL;
    }

    return prv_add_element(serializer, key, key_len, number_str, (size_t)num_len, LWJSON_TYPE_NUM_REAL);
}

/**
 * \brief           Add boolean value to JSON
 *
 * Adds a boolean value to the JSON output. The value is represented as `true` or `false`
 * literal (without quotes).
 *
 * \param[in,out]   serializer: Pointer to serializer structure. Must not be `NULL`.
 * \param[in]       key: Key name for object member. Use `NULL` for array element.
 * \param[in]       key_len: Length of key string in bytes. Ignored if key is `NULL`.
 * \param[in]       value: Boolean value (`0` for `false`, non-zero for `true`).
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_add_bool(lwjson_serializer_t* serializer, const char* key, size_t key_len, uint8_t value) {
    if (value) {
        return prv_add_element(serializer, key, key_len, "true", 4, LWJSON_TYPE_TRUE);
    } else {
        return prv_add_element(serializer, key, key_len, "false", 5, LWJSON_TYPE_FALSE);
    }
}

/**
 * \brief           Add null value to JSON
 *
 * Adds a null value to the JSON output. The value is represented as "null" literal
 * (without quotes).
 *
 * \param[in,out]   serializer: Pointer to serializer structure. Must not be `NULL`.
 * \param[in]       key: Key name for object member. Use `NULL` for array element.
 * \param[in]       key_len: Length of key string in bytes. Ignored if key is `NULL`.
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_serializer_add_null(lwjson_serializer_t* serializer, const char* key, size_t key_len) {
    return prv_add_element(serializer, key, key_len, "null", 4, LWJSON_TYPE_NULL);
}
