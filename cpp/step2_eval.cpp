#include "MAL.h"

#include "Environment.h"
#include "ReadLine.h"
#include "Types.h"

#include <iostream>
#include <memory>

malValuePtr READ(const String& input);
String PRINT(malValuePtr ast);

static ReadLine s_readLine("~/.mal-history");
static malBuiltIn::ApplyFunc
    builtIn_add, builtIn_sub, builtIn_mul, builtIn_div, builtIn_hash_map;

int main(int argc, char* argv[])
{
    String prompt = "user> ";
    String input;
    malEnv replEnv;
    replEnv.set("+", mal::builtin("+", &builtIn_add));
    replEnv.set("-", mal::builtin("-", &builtIn_sub));
    replEnv.set("*", mal::builtin("+", &builtIn_mul));
    replEnv.set("/", mal::builtin("/", &builtIn_div));
    replEnv.set("hash-map", mal::builtin("hash-map", &builtIn_hash_map));
    while (s_readLine.get(prompt, input)) {
        String out;
        try {
            out = rep(input, replEnv);
        }
        catch (malEmptyInputException&) {
            continue; // no output
        }
        catch (String& s) {
            out = s;
        };
        std::cout << out << "\n";
    }
    return 0;
}

String rep(const String& input, malEnv& env)
{
    return PRINT(EVAL(READ(input), env));
}

malValuePtr READ(const String& input)
{
    return readStr(input);
}

malValuePtr EVAL(malValuePtr ast, malEnv& env)
{
    return ast->eval(env);
}

String PRINT(malValuePtr ast)
{
    return ast->print(true);
}

malValuePtr APPLY(malValuePtr op, malValueIter argsBegin, malValueIter argsEnd,
                  malEnv& env)
{
    const malApplicable* handler = DYNAMIC_CAST(malApplicable, op);
    ASSERT(handler != NULL, "\"%s\" is not applicable", op->print(true).c_str());

    return handler->apply(argsBegin, argsEnd, env);
}

#define ARG(type, name) type* name = VALUE_CAST(type, *argsBegin++)

#define CHECK_ARGS_IS(expected) \
    checkArgsIs(name.c_str(), expected, std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_BETWEEN(min, max) \
    checkArgsBetween(name.c_str(), min, max, std::distance(argsBegin, argsEnd))


static malValuePtr builtIn_add(const String& name,
    malValueIter argsBegin, malValueIter argsEnd, malEnv& env)
{
        CHECK_ARGS_IS(2);
        ARG(malInteger, lhs);
        ARG(malInteger, rhs);
        return mal::integer(lhs->value() + rhs->value());
}

static malValuePtr builtIn_sub(const String& name,
    malValueIter argsBegin, malValueIter argsEnd, malEnv& env)
{
        int argCount = CHECK_ARGS_BETWEEN(1, 2);
        ARG(malInteger, lhs);
        if (argCount == 1) {
            return mal::integer(- lhs->value());
        }
        ARG(malInteger, rhs);
        return mal::integer(lhs->value() - rhs->value());
}

static malValuePtr builtIn_mul(const String& name,
    malValueIter argsBegin, malValueIter argsEnd, malEnv& env)
{
        CHECK_ARGS_IS(2);
        ARG(malInteger, lhs);
        ARG(malInteger, rhs);
        return mal::integer(lhs->value() * rhs->value());
}

static malValuePtr builtIn_div(const String& name,
    malValueIter argsBegin, malValueIter argsEnd, malEnv& env)
{
        CHECK_ARGS_IS(2);
        ARG(malInteger, lhs);
        ARG(malInteger, rhs);
        ASSERT(rhs->value() != 0, "Division by zero"); \
        return mal::integer(lhs->value() / rhs->value());
}

static malValuePtr builtIn_hash_map(const String& name,
    malValueIter argsBegin, malValueIter argsEnd, malEnv& env)
{
    return mal::hash(argsBegin, argsEnd);
}
