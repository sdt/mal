#include "MAL.h"

#include "Environment.h"
#include "ReadLine.h"
#include "Types.h"

#include <iostream>

malValuePtr READ(const String& input);
String PRINT(malValuePtr ast);

static ReadLine s_readLine("~/.mal-history");

int main(int argc, char* argv[])
{
    String prompt = "user> ";
    String input;
    malEnvPtr replEnv(new malEnv);
    installCore(replEnv);
    while (s_readLine.get(prompt, input)) {
        String out;
        try {
            out = rep(input, replEnv);
        }
        catch (String& s) {
            out = s;
        };
        std::cout << out << "\n";
    }
}

String rep(const String& input, malEnvPtr env)
{
    return PRINT(EVAL(READ(input), env));
}

malValuePtr READ(const String& input)
{
    return readStr(input);
}

#define CHECK_ARGS_IS(name, expected) \
    checkArgsIs(name, expected, argCount)

malValuePtr EVAL(malValuePtr ast, malEnvPtr env)
{
    if (const malList* list = DYNAMIC_CAST(malList, ast)) {
        const malSymbol* symbol;
        if ((list->count() > 0) &&
            (symbol = DYNAMIC_CAST(malSymbol, list->item(0)))) {

            String special = symbol->value();
            int argCount = list->count() - 1;

            if (special == "def!") {
                CHECK_ARGS_IS("def!", 2);
                const malSymbol* id = VALUE_CAST(malSymbol, list->item(1));
                return env->set(id->value(), EVAL(list->item(2), env));
            }

            if (special == "let*") {
                CHECK_ARGS_IS("let*", 2);
                const malSequence* bindings =
                    VALUE_CAST(malSequence, list->item(1));
                int count = checkArgsEven("let*", bindings->count());
                malEnvPtr inner(new malEnv(env));
                for (int i = 0; i < count; i += 2) {
                    const malSymbol* var =
                        VALUE_CAST(malSymbol, bindings->item(i));
                    inner->set(var->value(), EVAL(bindings->item(i+1), inner));
                }
                return EVAL(list->item(2), inner);
            }
        }
    }
    return ast->eval(env);
}

String PRINT(malValuePtr ast)
{
    return ast->print(true);
}

malValuePtr APPLY(malValuePtr op, malValueIter argsBegin, malValueIter argsEnd,
                  malEnvPtr env)
{
    const malApplicable* handler = DYNAMIC_CAST(malApplicable, op);
    ASSERT(handler != NULL, "\"%s\" is not applicable",
                            op->print(true).c_str());

    return handler->apply(argsBegin, argsEnd, env);
}

malValuePtr readline(const String& prompt)
{
    String input;
    if (s_readLine.get(prompt, input)) {
        return mal::string(input);
    }
    return mal::nilValue();
}
