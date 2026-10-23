/**
 * \file            lwjson.c
 * \brief           Lightweight JSON format parser
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
#include <string.h>
#include "lwjson/lwjson.h"
#include "lwjson/lwjson_utils.h"

/**
 * \brief           Internal string object
 */
typedef struct {
    const char* const start; /*!< Original pointer to beginning of JSON object */
    const char* const end;   /*!< Pointer to one past the last character of input json string */
    const char* cptr;        /*!< Current char pointer */
} lwjson_int_str_t;

/**
 * \brief           Calculate end pointer of input data
 * \param[in]       data: Input data
 * \param[in]       len: Input data length
 * \return          Pointer to one past the last character,
 *                  or `NULL` if `data` is `NULL` or `data + len` would overflow
 */
static inline const char*
prv_get_end_ptr(const void* data, size_t len) {
    if (data == NULL || len > (UINTPTR_MAX - (uintptr_t)data)) {
        return NULL;
    }
    return (const char*)data + len;
}

/**
 * \brief           Get current character without advancing
 * \param[in]       pobj: Pointer to text
 * \return          Current character, or `'\0'` when at end of input
 */
static inline char
prv_peek(const lwjson_int_str_t* pobj) {
    return pobj->cptr < pobj->end ? *pobj->cptr : '\0';
}

/**
 * \brief           Check if at least `cnt` characters remain in the input
 * \param[in]       pobj: Pointer to text
 * \param[in]       cnt: Number of characters required
 * \return          `1` if available, `0` otherwise
 */
static inline uint8_t
prv_has(const lwjson_int_str_t* pobj, size_t cnt) {
    return (size_t)(pobj->end - pobj->cptr) >= cnt;
}

/**
 * \brief           Check if character is decimal digit
 * \param[in]       chr: Character to check
 * \return          `1` if digit, `0` otherwise
 */
static inline uint8_t
prv_is_digit(char chr) {
    return chr >= '0' && chr <= '9';
}

/**
 * \brief           Allocate new token for JSON block
 * \param[in]       lwobj: LwJSON instance
 * \return          Pointer to new token
 */
static lwjson_token_t*
prv_alloc_token(lwjson_t* lwobj) {
    if (lwobj->next_free_token_pos < lwobj->tokens_len) {
        LWJSON_MEMSET(&lwobj->tokens[lwobj->next_free_token_pos], 0x00, sizeof(*lwobj->tokens));
        return &lwobj->tokens[lwobj->next_free_token_pos++];
    }
    return NULL;
}

/**
 * \brief           Skip all characters that are considered *blank* as per RFC4627
 * \param[in,out]   pobj: Pointer to text that is modified on success
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_skip_blank(lwjson_int_str_t* pobj) {
    while (prv_peek(pobj) != '\0') {
        if (*pobj->cptr == ' ' || *pobj->cptr == '\t' || *pobj->cptr == '\r' || *pobj->cptr == '\n') {
            ++pobj->cptr;
#if LWJSON_CFG_COMMENTS
            /* Check for comments and remove them */
        } else if (*pobj->cptr == '/' && prv_has(pobj, 2) && pobj->cptr[1] == '*') {
            pobj->cptr += 2;
            while (prv_peek(pobj) != '\0') {
                if (*pobj->cptr == '*') {
                    ++pobj->cptr;
                    if (prv_peek(pobj) == '/') {
                        ++pobj->cptr;
                        break;
                    }
                }
                ++pobj->cptr;
            }
#endif /* LWJSON_CFG_COMMENTS */
        } else {
            break;
        }
    }
    if (prv_peek(pobj) != '\0') {
        return lwjsonOK;
    }
    return lwjsonERRJSON;
}

/**
 * \brief           Parse JSON string that must start end end with double quotes `"` character
 * It validates escape sequences and parses length of characters, but does not perform any decode operation
 * \param[in,out]   pobj: Pointer to text that is modified on success
 * \param[out]      pout: Pointer to pointer to string that is set where string starts
 * \param[out]      poutlen: Length of string in units of characters is stored here
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_parse_string(lwjson_int_str_t* pobj, const char** pout, size_t* poutlen) {
    lwjsonr_t res;
    size_t len = 0;

    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    if (prv_peek(pobj) != '"') {
        return lwjsonERRJSON;
    }
    ++pobj->cptr;
    *pout = pobj->cptr;
    /* Parse string but take care of escape characters */
    for (;; ++pobj->cptr, ++len) {
        if (prv_peek(pobj) == '\0') {
            return lwjsonERRJSON;
        }
        /* Check special characters */
        if (*pobj->cptr == '\\') {
            char seq_out[4];
            size_t consumed, written;

            /* Validate escape sequence, string is not decoded here */
            if (lwjson_utils_unescape_seq(pobj->cptr, (size_t)(pobj->end - pobj->cptr), seq_out, &consumed, &written)
                != lwjsonOK) {
                return lwjsonERRJSON;
            }
            /* Loop increments for the last character of the sequence */
            pobj->cptr += consumed - 1;
            len += consumed - 1;
        } else if (*pobj->cptr == '"') {
            ++pobj->cptr;
            break;
        }
    }
    *poutlen = len;
    return res;
}

/**
 * \brief           Parse property name that must comply with JSON string format as in RFC4627
 * Property string must be followed by colon character ":"
 * \param[in,out]   pobj: Pointer to text that is modified on success
 * \param[out]      tkn: Token instance to write property name to
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_parse_property_name(lwjson_int_str_t* pobj, lwjson_token_t* tkn) {
    lwjsonr_t res;

    /* Parse property string first */
    res = prv_parse_string(pobj, &tkn->token_name, &tkn->token_name_len);
    if (res != lwjsonOK) {
        return res;
    }
    /* Skip any spaces */
    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    /* Must continue with colon */
    if (prv_peek(pobj) != ':') {
        return lwjsonERRJSON;
    }
    ++pobj->cptr;
    /* Skip any spaces */
    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    return lwjsonOK;
}

/**
 * \brief           Calculate new value for exponent 10^exponent.
 * \param[in]       exp: Exponent.
 * \param[in]       is_minus: Indicates whether the exponent is negative.
 * \return          Value of 10^exponent.
 */
static lwjson_real_t
prv_power10(lwjson_int_t exp, uint8_t is_minus) {
    lwjson_real_t result = (lwjson_real_t)1;
    lwjson_real_t base = (lwjson_real_t)10;

    for (; exp > 0; base *= base, exp >>= 1) {
        if ((exp & 1) == 1) {
            result *= base;
        }
    }

    return is_minus ? ((lwjson_real_t)1 / result) : result;
}

/**
 * \brief           Parse number as described in RFC4627
 * \param[in,out]   pobj: Pointer to text that is modified on success
 * \param[out]      tout: Pointer to output number format
 * \param[out]      fout: Pointer to output real-type variable. Used if type is REAL.
 * \param[out]      iout: Pointer to output int-type variable. Used if type is INT.
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_parse_number(lwjson_int_str_t* pobj, lwjson_type_t* tout, lwjson_real_t* fout, lwjson_int_t* iout) {
    lwjsonr_t res;
    uint8_t is_minus;
    lwjson_real_t real_num = 0;
    lwjson_int_t int_num = 0;
    lwjson_type_t type = LWJSON_TYPE_NUM_INT;

    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    if (prv_peek(pobj) == '\0') {
        return lwjsonERRJSON;
    }
    is_minus = *pobj->cptr == '-' ? (++pobj->cptr, 1) : 0;
    if (!prv_is_digit(prv_peek(pobj)) /* End of input or character outside number range */
        || (*pobj->cptr == '0' && prv_has(pobj, 2) && prv_is_digit(pobj->cptr[1]))) { /* Number starts with 0 but not
                                                                                         followed by dot */
        return lwjsonERRJSON;
    }

    /* Parse number */
    for (int_num = 0; prv_is_digit(prv_peek(pobj)); ++pobj->cptr) {
        int_num = int_num * (lwjson_int_t)10 + (*pobj->cptr - '0');
    }

    real_num = (lwjson_real_t)int_num;

    if (prv_peek(pobj) == '.') { /* Number has exponent */
        lwjson_real_t exp;
        lwjson_int_t dec_num;

        real_num = (lwjson_real_t)int_num;

        type = LWJSON_TYPE_NUM_REAL;         /* Format is real */
        ++pobj->cptr;                        /* Ignore comma character */
        if (!prv_is_digit(prv_peek(pobj))) { /* Must be followed by number characters */
            return lwjsonERRJSON;
        }

        /* Get number after decimal point */
        for (exp = (lwjson_real_t)1, dec_num = 0; prv_is_digit(prv_peek(pobj));
             ++pobj->cptr, exp *= (lwjson_real_t)10) {
            dec_num = dec_num * (lwjson_int_t)10 + (lwjson_int_t)(*pobj->cptr - '0');
        }

        /* Add decimal part to number */
        real_num += (lwjson_real_t)dec_num / exp;
    }
    if (prv_peek(pobj) == 'e' || prv_peek(pobj) == 'E') { /* Engineering mode */
        uint8_t is_minus_exp;
        lwjson_int_t exp_cnt;

        type = LWJSON_TYPE_NUM_REAL; /* Format is real */
        ++pobj->cptr;                /* Ignore enginnering sing part */
        if (prv_peek(pobj) == '\0') {
            return lwjsonERRJSON;
        }
        is_minus_exp = *pobj->cptr == '-' ? (++pobj->cptr, 1) : 0; /* Check if negative */
        if (prv_peek(pobj) == '+') {                               /* Optional '+' is possible too */
            ++pobj->cptr;
        }
        if (!prv_is_digit(prv_peek(pobj))) { /* Must be followed by number characters */
            return lwjsonERRJSON;
        }

        /* Parse exponent number */
        for (exp_cnt = 0; prv_is_digit(prv_peek(pobj)); ++pobj->cptr) {
            exp_cnt = exp_cnt * (lwjson_int_t)10 + (lwjson_int_t)(*pobj->cptr - '0');
        }

        /* Calculate new value for exponent 10^exponent */
        real_num *= prv_power10(exp_cnt, is_minus_exp);
    }

    /* Write output values */
    if (tout != NULL) {
        *tout = type;
    }
    if (type == LWJSON_TYPE_NUM_INT) {
        *iout = is_minus ? -int_num : int_num;
    } else {
        *fout = is_minus ? -real_num : real_num;
    }
    return lwjsonOK;
}

/**
 * \brief           Compare escaped JSON string with unescaped user string
 * \param[in]       escaped: Escaped string, as it appears in JSON input
 * \param[in]       escaped_len: Length of escaped string
 * \param[in]       str: Unescaped string to compare with
 * \param[in]       str_len: Length of unescaped string
 * \return          `1` if strings match after unescaping, `0` otherwise
 */
static uint8_t
prv_escaped_string_equal(const char* escaped, size_t escaped_len, const char* str, size_t str_len) {
    char seq_out[4];
    size_t consumed, written;

    for (size_t idx = 0; idx < escaped_len; idx += consumed) {
        if (escaped[idx] == '\\') {
            if (lwjson_utils_unescape_seq(&escaped[idx], escaped_len - idx, seq_out, &consumed, &written)
                != lwjsonOK) {
                return 0;
            }
        } else {
            seq_out[0] = escaped[idx];
            consumed = 1;
            written = 1;
        }
        if (written > str_len || memcmp(seq_out, str, written) != 0) {
            return 0;
        }
        str += written;
        str_len -= written;
    }
    return str_len == 0;
}

/**
 * \brief           Create path segment from input path for search operation
 * \param[in,out]   ppath: Pointer to pointer to input path. Pointer is modified
 * \param[out]      opath: Pointer to pointer to write path segment
 * \param[out]      olen: Pointer to variable to write length of segment
 * \param[out]      is_last: Pointer to write if this is last segment
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_create_path_segment(const char** ppath, const char** opath, size_t* olen, uint8_t* is_last) {
    const char* segment = *ppath;

    *is_last = 0;
    *opath = NULL;
    *olen = 0;

    /* Check input path */
    if (segment == NULL || *segment == '\0') {
        *is_last = 1;
        return 0;
    }

    /*
     * Path must be one of:
     * - literal text
     * - "#" followed by dot "."
     */
    if (*segment == '#') {
        *opath = segment;
        for (*olen = 0;; ++segment, ++(*olen)) {
            if (*segment == '.') {
                ++segment;
                break;
            } else if (*segment == '\0') {
                if (*olen == 1) {
                    return 0;
                } else {
                    break;
                }
            }
        }
        *ppath = segment;
    } else {
        *opath = segment;
        for (*olen = 0; *segment != '\0' && *segment != '.'; ++(*olen), ++segment) {}
        *ppath = segment + 1;
    }
    if (*segment == '\0') {
        *is_last = 1;
    }
    return 1;
}

/**
 * \brief           Input recursive function for find operation
 * \param[in]       parent: Parent token of type \ref LWJSON_TYPE_ARRAY or LWJSON_TYPE_OBJECT
 * \param[in]       path: Path to search for starting this token further
 * \return          Found token on success, `NULL` otherwise
 */
static const lwjson_token_t*
prv_find(const lwjson_token_t* parent, const char* path) {
    const char* segment;
    size_t segment_len;
    uint8_t is_last, res;

    /* Get path segments */
    res = prv_create_path_segment(&path, &segment, &segment_len, &is_last);
    if (res != 0) {
        /* Check if detected an array request */
        if (*segment == '#') {
            /* Parent must be array */
            if (parent->type != LWJSON_TYPE_ARRAY) {
                return NULL;
            }

            /* Check if index requested */
            if (segment_len > 1) {
                const lwjson_token_t* tkn;
                size_t index = 0;

                /* Parse number */
                for (size_t idx = 1; idx < segment_len; ++idx) {
                    if (!prv_is_digit(segment[idx])) {
                        return NULL;
                    } else {
                        index = index * 10 + (segment[idx] - '0');
                    }
                }

                /* Start from beginning */
                for (tkn = parent->u.first_child; tkn != NULL && index > 0; tkn = tkn->next, --index) {}
                if (tkn != NULL) {
                    if (is_last) {
                        return tkn;
                    } else {
                        return prv_find(tkn, path);
                    }
                }
                return NULL;
            }

            /* Scan all indexes and get first match */
            for (const lwjson_token_t* tkn = parent->u.first_child; tkn != NULL; tkn = tkn->next) {
                const lwjson_token_t* tmp = prv_find(tkn, path);
                if (tmp != NULL) {
                    return tmp;
                }
            }
        } else {
            if (parent->type != LWJSON_TYPE_OBJECT) {
                return NULL;
            }
            for (const lwjson_token_t* tkn = parent->u.first_child; tkn != NULL; tkn = tkn->next) {
                if (prv_escaped_string_equal(tkn->token_name, tkn->token_name_len, segment, segment_len)) {
                    const lwjson_token_t* tmp;
                    if (is_last) {
                        return tkn;
                    }
                    tmp = prv_find(tkn, path);
                    if (tmp != NULL) {
                        return tmp;
                    }
                }
            }
        }
    }
    return NULL;
}

/**
 * \brief           Check for character after opening bracket of array or object
 * \param[in,out]   pobj: JSON string
 * \param[in]       tkn: Token to check for type
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static inline lwjsonr_t
prv_check_valid_char_after_open_bracket(lwjson_int_str_t* pobj, lwjson_token_t* tkn) {
    lwjsonr_t res;
    char chr;

    /* Check next character after object open */
    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    chr = prv_peek(pobj);
    if (chr == '\0' || (tkn->type == LWJSON_TYPE_OBJECT && (chr != '"' && chr != '}'))
        || (tkn->type == LWJSON_TYPE_ARRAY
            && (chr != '"' && chr != ']' && chr != '[' && chr != '{' && chr != '-' && !prv_is_digit(chr) && chr != 't'
                && chr != 'n' && chr != 'f'))) {
        res = lwjsonERRJSON;
    }
    return res;
}

/**
 * \brief           Setup LwJSON instance for parsing JSON strings
 * \param[in,out]   lwobj: LwJSON instance
 * \param[in]       tokens: Pointer to array of tokens used for parsing
 * \param[in]       tokens_len: Number of tokens
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_init(lwjson_t* lwobj, lwjson_token_t* tokens, size_t tokens_len) {
    LWJSON_MEMSET(lwobj, 0x00, sizeof(*lwobj));
    LWJSON_MEMSET(tokens, 0x00, sizeof(*tokens) * tokens_len);
    lwobj->tokens = tokens;
    lwobj->tokens_len = tokens_len;
    lwobj->first_token.type = LWJSON_TYPE_OBJECT;
    return lwjsonOK;
}

/**
 * \brief           Parse JSON data with length parameter
 * JSON format must be complete and must comply with RFC4627
 * \param[in,out]   lwobj: LwJSON instance
 * \param[in]       json_data: JSON string to parse
 * \param[in]       json_len: JSON data length
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_parse_ex(lwjson_t* lwobj, const void* json_data, size_t json_len) {
    lwjsonr_t res = lwjsonOK;
    lwjson_token_t *tkn, *tkn_obj;
    lwjson_int_str_t pobj = {
        .start = json_data,
        .end = prv_get_end_ptr(json_data, json_len),
        .cptr = json_data,
    };

    /* Check input parameters, end is NULL when data is NULL or length overflows */
    if (lwobj == NULL || pobj.end == NULL || json_len == 0) {
        res = lwjsonERRPAR;
        goto ret;
    }

    /* set first token */
    tkn_obj = &lwobj->first_token;

    /* values from very beginning */
    lwobj->flags.parsed = 0;
    lwobj->next_free_token_pos = 0;
    LWJSON_MEMSET(tkn_obj, 0x00, sizeof(*tkn_obj));

    /* First parse */
    res = prv_skip_blank(&pobj);
    if (res != lwjsonOK) {
        goto ret;
    }
    if (prv_peek(&pobj) == '\0') {
        res = lwjsonERRJSON;
        goto ret;
    }
    if (*pobj.cptr == '{') {
        tkn_obj->type = LWJSON_TYPE_OBJECT;
    } else if (*pobj.cptr == '[') {
        tkn_obj->type = LWJSON_TYPE_ARRAY;
    } else {
        res = lwjsonERRJSON;
        goto ret;
    }
    ++pobj.cptr;
    res = prv_check_valid_char_after_open_bracket(&pobj, tkn_obj);
    if (res != lwjsonOK) {
        goto ret;
    }

    /* Process all characters as indicated by input user */
    while (prv_peek(&pobj) != '\0') {
        /* Filter out blanks */
        res = prv_skip_blank(&pobj);
        if (res != lwjsonOK) {
            goto ret;
        }
        if (*pobj.cptr == ',') {
            ++pobj.cptr;
            continue;
        }

        /* Check if end of an object or an array */
        if (*pobj.cptr == (tkn_obj->type == LWJSON_TYPE_OBJECT ? '}' : ']')) {
            lwjson_token_t* parent = tkn_obj->next;
            tkn_obj->next = NULL;
            ++pobj.cptr;

            /* End of string if tkn_obj == NULL (no parent), check if properly terminated */
            tkn_obj = parent;
            if (tkn_obj == NULL) {
                prv_skip_blank(&pobj);
                res = prv_peek(&pobj) == '\0' ? lwjsonOK : lwjsonERR;
                goto ret;
            }
            continue;
        }

        /* Allocate new token */
        tkn = prv_alloc_token(lwobj);
        if (tkn == NULL) {
            res = lwjsonERRMEM;
            goto ret;
        }

        /* If object type is not array, first thing is property that starts with quotes */
        if (tkn_obj->type != LWJSON_TYPE_ARRAY) {
            if (*pobj.cptr != '"') {
                res = lwjsonERRJSON;
                goto ret;
            }
            res = prv_parse_property_name(&pobj, tkn);
            if (res != lwjsonOK) {
                goto ret;
            }
        }

        /* Add element to linked list */
        if (tkn_obj->u.first_child == NULL) {
            tkn_obj->u.first_child = tkn;
        } else {
            lwjson_token_t* tkn_child;
            for (tkn_child = tkn_obj->u.first_child; tkn_child->next != NULL; tkn_child = tkn_child->next) {}
            tkn_child->next = tkn;
        }

        /* Check next character to process */
        switch (*pobj.cptr) {
            case '{':
            case '[':
                tkn->type = *pobj.cptr == '{' ? LWJSON_TYPE_OBJECT : LWJSON_TYPE_ARRAY;
                ++pobj.cptr;

                res = prv_check_valid_char_after_open_bracket(&pobj, tkn);
                if (res != lwjsonOK) {
                    goto ret;
                }
                tkn->next = tkn_obj; /* Temporary saved as parent object */
                tkn_obj = tkn;
                break;
            case '"':
                res = prv_parse_string(&pobj, &tkn->u.str.token_value, &tkn->u.str.token_value_len);
                if (res == lwjsonOK) {
                    tkn->type = LWJSON_TYPE_STRING;
                } else {
                    goto ret;
                }
                break;
            case 't':
                /* RFC4627 is lower-case only */
                if (prv_has(&pobj, 4) && strncmp(pobj.cptr, "true", 4) == 0) {
                    tkn->type = LWJSON_TYPE_TRUE;
                    pobj.cptr += 4;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            case 'f':
                /* RFC4627 is lower-case only */
                if (prv_has(&pobj, 5) && strncmp(pobj.cptr, "false", 5) == 0) {
                    tkn->type = LWJSON_TYPE_FALSE;
                    pobj.cptr += 5;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            case 'n':
                /* RFC4627 is lower-case only */
                if (prv_has(&pobj, 4) && strncmp(pobj.cptr, "null", 4) == 0) {
                    tkn->type = LWJSON_TYPE_NULL;
                    pobj.cptr += 4;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            default:
                if (*pobj.cptr == '-' || prv_is_digit(*pobj.cptr)) {
                    if (prv_parse_number(&pobj, &tkn->type, &tkn->u.num_real, &tkn->u.num_int) != lwjsonOK) {
                        res = lwjsonERRJSON;
                        goto ret;
                    }
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
        }

        /* Below code is used to check characters after valid tokens */
        if (tkn->type == LWJSON_TYPE_ARRAY || tkn->type == LWJSON_TYPE_OBJECT) {
            continue;
        }

        /*
         * Check what are values after the token value
         *
         * As per RFC4627, every token value may have one or more
         * blank characters, followed by one of below options:
         *  - Comma separator for next token
         *  - End of array indication
         *  - End of object indication
         */
        res = prv_skip_blank(&pobj);
        if (res != lwjsonOK) {
            goto ret;
        }
        /* Check if valid string is availabe after */
        if (prv_peek(&pobj) == '\0' || (*pobj.cptr != ',' && *pobj.cptr != ']' && *pobj.cptr != '}')) {
            res = lwjsonERRJSON;
            goto ret;
        } else if (*pobj.cptr == ',') { /* Check to advance to next token immediately */
            ++pobj.cptr;
        }
    }
    /*
     * Successful parse returns immediately when root object or array gets closed.
     * Reaching end of input here means at least root (`tkn_obj` stack) is still open.
     */
    res = lwjsonERRJSON;
ret:
    if (res == lwjsonOK) {
        lwobj->flags.parsed = 1;
    }
    return res;
}

/**
 * \brief           Parse input JSON format
 * JSON format must be complete and must comply with RFC4627
 * \param[in,out]   lwobj: LwJSON instance
 * \param[in]       json_str: JSON string to parse
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_parse(lwjson_t* lwobj, const char* json_str) {
    return lwjson_parse_ex(lwobj, json_str, strlen(json_str));
}

/**
 * \brief           Free token instances (specially used in case of dynamic memory allocation)
 * \param[in,out]   lwobj: LwJSON instance
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_free(lwjson_t* lwobj) {
    LWJSON_MEMSET(lwobj->tokens, 0x00, sizeof(*lwobj->tokens) * lwobj->tokens_len);
    lwobj->flags.parsed = 0;
    return lwjsonOK;
}

/**
 * \brief           Find first match in the given path for JSON entry
 * JSON must be valid and parsed with \ref lwjson_parse function
 * \param[in]       lwobj: JSON instance with parsed JSON string
 * \param[in]       path: Path with dot-separated entries to search for the JSON key to return
 * \return          Pointer to found token on success, `NULL` if token cannot be found
 */
const lwjson_token_t*
lwjson_find(lwjson_t* lwobj, const char* path) {
    if (lwobj == NULL || !lwobj->flags.parsed || path == NULL) {
        return NULL;
    }
    return prv_find(lwjson_get_first_token(lwobj), path);
}

/**
 * \brief           Find first match in the given path for JSON path
 * JSON must be valid and parsed with \ref lwjson_parse function
 *
 * \param[in]       lwobj: JSON instance with parsed JSON string
 * \param[in]       token: Root token to start search at.
 *                      Token must be type \ref LWJSON_TYPE_OBJECT or \ref LWJSON_TYPE_ARRAY.
 *                      Set to `NULL` to use root token of LwJSON object
 * \param[in]       path: path with dot-separated entries to search for JSON key
 * \return          Pointer to found token on success, `NULL` if token cannot be found
 */
const lwjson_token_t*
lwjson_find_ex(lwjson_t* lwobj, const lwjson_token_t* token, const char* path) {
    if (lwobj == NULL || !lwobj->flags.parsed || path == NULL) {
        return NULL;
    }
    if (token == NULL) {
        token = lwjson_get_first_token(lwobj);
    }
    if (token == NULL || (token->type != LWJSON_TYPE_ARRAY && token->type != LWJSON_TYPE_OBJECT)) {
        return NULL;
    }
    return prv_find(token, path);
}
