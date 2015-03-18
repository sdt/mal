#ifndef INCLUDE_STRING_H
#define INCLUDE_STRING_H

#include <string>

typedef std::string String;

#define STR         StringPrintf
#define PLURAL(n)   &("s"[(n)==1])

extern String StringPrintf(const char* fmt, ...);
extern String CopyAndFree(char* mallocedString);


#endif // INCLUDE_STRING_H
