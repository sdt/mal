#ifndef INCLUDE_MAL_H
#define INCLUDE_MAL_H

#include <string>

typedef std::string         String;

extern String rep(const String& input);
extern String READ(const String& input);
extern String EVAL(const String& input);
extern String PRINT(const String& input);

#endif // INCLUDE_MAL_H
