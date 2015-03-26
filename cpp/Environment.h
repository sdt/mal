#ifndef INCLUDE_ENVIRONMENT_H
#define INCLUDE_ENVIRONMENT_H

#include "MAL.h"

#include <map>

class malEnv {
public:
    malEnv() {}

    malValuePtr get(const String& symbol);
    malValuePtr set(const String& symbol, malValuePtr value);

private:
    typedef std::map<String, malValuePtr> Map;
    Map m_map;
};

#endif // INCLUDE_ENVIRONMENT_H
