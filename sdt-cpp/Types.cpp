#include "Debug.h"
#include "Environment.h"
#include "Types.h"

extern malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env);
extern malObjectPtr APPLY(malObjectPtr op, malObjectVec args, malEnvPtr env);

malObjectPtr malList::eval(malEnvPtr env)
{
    malObjectVec items = eval_items(env);
    if (items.empty()) {
        throw STR("Cannot eval an empty list");
    }
    auto it = items.begin();
    malObjectPtr op = *it;
    malObjectVec args(items.size() - 1);
    std::copy(++it, items.end(), args.begin());
    return APPLY(op, args, env);
}

malObjectPtr malBuiltIn::eval(malEnvPtr env)
{
    return malObjectPtr(this);
}

malObjectPtr malInteger::eval(malEnvPtr env)
{
    return malObjectPtr(this);
}

malObjectVec malSequence::eval_items(malEnvPtr env)
{
    malObjectVec items;
    for (auto it = m_items.begin(), end = m_items.end(); it != end; ++it) {
        items.push_back(EVAL(*it, env));
    }
    return items;
}

malObjectPtr malSymbol::eval(malEnvPtr env)
{
    return env->get(m_value);
}

String malSequence::print() {
    String str = "(";
    auto end = m_items.end();
    auto it = m_items.begin();
    if (it != end) {
        str += (*it)->print();
        ++it;
    }
    for ( ; it != end; ++it) {
        str += " ";
        str += (*it)->print();
    }
    str += ")";
    return str;
}
