/**
 * \file            lwjson_utils.c
 * \brief           JSON string utility functions for character escaping and unescaping
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
 * Version:         v1.10.1
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
    for (size_t idx = 0; idx < ESCAPE_CHARS_COUNT; ++idx) {
        if (*ch == escape_map[idx][0]) {
            *ch = escape_map[idx][1];
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

    for (size_t idx = 0; idx < input_len; ++idx) {
        char escape_char = input[idx];
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

    for (size_t idx = 0; idx < input_len; ++idx) {
        char escape_char = input[idx];
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
 * \brief           Get value of a hexadecimal character
 * \param[in]       chr: Character to convert
 * \return          Value between `0` and `15`, or `-1` if not a hexadecimal character
 */
static int
prv_hex_val(char chr) {
    if (chr >= '0' && chr <= '9') {
        return chr - '0';
    } else if (chr >= 'a' && chr <= 'f') {
        return chr - 'a' + 10;
    } else if (chr >= 'A' && chr <= 'F') {
        return chr - 'A' + 10;
    }
    return -1;
}

/**
 * \brief           Parse exactly `4` hexadecimal characters
 * \param[in]       input: Input with at least `4` characters
 * \param[out]      value: Parsed value
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_parse_hex4(const char* input, uint32_t* value) {
    *value = 0;
    for (size_t idx = 0; idx < 4; ++idx) {
        int val = prv_hex_val(input[idx]);
        if (val < 0) {
            return 0;
        }
        *value = (*value << 4) | (uint32_t)val;
    }
    return 1;
}

/**
 * \brief           Encode unicode code point to UTF-8
 * \param[in]       code_point: Unicode code point to encode
 * \param[out]      output: Output buffer with at least `4` bytes of space
 * \return          Number of bytes written, `0` if code point is out of range
 */
uint8_t
lwjson_utils_utf8_encode(uint32_t code_point, char* output) {
    if (code_point < 0x80) {
        output[0] = (char)code_point;
        return 1;
    } else if (code_point < 0x800) {
        output[0] = (char)(0xC0 | (code_point >> 6));
        output[1] = (char)(0x80 | (code_point & 0x3F));
        return 2;
    } else if (code_point < 0x10000) {
        output[0] = (char)(0xE0 | (code_point >> 12));
        output[1] = (char)(0x80 | ((code_point >> 6) & 0x3F));
        output[2] = (char)(0x80 | (code_point & 0x3F));
        return 3;
    } else if (code_point <= 0x10FFFF) {
        output[0] = (char)(0xF0 | (code_point >> 18));
        output[1] = (char)(0x80 | ((code_point >> 12) & 0x3F));
        output[2] = (char)(0x80 | ((code_point >> 6) & 0x3F));
        output[3] = (char)(0x80 | (code_point & 0x3F));
        return 4;
    }
    return 0;
}

/**
 * \brief           Unescape single JSON escape sequence
 *
 * Supports simple escapes (`\"`, `\\`, `\/`, `\b`, `\f`, `\n`, `\r`, `\t`)
 * and unicode escapes `\uXXXX`, including surrogate pairs `😀`.
 * Unicode escapes are decoded to UTF-8.
 *
 * \param[in]       input: Input that starts with backslash character. Must not be `NULL`.
 * \param[in]       input_len: Number of available input characters
 * \param[out]      output: Output buffer with at least `4` bytes of space. Must not be `NULL`.
 * \param[out]      consumed: Number of input characters consumed by the sequence. Must not be `NULL`.
 * \param[out]      written: Number of bytes written to output. Must not be `NULL`.
 * \return          \ref lwjsonOK on success, \ref lwjsonERRJSON for invalid or incomplete escape sequence,
 *                  member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_utils_unescape_seq(const char* input, size_t input_len, char* output, size_t* consumed, size_t* written) {
    /* Parameter validation */
    if (input == NULL || output == NULL || consumed == NULL || written == NULL) {
        return lwjsonERRNULL;
    }
    if (input_len < 2 || input[0] != '\\') {
        return lwjsonERRJSON;
    }

    /* Unicode escape sequence */
    if (input[1] == 'u') {
        uint32_t code_point, low_surrogate;

        if (input_len < 6 || !prv_parse_hex4(&input[2], &code_point)) {
            return lwjsonERRJSON;
        }
        *consumed = 6;

        /* High surrogate must be followed by low surrogate escape sequence */
        if (code_point >= 0xD800 && code_point <= 0xDBFF) {
            if (input_len < 12 || input[6] != '\\' || input[7] != 'u' || !prv_parse_hex4(&input[8], &low_surrogate)
                || low_surrogate < 0xDC00 || low_surrogate > 0xDFFF) {
                return lwjsonERRJSON;
            }
            code_point = 0x10000 + ((code_point - 0xD800) << 10) + (low_surrogate - 0xDC00);
            *consumed = 12;
        } else if (code_point >= 0xDC00 && code_point <= 0xDFFF) {
            /* Low surrogate without high one */
            return lwjsonERRJSON;
        }
        *written = lwjson_utils_utf8_encode(code_point, output);
        return lwjsonOK;
    }

    /* Simple escape sequence */
    for (size_t idx = 0; idx < ESCAPE_CHARS_COUNT; ++idx) {
        if (input[1] == escape_map[idx][1]) {
            output[0] = escape_map[idx][0];
            *consumed = 2;
            *written = 1;
            return lwjsonOK;
        }
    }
    return lwjsonERRJSON;
}

/**
 * \brief           Unescape string from JSON format
 * \note            Unicode escape sequences are decoded to UTF-8
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
    lwjsonr_t res;
    char seq_out[4];
    size_t consumed, written;

    /* Parameter validation */
    if (input == NULL || output == NULL || bytes_written == NULL) {
        return lwjsonERRNULL;
    }

    /* Initialize return value */
    *bytes_written = 0;

    /* Main loop */
    for (size_t idx = 0; idx < input_len; idx += consumed) {
        if (input[idx] == '\\') {
            res = lwjson_utils_unescape_seq(&input[idx], input_len - idx, seq_out, &consumed, &written);
            if (res != lwjsonOK) {
                return res;
            }
        } else {
            seq_out[0] = input[idx];
            consumed = 1;
            written = 1;
        }
        if (written > (output_capacity - *bytes_written)) {
            return lwjsonERRBUF;
        }
        LWJSON_MEMCPY(&output[*bytes_written], seq_out, written);
        *bytes_written += written;
    }
    return lwjsonOK;
}
