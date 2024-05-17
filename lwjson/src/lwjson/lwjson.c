/**
 * \file            lwjson.c
 * \brief           Lightweight JSON format parser
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
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
 * Version:         v1.7.0
 */
#include <string.h>
#include "lwjson/lwjson.h"

/**
 * \brief           Internal string object
 */
typedef struct {
    const char* start; /*!< Original pointer to beginning of JSON object */
    size_t len;        /*!< Total length of input json string */
    const char* p;     /*!< Current char pointer */
} lwjson_int_str_t;

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
    while (pobj->p != NULL && *pobj->p != '\0' && (size_t)(pobj->p - pobj->start) < pobj->len) {
        if (*pobj->p == ' ' || *pobj->p == '\t' || *pobj->p == '\r' || *pobj->p == '\n' || *pobj->p == '\f') {
            ++pobj->p;
#if LWJSON_CFG_COMMENTS
            /* Check for comments and remove them */
        } else if (*pobj->p == '/') {
            ++pobj->p;
            if (pobj->p != NULL && *pobj->p == '*') {
                ++pobj->p;
                while (pobj->p != NULL && *pobj->p != '\0' && (size_t)(pobj->p - pobj->start) < pobj->len) {
                    if (*pobj->p == '*') {
                        ++pobj->p;
                        if (*pobj->p == '/') {
                            ++pobj->p;
                            break;
                        }
                    }
                    ++pobj->p;
                }
            }
#endif /* LWJSON_CFG_COMMENTS */
        } else {
            break;
        }
    }
    if (pobj->p != NULL && *pobj->p != '\0' && (size_t)(pobj->p - pobj->start) < pobj->len) {
        return lwjsonOK;
    }
    return lwjsonERRJSON;
}

/**
 * \brief           Parse JSON string that must start end end with double quotes `"` character
 * It just parses length of characters and does not perform any decode operation
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
    if (*pobj->p++ != '"') {
        return lwjsonERRJSON;
    }
    *pout = pobj->p;
    /* Parse string but take care of escape characters */
    for (;; ++pobj->p, ++len) {
        if (pobj->p == NULL || *pobj->p == '\0' || (size_t)(pobj->p - pobj->start) >= pobj->len) {
            return lwjsonERRJSON;
        }
        /* Check special characters */
        if (*pobj->p == '\\') {
            ++pobj->p;
            ++len;
            switch (*pobj->p) {
                case '"':  /* fallthrough */
                case '\\': /* fallthrough */
                case '/':  /* fallthrough */
                case 'b':  /* fallthrough */
                case 'f':  /* fallthrough */
                case 'n':  /* fallthrough */
                case 'r':  /* fallthrough */
                case 't': break;
                case 'u':
                    ++pobj->p;
                    for (size_t i = 0; i < 4; ++i, ++len) {
                        if (!((*pobj->p >= '0' && *pobj->p <= '9') || (*pobj->p >= 'a' && *pobj->p <= 'f')
                              || (*pobj->p >= 'A' && *pobj->p <= 'F'))) {
                            return lwjsonERRJSON;
                        }
                        if (i < 3) {
                            ++pobj->p;
                        }
                    }
                    break;
                default: return lwjsonERRJSON;
            }
        } else if (*pobj->p == '"') {
            ++pobj->p;
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
 * \param[out]      t: Token instance to write property name to
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_parse_property_name(lwjson_int_str_t* pobj, lwjson_token_t* t) {
    lwjsonr_t res;

    /* Parse property string first */
    res = prv_parse_string(pobj, &t->token_name, &t->token_name_len);
    if (res != lwjsonOK) {
        return res;
    }
    /* Skip any spaces */
    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    /* Must continue with colon */
    if (*pobj->p++ != ':') {
        return lwjsonERRJSON;
    }
    /* Skip any spaces */
    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    return lwjsonOK;
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
    if (*pobj->p == '\0' || (size_t)(pobj->p - pobj->start) >= pobj->len) {
        return lwjsonERRJSON;
    }
    is_minus = *pobj->p == '-' ? (++pobj->p, 1) : 0;
    if (*pobj->p == '\0'                    /* Invalid string */
        || *pobj->p < '0' || *pobj->p > '9' /* Character outside number range */
        || (*pobj->p == '0'
            && (pobj->p[1] < '0' && pobj->p[1] > '9'))) { /* Number starts with 0 but not followed by dot */
        return lwjsonERRJSON;
    }

    /* Parse number */
    for (int_num = 0; *pobj->p >= '0' && *pobj->p <= '9'; ++pobj->p) {
        int_num = int_num * (lwjson_int_t)10 + (*pobj->p - '0');
    }

    real_num = (lwjson_real_t)int_num;

    if (pobj->p != NULL && *pobj->p == '.') { /* Number has exponent */
        lwjson_real_t exp;
        lwjson_int_t dec_num;

        real_num = (lwjson_real_t)int_num;

        type = LWJSON_TYPE_NUM_REAL;            /* Format is real */
        ++pobj->p;                              /* Ignore comma character */
        if (*pobj->p < '0' || *pobj->p > '9') { /* Must be followed by number characters */
            return lwjsonERRJSON;
        }

        /* Get number after decimal point */
        for (exp = (lwjson_real_t)1, dec_num = 0; *pobj->p >= '0' && *pobj->p <= '9';
             ++pobj->p, exp *= (lwjson_real_t)10) {
            dec_num = dec_num * (lwjson_int_t)10 + (lwjson_int_t)(*pobj->p - '0');
        }

        /* Add decimal part to number */
        real_num += (lwjson_real_t)dec_num / exp;
    }
    if (pobj->p != NULL && (*pobj->p == 'e' || *pobj->p == 'E')) { /* Engineering mode */
        uint8_t is_minus_exp;
        lwjson_int_t exp_cnt;

        type = LWJSON_TYPE_NUM_REAL;                         /* Format is real */
        ++pobj->p;                                           /* Ignore enginnering sing part */
        is_minus_exp = *pobj->p == '-' ? (++pobj->p, 1) : 0; /* Check if negative */
        if (*pobj->p == '+') {                               /* Optional '+' is possible too */
            ++pobj->p;
        }
        if (*pobj->p < '0' || *pobj->p > '9') { /* Must be followed by number characters */
            return lwjsonERRJSON;
        }

        /* Parse exponent number */
        for (exp_cnt = 0; *pobj->p >= '0' && *pobj->p <= '9'; ++pobj->p) {
            exp_cnt = exp_cnt * (lwjson_int_t)10 + (lwjson_int_t)(*pobj->p - '0');
        }

        /* Calculate new value for exponent 10^exponent */
        /* TODO: We could change this to lookup tables... */
        if (is_minus_exp) {
            for (; exp_cnt > 0; real_num /= (lwjson_real_t)10, --exp_cnt) {}
        } else {
            for (; exp_cnt > 0; real_num *= (lwjson_real_t)10, --exp_cnt) {}
        }
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
                for (size_t i = 1; i < segment_len; ++i) {
                    if (segment[i] < '0' || segment[i] > '9') {
                        return NULL;
                    } else {
                        index = index * 10 + (segment[i] - '0');
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
                if (tkn->token_name_len == segment_len && !strncmp(tkn->token_name, segment, segment_len)) {
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
 * \param[in]       t: Token to check for type
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static inline lwjsonr_t
prv_check_valid_char_after_open_bracket(lwjson_int_str_t* pobj, lwjson_token_t* t) {
    lwjsonr_t res;

    /* Check next character after object open */
    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    if (*pobj->p == '\0' || (t->type == LWJSON_TYPE_OBJECT && (*pobj->p != '"' && *pobj->p != '}'))
        || (t->type == LWJSON_TYPE_ARRAY
            && (*pobj->p != '"' && *pobj->p != ']' && *pobj->p != '[' && *pobj->p != '{' && *pobj->p != '-'
                && (*pobj->p < '0' || *pobj->p > '9') && *pobj->p != 't' && *pobj->p != 'n' && *pobj->p != 'f'))) {
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
 * \param[in]       jsonČlen: JSON data length
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_parse_ex(lwjson_t* lwobj, const void* json_data, size_t json_len) {
    lwjsonr_t res = lwjsonOK;
    lwjson_token_t *t, *to;
    lwjson_int_str_t pobj = {.start = json_data, .len = json_len, .p = json_data};

    /* Check input parameters */
    if (lwobj == NULL || json_data == NULL || json_len == 0) {
        res = lwjsonERRPAR;
        goto ret;
    }

    /* set first token */
    to = &lwobj->first_token;

    /* values from very beginning */
    lwobj->flags.parsed = 0;
    lwobj->next_free_token_pos = 0;
    LWJSON_MEMSET(to, 0x00, sizeof(*to));

    /* First parse */
    res = prv_skip_blank(&pobj);
    if (res != lwjsonOK) {
        goto ret;
    }
    if (*pobj.p == '{') {
        to->type = LWJSON_TYPE_OBJECT;
    } else if (*pobj.p == '[') {
        to->type = LWJSON_TYPE_ARRAY;
    } else {
        res = lwjsonERRJSON;
        goto ret;
    }
    ++pobj.p;
    res = prv_check_valid_char_after_open_bracket(&pobj, to);
    if (res != lwjsonOK) {
        goto ret;
    }

    /* Process all characters as indicated by input user */
    while (pobj.p != NULL && *pobj.p != '\0' && (size_t)(pobj.p - pobj.start) < pobj.len) {
        /* Filter out blanks */
        res = prv_skip_blank(&pobj);
        if (res != lwjsonOK) {
            goto ret;
        }
        if (*pobj.p == ',') {
            ++pobj.p;
            continue;
        }

        /* Check if end of object or array*/
        if (*pobj.p == (to->type == LWJSON_TYPE_OBJECT ? '}' : ']')) {
            lwjson_token_t* parent = to->next;
            to->next = NULL;
            ++pobj.p;

            /* End of string if to == NULL (no parent), check if properly terminated */
            to = parent;
            if (to == NULL) {
                prv_skip_blank(&pobj);
                res = (pobj.p == NULL || *pobj.p == '\0' || (size_t)(pobj.p - pobj.start) == pobj.len) ? lwjsonOK
                                                                                                       : lwjsonERR;
                goto ret;
            }
            continue;
        }

        /* Allocate new token */
        t = prv_alloc_token(lwobj);
        if (t == NULL) {
            res = lwjsonERRMEM;
            goto ret;
        }

        /* If object type is not array, first thing is property that starts with quotes */
        if (to->type != LWJSON_TYPE_ARRAY) {
            if (*pobj.p != '"') {
                res = lwjsonERRJSON;
                goto ret;
            }
            res = prv_parse_property_name(&pobj, t);
            if (res != lwjsonOK) {
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
        switch (*pobj.p) {
            case '{':
            case '[':
                t->type = *pobj.p == '{' ? LWJSON_TYPE_OBJECT : LWJSON_TYPE_ARRAY;
                ++pobj.p;

                res = prv_check_valid_char_after_open_bracket(&pobj, t);
                if (res != lwjsonOK) {
                    goto ret;
                }
                t->next = to; /* Temporary saved as parent object */
                to = t;
                break;
            case '"':
                res = prv_parse_string(&pobj, &t->u.str.token_value, &t->u.str.token_value_len);
                if (res == lwjsonOK) {
                    t->type = LWJSON_TYPE_STRING;
                } else {
                    goto ret;
                }
                break;
            case 't':
                /* RFC4627 is lower-case only */
                if (strncmp(pobj.p, "true", 4) == 0) {
                    t->type = LWJSON_TYPE_TRUE;
                    pobj.p += 4;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            case 'f':
                /* RFC4627 is lower-case only */
                if (strncmp(pobj.p, "false", 5) == 0) {
                    t->type = LWJSON_TYPE_FALSE;
                    pobj.p += 5;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            case 'n':
                /* RFC4627 is lower-case only */
                if (strncmp(pobj.p, "null", 4) == 0) {
                    t->type = LWJSON_TYPE_NULL;
                    pobj.p += 4;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            default:
                if (*pobj.p == '-' || (*pobj.p >= '0' && *pobj.p <= '9')) {
                    if (prv_parse_number(&pobj, &t->type, &t->u.num_real, &t->u.num_int) != lwjsonOK) {
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
        if (t->type == LWJSON_TYPE_ARRAY || t->type == LWJSON_TYPE_OBJECT) {
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
        if (pobj.p == NULL || *pobj.p == '\0' || (*pobj.p != ',' && *pobj.p != ']' && *pobj.p != '}')) {
            res = lwjsonERRJSON;
            goto ret;
        } else if (*pobj.p == ',') { /* Check to advance to next token immediatey */
            ++pobj.p;
        }
    }
    if (to != &lwobj->first_token || (to != NULL && to->next != NULL)) {
        res = lwjsonERRJSON;
        to = NULL;
    }
    if (to != NULL) {
        if (to->type != LWJSON_TYPE_ARRAY && to->type != LWJSON_TYPE_OBJECT) {
            res = lwjsonERRJSON;
        }
        to->token_name = NULL;
        to->token_name_len = 0;
    }
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
