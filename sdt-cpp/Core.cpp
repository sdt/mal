#include "Debug.h"
#include "Environment.h"
#include "String.h"
#include "Types.h"
#include "Validation.h"

#include <iostream>

#define CHECK_ARGS_IS(name, expected) \
    check_args_is(name, expected, std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_BETWEEN(name, min, max) \
    check_args_between(name, min, max, std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_AT_LEAST(name, expected) \
    check_args_at_least(name, expected, std::distance(argsBegin, argsEnd))

extern malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env);
extern malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env);
extern String rep(const String& input, malEnvPtr env);
static String printObjects(malObjectIter begin, malObjectIter end,
                           const String& sep, bool readably);

#define ARG(type, name) type* name = OBJECT_CAST(type, *argsBegin++)

malObjectPtr builtIn_ADD(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("+", 2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::integer(lhs->value() + rhs->value());
}

malObjectPtr builtIn_APPLY(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_AT_LEAST("apply", 2);
    malObjectPtr op = *argsBegin++; // this gets checked in APPLY

    // Copy the first N-1 arguments in.
    malObjectVec args(argsBegin, argsEnd-1);

    // Then append the argument as a list.
    malList* lastArg = OBJECT_CAST(malList, *(argsEnd-1));
    for (int i = 0; i < lastArg->count(); i++) {
        args.push_back(lastArg->item(i));
    }

    return APPLY(op, args.begin(), args.end(), env->getRoot());
}

malObjectPtr builtIn_COUNT(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("=", 1);
    if (*argsBegin == mal::nil()) {
        return mal::integer(0);
    }

    ARG(malSequence, seq);
    return mal::integer(seq->count());
}

malObjectPtr builtIn_DIV(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("/", 2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::integer(lhs->value() / rhs->value());
}

malObjectPtr builtIn_EMPTY_Q(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("empty?", 1);
    ARG(malSequence, seq);

    return mal::boolean(seq->isEmpty());
}


malObjectPtr builtIn_EQUALS(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("=", 2);
    malObject* lhs = (*argsBegin++).ptr();
    malObject* rhs = (*argsBegin++).ptr();

    return mal::boolean(lhs->isEqualTo(rhs));
}

malObjectPtr builtIn_EVAL(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("eval", 1);
    return EVAL(*argsBegin, env->getRoot());
}

malObjectPtr builtIn_LE(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("*", 2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::boolean(lhs->value() <= rhs->value());
}

malObjectPtr builtIn_LIST_Q(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("list?", 1);
    return mal::boolean(DYNAMIC_CAST(malList, *argsBegin));
}

malObjectPtr builtIn_MUL(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("*", 2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::integer(lhs->value() * rhs->value());
}

malObjectPtr builtIn_PR_STR(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    return mal::string(printObjects(argsBegin, argsEnd, " ", true));
}

malObjectPtr builtIn_PRINTLN(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    std::cout << printObjects(argsBegin, argsEnd, " ", false) << "\n";
    return mal::nil();
}

malObjectPtr builtIn_PRN(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    std::cout << printObjects(argsBegin, argsEnd, " ", true) << "\n";
    return mal::nil();
}

malObjectPtr builtIn_STR(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    return mal::string(printObjects(argsBegin, argsEnd, "", false));
}

malObjectPtr builtIn_SUB(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    int argCount = CHECK_ARGS_BETWEEN("-", 1, 2);
    ARG(malInteger, lhs);
    if (argCount == 1) {
        return mal::integer(- lhs->value());
    }

    ARG(malInteger, rhs);
    return mal::integer(lhs->value() - rhs->value());
}

struct Handler {
    malBuiltIn::ApplyFunc* handler;
    const char* name;
};

static Handler handlerTable[] = {
    { builtIn_ADD,              "+"                                 },
    { builtIn_APPLY,            "apply"                             },
    { builtIn_COUNT,            "count"                             },
    { builtIn_DIV,              "/"                                 },
    { builtIn_EQUALS,           "="                                 },
    { builtIn_EMPTY_Q,          "empty?"                            },
    { builtIn_EVAL,             "eval"                              },
    { builtIn_LE,               "<="                                },
    { builtIn_LIST_Q,           "list?"                             },
    { builtIn_MUL,              "*"                                 },
    { builtIn_PR_STR,           "pr-str"                            },
    { builtIn_PRINTLN,          "println"                           },
    { builtIn_PRN,              "prn"                               },
    { builtIn_STR,              "str"                               },
    { builtIn_SUB,              "-"                                 },
};

static const char* malFunctionTable[] = {
    "(def! list (fn* (& items) items))",
    "(def! not (fn* (cond) (if cond false true)))",
    "(def! >= (fn* (a b) (<= b a)))",
    "(def! < (fn* (a b) (not (<= b a))))",
    "(def! > (fn* (a b) (not (<= a b))))",
};

void install_core(malEnvPtr env) {
    int handlerCount = sizeof(handlerTable) / sizeof(handlerTable[0]);
    for (Handler *it = &handlerTable[0],
                 *end = &handlerTable[handlerCount]; it < end; ++it) {
        env->set(it->name, mal::builtin(it->name, it->handler));
    }

    int nativeCount = sizeof(malFunctionTable) / sizeof(malFunctionTable[0]);
    for (int i = 0; i < nativeCount; i++) {
        rep(malFunctionTable[i], env);
    }
}

static String printObjects(malObjectIter begin, malObjectIter end,
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
