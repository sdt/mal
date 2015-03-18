#ifndef INCLUDE_VALIDATION_H
#define INCLUDE_VALIDATION_H

#include "String.h"

#define ASSERT(condition, ...)  \
    if (!(condition)) { throw STR(__VA_ARGS__); } else { }

extern int check_args_count(const char* name, int expected, int got);
extern int check_args_between(const char* name, int min, int max, int got);
extern int check_args_at_least(const char* name, int min, int got);
extern int check_args_even(const char* name, int got);

#endif // INCLUDE_VALIDATION_H
