#include "MAL.h"
#include "Types.h"

int checkArgsIs(const char* name, int expected, int got)
{
    MAL_CHECK(got == expected,
           "\"%s\" expects %d arg%s, %d supplied",
           name, expected, PLURAL(expected), got);
    return got;
}

int checkArgsBetween(const char* name, int min, int max, int got)
{
    MAL_CHECK((got >= min) && (got <= max),
           "\"%s\" expects between %d and %d arg%s, %d supplied",
           name, min, max, PLURAL(max), got);
    return got;
}

int checkArgsAtLeast(const char* name, int min, int got)
{
    MAL_CHECK(got >= min,
           "\"%s\" expects at least %d arg%s, %d supplied",
           name, min, PLURAL(min), got);
    return got;
}

int checkArgsEven(const char* name, int got)
{
    MAL_CHECK(got % 2 == 0,
           "\"%s\" expects an even number of args, %d supplied",
           name, got);
    return got;
}

static String argsErrorMsg(const char* sym, int min, int max, int got)
{
    String expected;
    if (min == max) {
        // exact
        expected = STRF("%d arg%s", min, PLURAL(min));
    }
    else if (min < max) {
        // range
        expected = STRF("between %d and %d args", min, max);
    }
    else {
        // at least min
        expected = STRF("at least %d arg%s", min, PLURAL(min));
    }
    return STRF("\"%s\" expects %s, %d supplied", sym, expected.c_str(), got);
}

malValuePtr shiftArgs(const char* sym, malValuePtr argList,
                      int min, int max, malValuePtr* args)
{
    // We need to be careful to hold onto these malList* values with
    // malValuePtrs so they don't get GC'd out from underneath us.
    malList* list = VALUE_CAST(malList, argList);
    int i = 0;
    for ( ; i < min; i++) {
        MAL_CHECK(!list->isEmpty(), argsErrorMsg(sym, min, max, i).c_str());

        args[i] = list->first();
        argList = list->rest();
        list = VALUE_CAST(malList, argList);
    }

    // Note we only enter this loop if min < max
    for ( ; !list->isEmpty() && (i < max); i++) {
        args[i] = list->first();
        argList = list->rest();
        list = VALUE_CAST(malList, argList);
    }

    // Now check that there isn't args left over, if need be.
    if (min <= max) {
        MAL_CHECK(list->isEmpty(),
                  argsErrorMsg(sym, min, max, i + list->count()).c_str());
    }

    return argList;
}

