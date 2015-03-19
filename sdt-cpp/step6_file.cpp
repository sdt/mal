#include "Environment.h"
#include "ReadLine.h"
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
malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env);

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
    while (1) {
        malList* list = DYNAMIC_CAST(malList, ast);
        if (!list || (list->count() == 0)) {
            return ast->eval(env);
        }

        // From here on down we are evaluating a non-empty list.
        // First handle the special forms.
        if (malSymbol* symbol = DYNAMIC_CAST(malSymbol, list->item(0))) {
            String special = symbol->value();
            int argCount = list->count() - 1;

            if (special == "def!") {
                check_args_is("def!", 2, argCount);
                malSymbol* id = OBJECT_CAST(malSymbol, list->item(1));
                return env->set(id->value(), EVAL(list->item(2), env));
            }

            if (special == "do") {
                check_args_at_least("do", 1, argCount);

                for (int i = 1; i < argCount; i++) {
                    EVAL(list->item(i), env);
                }
                ast = list->item(argCount);
                continue; // TCO
            }

            if (special == "fn*") {
                check_args_is("fn*", 2, argCount);

                malSequence* bindings = OBJECT_CAST(malSequence, list->item(1));
                StringVec params;
                for (int i = 0; i < bindings->count(); i++) {
                    malSymbol* sym = OBJECT_CAST(malSymbol, bindings->item(i));
                    params.push_back(sym->value());
                }

                return mal::lambda(params, list->item(2), env);
            }

            if (special == "if") {
                check_args_between("if", 2, 3, argCount);

                malObjectPtr cond = EVAL(list->item(1), env);
                bool isTrue = (cond != mal::falseObject())
                            && (cond != mal::nil());

                if (!isTrue && (argCount == 2)) {
                    return mal::nil();
                }
                ast = list->item(isTrue ? 2 : 3);
                continue; // TCO
            }

            if (special == "let*") {
                check_args_is("let*", 2, argCount);
                malSequence* bindings = OBJECT_CAST(malSequence, list->item(1));
                int count = check_args_even("let*", bindings->count());
                malEnvPtr inner(new malEnv(env));
                for (int i = 0; i < count; i += 2) {
                    malSymbol* var = OBJECT_CAST(malSymbol, bindings->item(i));
                    inner->set(var->value(), EVAL(bindings->item(i+1), inner));
                }
                ast = list->item(2);
                env = inner;
                continue; // TCO
            }

            if (special == "quote") {
                check_args_is("quote", 1, argCount);
                return list->item(1);
            }
        }

        // Now we're left with the case of a regular list to be evaluated.
        malObjectVec items = list->eval_items(env);
        malObjectPtr op = items[0];
        if (malLambda* lambda = DYNAMIC_CAST(malLambda, op)) {
            ast = lambda->getBody();
            env = lambda->makeEnv(items.begin()+1, items.end());
            continue; // TCO
        }
        else {
            return APPLY(op, items.begin()+1, items.end(), env);
        }
    }
}

String PRINT(malObjectPtr ast)
{
    return ast->print(true);
}

malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    malApplicable* handler = DYNAMIC_CAST(malApplicable, op);
    ASSERT(handler != NULL, "\"%s\" is not applicable", op->print(true).c_str());

    return handler->apply(argsBegin, argsEnd, env);
}