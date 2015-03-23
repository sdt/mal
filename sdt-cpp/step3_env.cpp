#include "MAL.h"

#include "Environment.h"
#include "ReadLine.h"
#include "Types.h"

#include <iostream>

malObjectPtr READ(const String& input);
String PRINT(malObjectPtr ast);

static ReadLine s_readLine("~/.mal-history");

int main(int argc, char* argv[])
{
    String prompt = "user> ";
    String input;
    malEnvPtr repl_env(new malEnv);
    install_core(repl_env);
    while (s_readLine.get(prompt, input)) {
        String out;
        try {
            out = rep(input, repl_env);
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

malObjectPtr READ(const String& input)
{
    return read_str(input);
}

#define CHECK_ARGS_IS(name, expected) \
    check_args_is(name, expected, argCount)

malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env)
{
    if (const malList* list = DYNAMIC_CAST(malList, ast)) {
        const malSymbol* symbol;
        if ((list->count() > 0) &&
            (symbol = DYNAMIC_CAST(malSymbol, list->item(0)))) {

            String special = symbol->value();
            int argCount = list->count() - 1;

            if (special == "def!") {
                CHECK_ARGS_IS("def!", 2);
                const malSymbol* id = OBJECT_CAST(malSymbol, list->item(1));
                return env->set(id->value(), EVAL(list->item(2), env));
            }

            if (special == "let*") {
                CHECK_ARGS_IS("let*", 2);
                const malSequence* bindings =
                  OBJECT_CAST(malSequence, list->item(1));
                int count = check_args_even("let*", bindings->count());
                malEnvPtr inner(new malEnv(env));
                for (int i = 0; i < count; i += 2) {
                    const malSymbol* var =
                      OBJECT_CAST(malSymbol, bindings->item(i));
                    inner->set(var->value(), EVAL(bindings->item(i+1), inner));
                }
                return EVAL(list->item(2), inner);
            }
        }
    }
    return ast->eval(env);
}

String PRINT(malObjectPtr ast)
{
    return ast->print(true);
}

malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    const malApplicable* handler = DYNAMIC_CAST(malApplicable, op);
    ASSERT(handler != NULL, "\"%s\" is not applicable",
                            op->print(true).c_str());

    return handler->apply(argsBegin, argsEnd, env);
}

malObjectPtr readline(const String& prompt)
{
    String input;
    if (s_readLine.get(prompt, input)) {
        return mal::string(input);
    }
    return mal::nil();
}
