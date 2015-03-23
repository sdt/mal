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
        catch (malEmptyInputException&) {
            continue;
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


malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env)
{
    if (const malList* list = DYNAMIC_CAST(malList, ast)) {
        const malSymbol* symbol;
        if ((list->count() > 0) &&
            (symbol = DYNAMIC_CAST(malSymbol, list->item(0)))) {

            String special = symbol->value();
            int argCount = list->count() - 1;

            if (special == "def!") {
                check_args_is("def!", 2, argCount);
                const malSymbol* id = OBJECT_CAST(malSymbol, list->item(1));
                return env->set(id->value(), EVAL(list->item(2), env));
            }

            if (special == "do") {
                check_args_at_least("do", 1, argCount);

                for (int i = 1; i < argCount; i++) {
                    EVAL(list->item(i), env);
                }
                return EVAL(list->item(argCount), env);
            }

            if (special == "fn*") {
                check_args_is("fn*", 2, argCount);

                const malList* bindings = OBJECT_CAST(malList, list->item(1));
                StringVec params;
                for (int i = 0; i < bindings->count(); i++) {
                    const malSymbol* sym =
                      OBJECT_CAST(malSymbol, bindings->item(i));
                    params.push_back(sym->value());
                }

                return mal::lambda(params, list->item(2), env);
            }

            if (special == "if") {
                check_args_between("if", 2, 3, argCount);

                malObjectPtr cond = EVAL(list->item(1), env);
                if (cond->isTrue()) {
                    // then
                    return EVAL(list->item(2), env);
                }
                if (argCount == 2) {
                    // no else case
                    return mal::nil();
                }
                // else case
                return EVAL(list->item(3), env);
            }

            if (special == "let*") {
                check_args_is("let*", 2, argCount);
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

            if (special == "quote") {
                check_args_is("quote", 1, argCount);
                return list->item(1);
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
