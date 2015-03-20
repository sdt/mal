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
static void makeArgv(malEnvPtr env, int argc, char* argv[]);
static void safe_rep(const String& input, malEnvPtr env);
static malObjectPtr quasiquote(malObjectPtr obj);

int main(int argc, char* argv[])
{
    ReadLine readline("~/.mal-history");
    String prompt = "user> ";
    String input;
    malEnvPtr repl_env(new malEnv);
    install_core(repl_env);
    makeArgv(repl_env, argc - 2, argv + 2);
    if (argc > 1) {
        String filename = escape(argv[1]);
        safe_rep(STRF("(load-file %s)", filename.c_str()), repl_env);
        return 0;
    }
    while (readline.get(prompt, input)) {
        safe_rep(input, repl_env);
    }
    return 0;
}

static void safe_rep(const String& input, malEnvPtr env)
{
    String out;
    try {
        out = rep(input, env);
    }
    catch (malEmptyInputException&) {
        return;
    }
    catch (String& s) {
        out = s;
    };
    std::cout << out << "\n";
}

static void makeArgv(malEnvPtr env, int argc, char* argv[])
{
    malObjectVec args;
    for (int i = 0; i < argc; i++) {
        args.push_back(mal::string(argv[i]));
    }
    env->set("*ARGV*", mal::list(args));
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
        const malList* list = DYNAMIC_CAST(malList, ast);
        if (!list || (list->count() == 0)) {
            return ast->eval(env);
        }

        // From here on down we are evaluating a non-empty list.
        // First handle the special forms.
        if (const malSymbol* symbol = DYNAMIC_CAST(malSymbol, list->item(0))) {
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
                ast = list->item(argCount);
                continue; // TCO
            }

            if (special == "fn*") {
                check_args_is("fn*", 2, argCount);

                const malSequence* bindings =
                    OBJECT_CAST(malSequence, list->item(1));
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

                bool isTrue = EVAL(list->item(1), env)->isTrue();
                if (!isTrue && (argCount == 2)) {
                    return mal::nil();
                }
                ast = list->item(isTrue ? 2 : 3);
                continue; // TCO
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
                ast = list->item(2);
                env = inner;
                continue; // TCO
            }

            if (special == "quasiquote") {
                check_args_is("quasiquote", 1, argCount);
                //TRACE("Before QQ: %s\n", ast->print(true).c_str());
                ast = quasiquote(list->item(1));
                //TRACE("After  QQ: %s\n", ast->print(true).c_str());
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
        if (const malLambda* lambda = DYNAMIC_CAST(malLambda, op)) {
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
    const malApplicable* handler = DYNAMIC_CAST(malApplicable, op);
    ASSERT(handler != NULL, "\"%s\" is not applicable", op->print(true).c_str());

    return handler->apply(argsBegin, argsEnd, env);
}

static bool isSymbol(malObjectPtr obj, const String& text)
{
    const malSymbol* sym = DYNAMIC_CAST(malSymbol, obj);
    return sym && (sym->value() == text);
}

static const malSequence* isPair(malObjectPtr obj)
{
    const malSequence* list = DYNAMIC_CAST(malSequence, obj);
    return list && !list->isEmpty() ? list : NULL;
}

static malObjectPtr quasiquote(malObjectPtr obj)
{
    const malSequence* seq = isPair(obj);
    if (!seq) {
        malObjectVec items;
        items.push_back(mal::symbol("quote"));
        items.push_back(obj);
        return mal::list(items);
    }

    if (isSymbol(seq->item(0), "unquote")) {
        // (qq (uq form)) -> form
        check_args_is("unquote", 1, seq->count() - 1);
        return seq->item(1);
    }

    malObjectVec items;
    const malSequence* innerSeq = isPair(seq->item(0));
    if (innerSeq && isSymbol(innerSeq->item(0), "splice-unquote")) {
        check_args_is("splice-unquote", 1, innerSeq->count() - 1);
        // (qq (sq '(a b c))) -> a b c
        items.push_back(mal::symbol("concat"));
        items.push_back(innerSeq->item(1));
    }
    else {
        // (qq (a b c)) -> (list (qq a) (qq b) (qq c))
        // (qq xs     ) -> (cons (qq (car xs)) (qq (cdr xs)))
        items.push_back(mal::symbol("cons"));
        items.push_back(quasiquote(seq->first()));
    }
    items.push_back(quasiquote(seq->rest()));
    return mal::list(items);
}