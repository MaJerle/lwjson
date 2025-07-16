/**
 * \file            tests.h
 * \author          Tilen MAJERLE <tilen@majerle.eu>
 * \brief           
 * \version         0.1
 * \date            2025-03-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef TEST_HDR_H
#define TEST_HDR_H

#include "math.h"

/* Assert check */
#define TEST_ASSERT(x)                                                                                                 \
    do {                                                                                                               \
        if (!(x)) {                                                                                                    \
            printf("Assert in file %s and on line %d failed with condition (" #x ")\r\n", __FILE__, (int)__LINE__);    \
            return -1;                                                                                                 \
        }                                                                                                              \
    } while (0)

#define INT_IS_EQUAL(a, b)   ((a) == (b))
#define FLOAT_IS_EQUAL(a, b) ((((a) * (a)) - ((b) * (b))) <= 0.0001)

#endif /* TEST_HDR_H */