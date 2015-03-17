#ifndef INCLUDE_STRING_H
#define INCLUDE_STRING_H

#include <string>

typedef std::string String;

#define STR StringPrintf

extern String StringPrintf(const char* fmt, ...);
extern String CopyAndFree(char* mallocedString);


#endif // INCLUDE_STRING_H
