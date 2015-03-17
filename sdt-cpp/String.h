#ifndef INCLUDE_STRING_H
#define INCLUDE_STRING_H

#include <string>

typedef std::string String;

extern String StringPrintf(const char* fmt, ...);

#define STR StringPrintf

#endif // INCLUDE_STRING_H
