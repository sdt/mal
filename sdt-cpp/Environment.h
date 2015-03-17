#ifndef INCLUDE_ENVIRONMENT_H
#define INCLUDE_ENVIRONMENT_H

#include "String.h"
#include "Types.h"

#include <map>

class Environment {
public:
    Environment(//malListPtr bindings,
                //malListPtr values,
                Environment* outer = NULL);

    malObjectPtr get(const String& symbol);
    Environment* find(const String& symbol);
    void set(const String& symbol, malObjectPtr value);

private:
    typedef std::map<String, malObjectPtr> Map;
    Map m_map;
    Environment* m_outer;
};

#endif // INCLUDE_ENVIRONMENT_H
