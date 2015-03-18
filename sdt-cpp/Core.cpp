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


malObjectPtr builtIn_ADD(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("+", 2);
    malInteger* lhs = OBJECT_CAST(malInteger, *argsBegin++);
    malInteger* rhs = OBJECT_CAST(malInteger, *argsBegin++);

    return mal::integer(lhs->value() + rhs->value());
}

malObjectPtr builtIn_DIV(
    malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    CHECK_ARGS_IS("/", 2);
    malInteger* lhs = OBJECT_CAST(malInteger, *argsBegin++);
    malInteger* rhs = OBJECT_CAST(malInteger, *argsBegin++);

    return mal::integer(lhs->value() / rhs->value());
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
    { builtIn_DIV,              "/"                                 },
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
