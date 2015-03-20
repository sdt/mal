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

    malObjectPtr hash(malObjectIter argsBegin, malObjectIter argsEnd) {
        return malObjectPtr(new malHash(argsBegin, argsEnd));
    }

    malObjectPtr integer(int value) {
        return malObjectPtr(new malInteger(value));
    };

    malObjectPtr integer(const String& token) {
        return integer(std::stoi(token));
    };

    malObjectPtr keyword(const String& token) {
        return malObjectPtr(new malKeyword(token));
    };

    malObjectPtr lambda(const StringVec& bindings, malObjectPtr body, malEnvPtr env) {
        return malObjectPtr(new malLambda(bindings, body, env));
    }

    malObjectPtr list(const malObjectVec& items) {
        return malObjectPtr(new malList(items));
    };

    malObjectPtr list(malObjectIter begin, malObjectIter end) {
        return malObjectPtr(new malList(begin, end));
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

    malObjectPtr vector(malObjectIter begin, malObjectIter end) {
        return malObjectPtr(new malVector(begin, end));
    };
};

malObjectPtr malBuiltIn::apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env) const
{
    return m_handler(m_name, argsBegin, argsEnd, env);
}

static String makeHashKey(malObjectPtr key)
{
    if (const malString* skey = DYNAMIC_CAST(malString, key)) {
        return skey->print(true);
    }
    else if (const malKeyword* kkey = DYNAMIC_CAST(malKeyword, key)) {
        return kkey->print(true);
    }
    ASSERT(false, "%s is not a string or keyword", key->print(true).c_str());
}

malHash::malHash(malObjectIter argsBegin, malObjectIter argsEnd)
{
    // This is intended to be called with pre-evaluated arguments.
    int itemCount = std::distance(argsBegin, argsEnd);
    ASSERT(itemCount % 2 == 0, "hash-map requires even-sized list");
    for (auto it = argsBegin; it != argsEnd; ++it) {
        String key = makeHashKey(*it++);
        m_map[key] = *it;
    }
}

String malHash::print(bool readably) const
{
    String s = "{";

    auto it = m_map.begin(), end = m_map.end();
    if (it != end) {
        s += it->first + " " + it->second->print(readably);
        ++it;
    }
    for ( ; it != end; ++it) {
        s += " " + it->first + " " + it->second->print(readably);
    }

    return s + "}";
}

bool malHash::doIsEqualTo(const malObject* rhs) const
{
    const malHash::Map& r_map = static_cast<const malHash*>(rhs)->m_map;
    if (m_map.size() != r_map.size()) {
        return false;
    }

    for (auto it0 = m_map.begin(), end0 = m_map.end(), it1 = r_map.begin();
         it0 != end0; ++it0, ++it1) {

        if (it0->first != it1->first) {
            return false;
        }
        if (!it0->second->isEqualTo(it1->second)) {
            return false;
        }
    }
    return true;
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
                              malEnvPtr) const
{
    return EVAL(m_body, makeEnv(argsBegin, argsEnd));
}

malEnvPtr malLambda::makeEnv(malObjectIter argsBegin, malObjectIter argsEnd) const
{
    return malEnvPtr(new malEnv(m_env, m_bindings, argsBegin, argsEnd));
}

malObjectPtr malList::first() const
{
    ASSERT(count() > 0, "Cannot take first of empty list");
    return item(0);
}

String malList::print(bool readably) const
{
    return '(' + malSequence::print(readably) + ')';
}

malObjectPtr malList::rest() const
{
    ASSERT(count() > 0, "Cannot take rest of empty list");
    malObjectVec items(begin() + 1, end());
    return mal::list(items);
}

malObjectPtr malObject::eval(malEnvPtr env) const
{
    // Default case of eval is just to return the object itself.
    return malObjectPtr(this);
}

bool malObject::isEqualTo(malObjectPtr rhs) const
{
    const malObject* rhsp = rhs.ptr();

    // Special-case. Vectors and Lists can be compared.
    bool matchingTypes = (typeid(*this) == typeid(*rhsp)) ||
        (dynamic_cast<const malSequence*>(this) &&
         dynamic_cast<const malSequence*>(rhsp));

    return matchingTypes && doIsEqualTo(rhs.ptr());
}

bool malObject::isTrue() const
{
    return (this != mal::falseObject().ptr())
        && (this != mal::nil().ptr());
}

bool malSequence::doIsEqualTo(const malObject* rhs) const
{
    const malSequence* rhsSeq = static_cast<const malSequence*>(rhs);
    if (count() != rhsSeq->count()) {
        return false;
    }

    for (malObjectIter it0 = m_items.begin(),
                       it1 = rhsSeq->begin(),
                       end = m_items.end(); it0 != end; ++it0, ++it1) {

        if (! (*it0)->isEqualTo(*it1)) {
            return false;
        }
    }
    return true;
}

malObjectVec malSequence::eval_items(malEnvPtr env) const
{
    malObjectVec items;
    for (auto it = m_items.begin(), end = m_items.end(); it != end; ++it) {
        items.push_back(EVAL(*it, env));
    }
    return items;
}

String malSequence::print(bool readably) const
{
    String str;
    auto end = m_items.cend();
    auto it = m_items.cbegin();
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

String malString::escapedValue() const
{
    return escape(m_value);
}

String malString::print(bool readably) const
{
    return readably ? escapedValue() : m_value;
}

malObjectPtr malSymbol::eval(malEnvPtr env) const
{
    return env->get(m_value);
}

malObjectPtr malVector::eval(malEnvPtr env) const
{
    //TODO: check if vectors can be handled like hashes
    //      ie. [ V ] -> (vector V)
    // I suspect not when they are used as parameter lists for fn*
    malObjectVec items = eval_items(env);
    return mal::vector(items);
}

String malVector::print(bool readably) const
{
    return '[' + malSequence::print(readably) + ']';
}
