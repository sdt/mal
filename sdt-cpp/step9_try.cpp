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
static malObjectPtr macro_expand(malObjectPtr obj, malEnvPtr env);
static void install_macros(malEnvPtr env);

static ReadLine s_readLine("~/.mal-history");

int main(int argc, char* argv[])
{
    String prompt = "user> ";
    String input;
    malEnvPtr repl_env(new malEnv);
    install_core(repl_env);
    install_macros(repl_env);
    makeArgv(repl_env, argc - 2, argv + 2);
    if (argc > 1) {
        String filename = escape(argv[1]);
        safe_rep(STRF("(load-file %s)", filename.c_str()), repl_env);
        return 0;
    }
    while (s_readLine.get(prompt, input)) {
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

        ast = macro_expand(ast, env);
        list = DYNAMIC_CAST(malList, ast);
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

            if (special == "defmacro!") {
                check_args_is("defmacro!", 2, argCount);

                const malSymbol* id = OBJECT_CAST(malSymbol, list->item(1));
                malObjectPtr body = EVAL(list->item(2), env);
                const malLambda* lambda = OBJECT_CAST(malLambda, body);
                return env->set(id->value(), mal::macro(lambda));
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

            if (special == "macroexpand") {
                check_args_is("macroexpand", 1, argCount);
                return macro_expand(list->item(1), env);
            }

            if (special == "quasiquote") {
                check_args_is("quasiquote", 1, argCount);
                ast = quasiquote(list->item(1));
                continue; // TCO
            }

            if (special == "quote") {
                check_args_is("quote", 1, argCount);
                return list->item(1);
            }

            if (special == "try*") {
                check_args_is("try*", 2, argCount);
                malObjectPtr tryBody = list->item(1);
                const malList* catchBlock = OBJECT_CAST(malList, list->item(2));

                check_args_is("catch*", 2, catchBlock->count() - 1);
                ASSERT(OBJECT_CAST(malSymbol,
                    catchBlock->item(0))->value() == "catch*",
                    "catch block must begin with catch*");
                const malSymbol* excSym =
                    OBJECT_CAST(malSymbol, catchBlock->item(1));
                malObjectPtr excVal;

                try {
                    return EVAL(tryBody, env);
                }
                catch(String& s) {
                    excVal = mal::string(s);
                }
                catch(malObjectPtr& o) {
                    excVal = o;
                };

                malEnvPtr innerEnv(new malEnv(env));
                innerEnv->set(excSym->value(), excVal);
                return EVAL(catchBlock->item(2), innerEnv);
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

static const malMacro* isMacroApplication(malObjectPtr obj, malEnvPtr env)
{
    if (const malSequence* seq = isPair(obj)) {
        if (const malSymbol* sym = DYNAMIC_CAST(malSymbol, seq->first())) {
            if ((env = env->find(sym->value())) != NULL) {
                malObjectPtr value = sym->eval(env);
                return DYNAMIC_CAST(malMacro, value);
            }
        }
    }
    return NULL;
}

static malObjectPtr macro_expand(malObjectPtr obj, malEnvPtr env)
{
    while (const malMacro* macro = isMacroApplication(obj, env)) {
        const malSequence* seq = STATIC_CAST(malSequence, obj);
        obj = macro->apply(seq->begin() + 1, seq->end(), env);
    }
    return obj;
}

static const char* macroTable[] = {
    "(defmacro! cond (fn* (& xs) (if (> (count xs) 0) (list 'if (first xs) (if (> (count xs) 1) (nth xs 1) (throw \"odd number of forms to cond\")) (cons 'cond (rest (rest xs)))))))",
    "(defmacro! or (fn* (& xs) (if (empty? xs) nil (if (= 1 (count xs)) (first xs) `(let* (or_FIXME ~(first xs)) (if or_FIXME or_FIXME (or ~@(rest xs))))))))",
};

static void install_macros(malEnvPtr env)
{
    for (int i = 0; i < ARRAY_SIZE(macroTable); i++) {
        rep(macroTable[i], env);
    }
}

malObjectPtr readline(const String& prompt)
{
    String input;
    if (s_readLine.get(prompt, input)) {
        return mal::string(input);
    }
    return mal::nil();
}
