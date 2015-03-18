#include "Debug.h"
#include "Environment.h"
#include "String.h"
#include "Types.h"
#include "Validation.h"

#define CHECK_ARGS_IS(name, expected) \
    check_args_is(name, expected, std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_BETWEEN(name, min, max) \
    check_args_between(name, min, max, std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_AT_LEAST(name, expected) \
    check_args_at_least(name, expected, std::distance(argsBegin, argsEnd))

extern malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env);
extern malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env);

malObjectPtr builtIn_ADD(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("+", 2);
    malInteger* lhs = OBJECT_CAST(malInteger, *argsBegin++);
    malInteger* rhs = OBJECT_CAST(malInteger, *argsBegin++);

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

malObjectPtr builtIn_DIV(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("/", 2);
    malInteger* lhs = OBJECT_CAST(malInteger, *argsBegin++);
    malInteger* rhs = OBJECT_CAST(malInteger, *argsBegin++);

    return mal::integer(lhs->value() / rhs->value());
}

malObjectPtr builtIn_EVAL(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("eval", 1);
    return EVAL(*argsBegin, env->getRoot());
}

malObjectPtr builtIn_MUL(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("*", 2);
    malInteger* lhs = OBJECT_CAST(malInteger, *argsBegin++);
    malInteger* rhs = OBJECT_CAST(malInteger, *argsBegin++);

    return mal::integer(lhs->value() * rhs->value());
}

malObjectPtr builtIn_SUB(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    int argCount = CHECK_ARGS_BETWEEN("-", 1, 2);
    malInteger* lhs = OBJECT_CAST(malInteger, *argsBegin++);
    if (argCount == 1) {
        return mal::integer(- lhs->value());
    }

    malInteger* rhs = OBJECT_CAST(malInteger, *argsBegin++);
    return mal::integer(lhs->value() - rhs->value());
}

struct Handler {
    malBuiltIn::ApplyFunc* handler;
    const char* name;
};

static Handler handler_table[] = {
    { builtIn_ADD,              "+"                                 },
    { builtIn_APPLY,            "apply"                             },
    { builtIn_DIV,              "/"                                 },
    { builtIn_EVAL,             "eval"                              },
    { builtIn_MUL,              "*"                                 },
    { builtIn_SUB,              "-"                                 },
};

void install_core(malEnvPtr env) {
    int handlerCount = sizeof(handler_table) / sizeof(handler_table[0]);
    for (Handler *it = &handler_table[0],
                 *end = &handler_table[handlerCount]; it < end; ++it) {
        env->set(it->name, mal::builtin(it->name, it->handler));
    }
}
