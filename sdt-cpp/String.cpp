#include "String.h"

#include <stdarg.h>
#include <stdio.h>

// Adapted from: http://stackoverflow.com/questions/2342162
String StringPrintf(const char* fmt, ...) {
    int size = strlen(fmt); // make a guess
    String str;
    va_list ap;
    while (1) {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt, ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    return str;
}
