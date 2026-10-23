/**
 * \file            lwjson_opts.h
 * \brief           Config file for escape sequence test
 */
#ifndef LWJSON_OPTS_HDR_H
#define LWJSON_OPTS_HDR_H

/*
 * Deliberately small string buffer so that longer values arrive
 * in several chunks and escape sequences get split across chunk boundaries.
 */
#define LWJSON_CFG_STREAM_STRING_MAX_LEN 8

#endif /* LWJSON_OPTS_HDR_H */
