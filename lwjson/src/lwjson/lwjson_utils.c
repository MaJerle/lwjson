/**
 * \file            lwjson_utils.c
 * \brief           JSON string utility functions for character escaping and unescaping
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
/* Include lwjson headers */
#include "lwjson/lwjson_utils.h"

/**
 * \brief           Mapping escape characters to their JSON escape sequences
 *                  [0] = character that needs escaping
 *                  [1] = second char of escape sequence
 */
static const char escape_map[][2] = {
    {'"', '"'},   /* " -> \" */
    {'/', '/'},   /* / -> \/ */
    {'\\', '\\'}, /* \ -> \\ */
    {'\b', 'b'},  /* backspace -> \b */
    {'\f', 'f'},  /* form feed -> \f */
    {'\n', 'n'},  /* newline -> \n */
    {'\r', 'r'},  /* carriage return -> \r */
    {'\t', 't'},  /* tab -> \t */
};

/**
 * \brief           Number of escape character mappings
 */
#define ESCAPE_CHARS_COUNT (sizeof(escape_map) / sizeof(escape_map[0]))

/**
 * \brief           Check if character needs to be escaped
 * \param[in,out]   ch: Pointer to character to check. If needs escaping, will be replaced with escape char
 * \return          `1` if character needs escaping, `0` otherwise
 */
static uint8_t
prv_char_needs_escape(char* ch) {
    for (size_t i = 0; i < ESCAPE_CHARS_COUNT; ++i) {
        if (*ch == escape_map[i][0]) {
            *ch = escape_map[i][1];
            return 1;
        }
    }
    return 0;
}

/**
 * \brief           Escape string for JSON format
 * \param[in]       input: Input string to escape. Must not be `NULL`.
 * \param[in]       input_len: Length of input string in bytes.
 * \param[out]      output: Buffer for escaped output string. Must not be `NULL`.
 * \param[in]       output_capacity: Maximum capacity of output buffer in bytes.
 * \param[out]      bytes_written: Number of bytes written to output buffer. Must not be `NULL`.
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_utils_escape_string(const char* input, size_t input_len, char* output, size_t output_capacity,
                           size_t* bytes_written) {
    /* Parameter validation */
    if (input == NULL || output == NULL || bytes_written == NULL) {
        return lwjsonERRNULL;
    }

    *bytes_written = 0;

    for (size_t i = 0; i < input_len; ++i) {
        char escape_char = input[i];
        if (prv_char_needs_escape(&escape_char)) {
            if (*bytes_written >= output_capacity) {
                return lwjsonERRBUF;
            }
            output[*bytes_written] = '\\';
            ++(*bytes_written);
        }
        if (*bytes_written >= output_capacity) {
            return lwjsonERRBUF;
        }
        output[*bytes_written] = escape_char;
        ++(*bytes_written);
    }
    return lwjsonOK;
}

/**
 * \brief           Escape string for JSON format using a streaming callback
 * \param[in]       input: Input string to escape. Must not be `NULL`.
 * \param[in]       input_len: Length of input string in bytes.
 * \param[in]       callback: Callback function for streaming output. Must not be `NULL`.
 * \param[in]       ctx: User context passed to callback. Can be `NULL`.
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_utils_escape_string_cb(const char* input, size_t input_len, lwjson_serializer_callback_fn callback, void* ctx) {
    lwjsonr_t res = lwjsonOK;

    /* Parameter validation */
    if (input == NULL || callback == NULL) {
        return lwjsonERRNULL;
    }

    for (size_t i = 0; i < input_len; ++i) {
        char escape_char = input[i];
        if (prv_char_needs_escape(&escape_char)) {
            res = callback(ctx, "\\", 1);
            if (res != lwjsonOK) {
                return res;
            }
        }
        res = callback(ctx, &escape_char, 1);
        if (res != lwjsonOK) {
            return res;
        }
    }
    return lwjsonOK;
}

/**
 * \brief           Unescape string from JSON format
 * \param[in]       input: Input string to unescape. Must not be `NULL`.
 * \param[in]       input_len: Length of input string in bytes.
 * \param[out]      output: Buffer for unescaped output string. Must not be `NULL`.
 * \param[in]       output_capacity: Maximum capacity of output buffer in bytes.
 * \param[out]      bytes_written: Number of bytes written to output buffer. Must not be `NULL`.
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_utils_unescape_string(const char* input, size_t input_len, char* output, size_t output_capacity,
                             size_t* bytes_written) {
    uint8_t escaped = 0;

    /* Parameter validation */
    if (input == NULL || output == NULL || bytes_written == NULL) {
        return lwjsonERRNULL;
    }

    /* Initialize return value */
    *bytes_written = 0;

    /* Main loop */
    for (size_t i = 0; i < input_len; ++i) {
        if (*bytes_written >= output_capacity) {
            return lwjsonERRBUF;
        }
        if (escaped) {
            for (size_t j = 0; j < ESCAPE_CHARS_COUNT; ++j) {
                if (input[i] == escape_map[j][1]) {
                    output[*bytes_written] = escape_map[j][0];
                    (*bytes_written)++;
                    escaped = 0;
                    break;
                }
            }
            if (escaped) { /*Check if character is in escape map */
                return lwjsonERRJSON;
            }
        } else {
            if (input[i] == '\\') {
                escaped = 1;
            } else {
                output[*bytes_written] = input[i];
                (*bytes_written)++;
            }
        }
    }
    return lwjsonOK;
}
