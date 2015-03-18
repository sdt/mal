#include "Debug.h"
#include "Environment.h"
#include "String.h"
#include "Types.h"

#define PLURAL(n)   &("s"[(n)==1])

static void check_args_count(const char* name, int expected, int got) {
    if (expected != got) {
        throw STR("\"%s\" expects %d arg%s, %d supplied",
                    name, expected, PLURAL(expected), got);
    }
}

#define CHECK_ARGS_COUNT(name, expected) \
    check_args_count(name, expected, args.size())

malObjectPtr builtIn_ADD(const malObjectVec& args, malEnvPtr env) {
    CHECK_ARGS_COUNT("+", 2);
    malInteger* lhs = OBJECT_CAST(malInteger, args[0]);
    malInteger* rhs = OBJECT_CAST(malInteger, args[1]);

    return mal::integer(lhs->value() + rhs->value());
}

struct Handler {
    const char* name;
    malBuiltIn::ApplyFunc* handler;
};

static Handler handler_table[] = {
    { "+", builtIn_ADD }
};

void install_core(malEnvPtr env) {
    int handlerCount = sizeof(handler_table) / sizeof(handler_table[0]);
    for (Handler *it = &handler_table[0],
                 *end = &handler_table[handlerCount]; it < end; ++it) {
        env->set(it->name, mal::builtin(it->name, it->handler));
    }
}
