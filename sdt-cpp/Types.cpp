#include "Debug.h"
#include "Environment.h"
#include "Types.h"

extern malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env);
extern malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env);

namespace mal {
    malObjectPtr boolean(bool value) {
        return value ? trueObject() : falseObject();
    }

    malObjectPtr builtin(const String& name, malBuiltIn::ApplyFunc handler) {
        return malObjectPtr(new malBuiltIn(name, handler));
    };

    malObjectPtr falseObject() {
        static malObjectPtr c(new malConstant("false"));
        return malObjectPtr(c);
    };

    malObjectPtr integer(int value) {
        return malObjectPtr(new malInteger(value));
    };

    malObjectPtr integer(const String& token) {
        return integer(std::stoi(token));
    };

    malObjectPtr lambda(const StringVec& bindings, malObjectPtr body, malEnvPtr env) {
        return malObjectPtr(new malLambda(bindings, body, env));
    }

    malObjectPtr list(const malObjectVec& items) {
        return malObjectPtr(new malList(items));
    };

    malObjectPtr nil() {
        static malObjectPtr c(new malConstant("nil"));
        return malObjectPtr(c);
    };

    malObjectPtr symbol(const String& token) {
        return malObjectPtr(new malSymbol(token));
    };

    malObjectPtr trueObject() {
        static malObjectPtr c(new malConstant("true"));
        return malObjectPtr(c);
    };

    malObjectPtr vector(const malObjectVec& items) {
        return malObjectPtr(new malVector(items));
    };
};

malObjectPtr malBuiltIn::apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env)
{
    return m_handler(argsBegin, argsEnd, env);
}

malObjectPtr malBuiltIn::eval(malEnvPtr env)
{
    return malObjectPtr(this);
}

malObjectPtr malInteger::eval(malEnvPtr env)
{
    return malObjectPtr(this);
}

malLambda::malLambda(const StringVec& bindings,
                     malObjectPtr body, malEnvPtr env)
: m_bindings(bindings)
, m_body(body)
, m_env(env)
{

}

malObjectPtr malLambda::apply(malObjectIter argsBegin,
                              malObjectIter argsEnd,
                              malEnvPtr)
{
    malEnvPtr inner(new malEnv(m_env, m_bindings, argsBegin, argsEnd));
    return EVAL(m_body, inner);
}

malObjectPtr malLambda::eval(malEnvPtr env)
{
    return malObjectPtr(this);
}

malObjectPtr malList::eval(malEnvPtr env)
{
    if (count() == 0) {
        return malObjectPtr(this);
    }

    malObjectVec items = eval_items(env);
    auto it = items.begin();
    malObjectPtr op = *it;
    return APPLY(op, ++it, items.end(), env);
}

String malList::print() {
    return '(' + malSequence::print() + ')';
}

malObjectVec malSequence::eval_items(malEnvPtr env)
{
    malObjectVec items;
    for (auto it = m_items.begin(), end = m_items.end(); it != end; ++it) {
        items.push_back(EVAL(*it, env));
    }
    return items;
}

String malSequence::print() {
    String str;
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
    return str;
}

malObjectPtr malSymbol::eval(malEnvPtr env)
{
    return env->get(m_value);
}

malObjectPtr malVector::eval(malEnvPtr env)
{
    malObjectVec items = eval_items(env);
    return mal::vector(items);
}

String malVector::print() {
    return '[' + malSequence::print() + ']';
}

