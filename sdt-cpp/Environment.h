#ifndef INCLUDE_ENVIRONMENT_H
#define INCLUDE_ENVIRONMENT_H

#include "MAL.h"

#include <map>

class malEnv : public RefCounted {
public:
    malEnv(malEnvPtr outer = NULL);
    malEnv(malEnvPtr outer,
           const StringVec& bindings,
           malObjectIter argsBegin,
           malObjectIter argsEnd);

    ~malEnv();

    malObjectPtr get(const String& symbol);
    malEnvPtr find(const String& symbol);
    malObjectPtr set(const String& symbol, malObjectPtr value);
    malEnvPtr getRoot();

private:
    typedef std::map<String, malObjectPtr> Map;
    Map m_map;
    malEnvPtr m_outer;
};

#endif // INCLUDE_ENVIRONMENT_H
