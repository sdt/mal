#ifndef INCLUDE_MAL_H
#define INCLUDE_MAL_H

#include "Debug.h"
#include "RefCounted.h"
#include "RefCountedPtr.h"
#include "String.h"
#include "Validation.h"

#include <vector>

class malObject;
typedef RefCountedPtr<const malObject>      malObjectPtr;
typedef std::vector<malObjectPtr>           malObjectVec;
typedef malObjectVec::const_iterator        malObjectIter;

class malEnv;
typedef RefCountedPtr<malEnv>               malEnvPtr;

// step*.cpp
extern malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env);
extern malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env);
extern malObjectPtr readline(const String& prompt);
extern String rep(const String& input, malEnvPtr env);

// Core.cpp
extern void install_core(malEnvPtr env);

// Reader.cpp
extern malObjectPtr read_str(const String& input);

#endif // INCLUDE_MAL_H
