#include "MAL.h"
#include "Environment.h"
#include "StaticList.h"
#include "Types.h"

#include <iostream>

#define CHECK_ARGS_IS(expected) \
    checkArgsIs(name.c_str(), expected, \
                  std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_BETWEEN(min, max) \
    checkArgsBetween(name.c_str(), min, max, \
                       std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_AT_LEAST(expected) \
    checkArgsAtLeast(name.c_str(), expected, \
                        std::distance(argsBegin, argsEnd))

static String printValues(malValueIter begin, malValueIter end,
                           const String& sep, bool readably);

static StaticList<malBuiltIn*> handlers;

#define ARG(type, name) type* name = VALUE_CAST(type, *argsBegin++)

#define FUNCNAME(uniq) builtIn ## uniq
#define HRECNAME(uniq) handler ## uniq
#define BUILTIN_DEF(uniq, symbol) \
    static malBuiltIn::ApplyFunc FUNCNAME(uniq); \
    static StaticList<malBuiltIn*>::Node HRECNAME(uniq) \
        (handlers, new malBuiltIn(symbol, FUNCNAME(uniq))); \
    malValuePtr FUNCNAME(uniq)(const String& name, \
        malValueIter argsBegin, malValueIter argsEnd, malEnvPtr env)

#define BUILTIN(symbol)  BUILTIN_DEF(__LINE__, symbol)

#define BUILTIN_ISA(symbol, type) \
    BUILTIN(symbol) { \
        CHECK_ARGS_IS(1); \
        return mal::boolean(DYNAMIC_CAST(type, *argsBegin)); \
    }

#define BUILTIN_INTOP(op, checkDivByZero) \
    BUILTIN(#op) { \
        CHECK_ARGS_IS(2); \
        ARG(malInteger, lhs); \
        ARG(malInteger, rhs); \
        if (checkDivByZero) { \
            ASSERT(rhs->value() != 0, "Division by zero"); \
        } \
        return mal::integer(lhs->value() op rhs->value()); \
    }

BUILTIN_ISA("list?",        malList);

BUILTIN_INTOP(+,            false);
BUILTIN_INTOP(/,            true);
BUILTIN_INTOP(*,            false);
BUILTIN_INTOP(%,            true);

BUILTIN("-")
{
    int argCount = CHECK_ARGS_BETWEEN(1, 2);
    ARG(malInteger, lhs);
    if (argCount == 1) {
        return mal::integer(- lhs->value());
    }

    ARG(malInteger, rhs);
    return mal::integer(lhs->value() - rhs->value());
}

BUILTIN("<=")
{
    CHECK_ARGS_IS(2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::boolean(lhs->value() <= rhs->value());
}

BUILTIN("=")
{
    CHECK_ARGS_IS(2);
    const malValue* lhs = (*argsBegin++).ptr();
    const malValue* rhs = (*argsBegin++).ptr();

    return mal::boolean(lhs->isEqualTo(rhs));
}

BUILTIN("count")
{
    CHECK_ARGS_IS(1);
    if (*argsBegin == mal::nilValue()) {
        return mal::integer(0);
    }

    ARG(malSequence, seq);
    return mal::integer(seq->count());
}

BUILTIN("empty?")
{
    CHECK_ARGS_IS(1);
    ARG(malSequence, seq);

    return mal::boolean(seq->isEmpty());
}

BUILTIN("hash-map")
{
    return mal::hash(argsBegin, argsEnd);
}

BUILTIN("pr-str")
{
    return mal::string(printValues(argsBegin, argsEnd, " ", true));
}

BUILTIN("println")
{
    std::cout << printValues(argsBegin, argsEnd, " ", false) << "\n";
    return mal::nilValue();
}

BUILTIN("prn")
{
    std::cout << printValues(argsBegin, argsEnd, " ", true) << "\n";
    return mal::nilValue();
}

BUILTIN("str")
{
    return mal::string(printValues(argsBegin, argsEnd, "", false));
}

static const char* malFunctionTable[] = {
    "(def! list (fn* (& items) items))",
    "(def! not (fn* (cond) (if cond false true)))",
    "(def! >= (fn* (a b) (<= b a)))",
    "(def! < (fn* (a b) (not (<= b a))))",
    "(def! > (fn* (a b) (not (<= a b))))",
};

void installCore(malEnvPtr env) {
    for (auto it = handlers.begin(), end = handlers.end(); it != end; ++it) {
        malBuiltIn* handler = *it;
        env->set(handler->name(), handler);
    }

    for (int i = 0; i < ARRAY_SIZE(malFunctionTable); i++) {
        rep(malFunctionTable[i], env);
    }
}

static String printValues(malValueIter begin, malValueIter end,
                          const String& sep, bool readably)
{
    String out;

    if (begin != end) {
        out += (*begin)->print(readably);
        ++begin;
    }

    for ( ; begin != end; ++begin) {
        out += sep;
        out += (*begin)->print(readably);
    }

    return out;
}
