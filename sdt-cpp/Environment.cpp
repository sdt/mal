#include "Environment.h"

Environment::Environment(Environment* outer)
: m_outer(outer)
{
}

Environment* Environment::find(const String& symbol)
{
    for (Environment* env = this; env != NULL; env = env->m_outer) {
        if (env->m_map.find(symbol) != env->m_map.end()) {
            return env;
        }
    }
    return NULL;
}

malObjectPtr Environment::get(const String& symbol)
{
    for (Environment* env = this; env != NULL; env = env->m_outer) {
        auto it = env->m_map.find(symbol);
        if (it != env->m_map.end()) {
            return it->second;
        }
    }
    throw STR("\"%s\" not found", symbol.c_str());
}

void Environment::set(const String& symbol, malObjectPtr value)
{
    m_map[symbol] = value;
}
