#include "Environment.h"
#include "Types.h"

malObjectPtr malInteger::eval(Environment* env)
{
    return malObjectPtr(this);
}

malObjectPtr malSymbol::eval(Environment* env)
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
