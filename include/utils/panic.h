#pragma once

#include <stdio.h>
#include <stdlib.h>

#define panic(...)                                                            \
    fprintf(stderr, __VA_ARGS__);                                             \
    exit(EXIT_FAILURE);
