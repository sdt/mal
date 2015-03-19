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

    malObjectPtr string(const String& token) {
        return malObjectPtr(new malString(token));
    }

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
    return EVAL(m_body, makeEnv(argsBegin, argsEnd));
}

malObjectPtr malLambda::eval(malEnvPtr env)
{
    return malObjectPtr(this);
}

malEnvPtr malLambda::makeEnv(malObjectIter argsBegin, malObjectIter argsEnd)
{
    return malEnvPtr(new malEnv(m_env, m_bindings, argsBegin, argsEnd));
}

malObjectPtr malList::eval(malEnvPtr env)
{
    //TODO: is this still called after TCO?
    if (count() == 0) {
        return malObjectPtr(this);
    }

    malObjectVec items = eval_items(env);
    auto it = items.begin();
    malObjectPtr op = *it;
    return APPLY(op, ++it, items.end(), env);
}

String malList::print(bool readably) {
    return '(' + malSequence::print(readably) + ')';
}

bool malObject::isEqualTo(malObjectPtr rhs) {
    malObject* rhsp = rhs.ptr();
    // Check matching types, and then hand off to the apples vs apples
    // comparisons.
    return (typeid(*this) == typeid(*rhsp)) && doIsEqualTo(rhs.ptr());
}

bool malSequence::doIsEqualTo(malObject* rhs)
{
    malSequence* rhsSeq = static_cast<malSequence*>(rhs);
    if (count() != rhsSeq->count()) {
        return false;
    }

    for (malObjectIter it0 = m_items.begin(),
                       it1 = rhsSeq->items().begin(),
                       end = m_items.end(); it0 != end; ++it0, ++it1) {

        if (! (*it0)->isEqualTo(*it1)) {
            return false;
        }
    }
    return true;
}

malObjectVec malSequence::eval_items(malEnvPtr env)
{
    malObjectVec items;
    for (auto it = m_items.begin(), end = m_items.end(); it != end; ++it) {
        items.push_back(EVAL(*it, env));
    }
    return items;
}

String malSequence::print(bool readably) {
    String str;
    auto end = m_items.end();
    auto it = m_items.begin();
    if (it != end) {
        str += (*it)->print(readably);
        ++it;
    }
    for ( ; it != end; ++it) {
        str += " ";
        str += (*it)->print(readably);
    }
    return str;
}

malString::malString(const String& token)
: m_value(token)
{

}

malObjectPtr malString::eval(malEnvPtr env)
{
    return malObjectPtr(this);
}

String malString::escapedValue()
{
    return escape(m_value);
}

String malString::print(bool readably)
{
    return readably ? escapedValue() : m_value;
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

String malVector::print(bool readably) {
    return '[' + malSequence::print(readably) + ']';
}
