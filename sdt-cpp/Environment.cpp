#include "Environment.h"

#include "String.h"
#include "Types.h"

malEnv::malEnv(malEnvPtr outer)
: m_outer(outer)
{
    TRACE_ENV("Creating malEnv %p, outer=%p\n", this, m_outer.ptr());
}

malEnv::~malEnv()
{
    TRACE_ENV("Destroying malEnv %p, outer=%p\n", this, m_outer.ptr());
}

malEnvPtr malEnv::find(const String& symbol)
{
    for (malEnvPtr env = this; env; env = env->m_outer) {
        if (env->m_map.find(symbol) != env->m_map.end()) {
            return env;
        }
    }
    return NULL;
}

malObjectPtr malEnv::get(const String& symbol)
{
    for (malEnvPtr env = this; env; env = env->m_outer) {
        auto it = env->m_map.find(symbol);
        if (it != env->m_map.end()) {
            return it->second;
        }
    }
    throw STR("\"%s\" not found", symbol.c_str());
}

malObjectPtr malEnv::set(const String& symbol, malObjectPtr value)
{
    m_map[symbol] = value;
    return value;
}
