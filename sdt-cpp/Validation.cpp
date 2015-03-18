#include "Validation.h"

int check_args_is(const char* name, int expected, int got)
{
    ASSERT(got == expected,
           "\"%s\" expects %d arg%s, %d supplied",
           name, expected, PLURAL(expected), got);
    return got;
}

int check_args_between(const char* name, int min, int max, int got)
{
    ASSERT((got >= min) && (got <= max),
           "\"%s\" expects between %d and %d arg%s, %d supplied",
           name, min, max, PLURAL(max), got);
    return got;
}

int check_args_at_least(const char* name, int min, int got)
{
    ASSERT(got >= min,
           "\"%s\" expects at least %d arg%s, %d supplied",
           name, min, PLURAL(min), got);
    return got;
}

int check_args_even(const char* name, int got)
{
    ASSERT(got % 2 == 0,
           "\"%s\" expects an even number of args, %d supplied",
           name, got);
    return got;
}
