/**
 * \file            lwjson_utils.h
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
#ifndef LWJSON_UTILS_HDR_H
#define LWJSON_UTILS_HDR_H

/* Include system headers */
#include <stddef.h>
#include <stdint.h>

/* Include lwjson library */
#include "lwjson.h"
#include "lwjson_serializer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup         LWJSON
 * \{
 */

lwjsonr_t lwjson_utils_escape_string(const char* input, size_t input_len, char* output, size_t output_capacity,
                                     size_t* bytes_written);
lwjsonr_t lwjson_utils_escape_string_cb(const char* input, size_t input_len, lwjson_serializer_callback_fn callback,
                                        void* ctx);
lwjsonr_t lwjson_utils_unescape_string(const char* input, size_t input_len, char* output, size_t output_capacity,
                                       size_t* bytes_written);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif

#endif /* LWJSON_UTILS_HDR_H */
