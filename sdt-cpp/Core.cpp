#include "Debug.h"
#include "Environment.h"
#include "String.h"
#include "Types.h"
#include "Validation.h"

#include <iostream>

#define CHECK_ARGS_IS(expected) \
    check_args_is(name.c_str(), expected, \
                  std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_BETWEEN(min, max) \
    check_args_between(name.c_str(), min, max, \
                       std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_AT_LEAST(expected) \
    check_args_at_least(name.c_str(), expected, \
                        std::distance(argsBegin, argsEnd))

extern malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env);
extern malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env);
extern String rep(const String& input, malEnvPtr env);
static String printObjects(malObjectIter begin, malObjectIter end,
                           const String& sep, bool readably);

#define ARG(type, name) type* name = OBJECT_CAST(type, *argsBegin++)
#define BUILTIN(func) \
    malObjectPtr builtIn_##func(const String& name, \
        malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)

BUILTIN(ADD)
{
    CHECK_ARGS_IS(2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::integer(lhs->value() + rhs->value());
}

BUILTIN(APPLY)
{
    CHECK_ARGS_AT_LEAST(2);
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

BUILTIN(COUNT)
{
    CHECK_ARGS_IS(1);
    if (*argsBegin == mal::nil()) {
        return mal::integer(0);
    }

    ARG(malSequence, seq);
    return mal::integer(seq->count());
}

BUILTIN(DIV)
{
    CHECK_ARGS_IS(2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::integer(lhs->value() / rhs->value());
}

BUILTIN(EMPTY_Q)
{
    CHECK_ARGS_IS(1);
    ARG(malSequence, seq);

    return mal::boolean(seq->isEmpty());
}


BUILTIN(EQUALS)
{
    CHECK_ARGS_IS(2);
    malObject* lhs = (*argsBegin++).ptr();
    malObject* rhs = (*argsBegin++).ptr();

    return mal::boolean(lhs->isEqualTo(rhs));
}

BUILTIN(EVAL)
{
    CHECK_ARGS_IS(1);
    return EVAL(*argsBegin, env->getRoot());
}

BUILTIN(LE)
{
    CHECK_ARGS_IS(2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::boolean(lhs->value() <= rhs->value());
}

BUILTIN(LIST_Q)
{
    CHECK_ARGS_IS(1);
    return mal::boolean(DYNAMIC_CAST(malList, *argsBegin));
}

BUILTIN(MUL)
{
    CHECK_ARGS_IS(2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::integer(lhs->value() * rhs->value());
}

BUILTIN(PR_STR)
{
    return mal::string(printObjects(argsBegin, argsEnd, " ", true));
}

BUILTIN(PRINTLN)
{
    std::cout << printObjects(argsBegin, argsEnd, " ", false) << "\n";
    return mal::nil();
}

BUILTIN(PRN)
{
    std::cout << printObjects(argsBegin, argsEnd, " ", true) << "\n";
    return mal::nil();
}

BUILTIN(STR)
{
    return mal::string(printObjects(argsBegin, argsEnd, "", false));
}

BUILTIN(SUB)
{
    int argCount = CHECK_ARGS_BETWEEN(1, 2);
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
