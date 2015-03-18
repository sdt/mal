#include "Debug.h"
#include "Environment.h"
#include "String.h"
#include "Types.h"

#define PLURAL(n)   &("s"[(n)==1])

static void check_args_count(const char* name, int expected, int got) {
    if (got != expected) {
        throw STR("\"%s\" expects %d arg%s, %d supplied",
                    name, expected, PLURAL(expected), got);
    }
}

static int check_args_between(const char* name, int min, int max, int got) {
    if (got < min || got > max) {
        throw STR("\"%s\" expects between %d and %d arg%s, %d supplied",
                    name, min, max, PLURAL(max), got);
    }
    return got;
}

static int check_args_at_least(const char* name, int min, int got) {
    if (got < min) {
        throw STR("\"%s\" expects at least %d arg%s, %d supplied",
                    name, min, PLURAL(min), got);
    }
    return got;
}

#define CHECK_ARGS_COUNT(name, expected) \
    check_args_count(name, expected, std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_BETWEEN(name, min, max) \
    check_args_between(name, min, max, std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_AT_LEAST(name, expected) \
    check_args_count(name, expected, std::distance(argsBegin, argsEnd))


malObjectPtr builtIn_ADD(malObjectIter argsBegin,
                         malObjectIter argsEnd,
                         malEnvPtr env)
{
    CHECK_ARGS_COUNT("+", 2);
    malInteger* lhs = OBJECT_CAST(malInteger, *argsBegin++);
    malInteger* rhs = OBJECT_CAST(malInteger, *argsBegin++);

    return mal::integer(lhs->value() + rhs->value());
}

malObjectPtr builtIn_SUB(malObjectIter argsBegin,
                         malObjectIter argsEnd,
                         malEnvPtr env)
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
    const char* name;
    malBuiltIn::ApplyFunc* handler;
};

static Handler handler_table[] = {
    { "+", builtIn_ADD },
    { "-", builtIn_SUB }
};

void install_core(malEnvPtr env) {
    int handlerCount = sizeof(handler_table) / sizeof(handler_table[0]);
    for (Handler *it = &handler_table[0],
                 *end = &handler_table[handlerCount]; it < end; ++it) {
        env->set(it->name, mal::builtin(it->name, it->handler));
    }
}
