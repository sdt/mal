#include "Environment.h"
#include "Readline.h"
#include "String.h"
#include "Types.h"
#include "Validation.h"

#include <iostream>

malObjectPtr READ(const String& input);
malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env);
String PRINT(malObjectPtr ast);
String rep(const String& input, malEnvPtr env);
malObjectPtr read_str(const String& input);
void install_core(malEnvPtr env);

int main(int argc, char* argv[])
{
    ReadLine readline("~/.mal-history");
    String prompt = "user> ";
    String input;
    malEnvPtr repl_env(new malEnv);
    install_core(repl_env);
    while (readline.get(prompt, input)) {
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

#define CHECK_ARGS_COUNT(name, expected) \
    check_args_count(name, expected, argCount)

malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env)
{
    if (malList* list = DYNAMIC_CAST(malList, ast)) {
        malSymbol* symbol;
        if ((list->count() > 0) &&
            (symbol = DYNAMIC_CAST(malSymbol, list->item(0)))) {

            String special = symbol->value();
            int argCount = list->count() - 1;

            if (special == "def!") {
                CHECK_ARGS_COUNT("def!", 2);
                malSymbol* id = OBJECT_CAST(malSymbol, list->item(1));
                return env->set(id->value(), EVAL(list->item(2), env));
            }

            if (special == "let*") {
                CHECK_ARGS_COUNT("let*", 2);
                malSequence* bindings = OBJECT_CAST(malSequence, list->item(1));
                int count = check_args_even("let*", bindings->count());
                malEnvPtr inner(new malEnv(env));
                for (int i = 0; i < count; i += 2) {
                    malSymbol* var = DYNAMIC_CAST(malSymbol, bindings->item(i));
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
    return ast->print();
}

malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    malApplicable* handler = DYNAMIC_CAST(malApplicable, op);
    if (handler == NULL) {
        throw STR("\"%s\" is not applicable", op->print().c_str());
    }

    return handler->apply(argsBegin, argsEnd, env);
}
