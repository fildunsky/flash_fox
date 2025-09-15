#pragma once

#include <stdio.h>

#define errorf(fmt, ...)                     \
    do {                                     \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
        fflush(stderr);                      \
    } while (0)

#define fatalf(fmt, ...)            \
    do {                            \
        errorf(fmt, ##__VA_ARGS__); \
        exit(EXIT_FAILURE);         \
    } while (0)

#define infof(fmt, ...)                      \
    do {                                     \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
        fflush(stderr);                      \
    } while (0)

