#include "MAL.h"

#include "Environment.h"
#include "ReadLine.h"
#include "Types.h"

#include <iostream>
#include <memory>

malObjectPtr READ(const String& input);
String PRINT(malObjectPtr ast);

static void makeArgv(malEnvPtr env, int argc, char* argv[]);
static void safeRep(const String& input, malEnvPtr env);
static malObjectPtr quasiquote(malObjectPtr obj);
static malObjectPtr macroExpand(malObjectPtr obj, malEnvPtr env);
static void installMacros(malEnvPtr env);

static ReadLine s_readLine("~/.mal-history");

int main(int argc, char* argv[])
{
    String prompt = "user> ";
    String input;
    malEnvPtr replEnv(new malEnv);
    installCore(replEnv);
    installMacros(replEnv);
    makeArgv(replEnv, argc - 2, argv + 2);
    if (argc > 1) {
        String filename = escape(argv[1]);
        safeRep(STRF("(load-file %s)", filename.c_str()), replEnv);
        return 0;
    }
    while (s_readLine.get(prompt, input)) {
        safeRep(input, replEnv);
    }
    return 0;
}

static void safeRep(const String& input, malEnvPtr env)
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
    malObjectVec* args = new malObjectVec();
    for (int i = 0; i < argc; i++) {
        args->push_back(mal::string(argv[i]));
    }
    env->set("*ARGV*", mal::list(args));
}

String rep(const String& input, malEnvPtr env)
{
    return PRINT(EVAL(READ(input), env));
}

malObjectPtr READ(const String& input)
{
    return readStr(input);
}

malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env)
{
    while (1) {
        const malList* list = DYNAMIC_CAST(malList, ast);
        if (!list || (list->count() == 0)) {
            return ast->eval(env);
        }

        ast = macroExpand(ast, env);
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
                checkArgsIs("def!", 2, argCount);
                const malSymbol* id = OBJECT_CAST(malSymbol, list->item(1));
                return env->set(id->value(), EVAL(list->item(2), env));
            }

            if (special == "defmacro!") {
                checkArgsIs("defmacro!", 2, argCount);

                const malSymbol* id = OBJECT_CAST(malSymbol, list->item(1));
                malObjectPtr body = EVAL(list->item(2), env);
                const malLambda* lambda = OBJECT_CAST(malLambda, body);
                return env->set(id->value(), mal::macro(*lambda));
            }

            if (special == "do") {
                checkArgsAtLeast("do", 1, argCount);

                for (int i = 1; i < argCount; i++) {
                    EVAL(list->item(i), env);
                }
                ast = list->item(argCount);
                continue; // TCO
            }

            if (special == "fn*") {
                checkArgsIs("fn*", 2, argCount);

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
                checkArgsBetween("if", 2, 3, argCount);

                bool isTrue = EVAL(list->item(1), env)->isTrue();
                if (!isTrue && (argCount == 2)) {
                    return mal::nil();
                }
                ast = list->item(isTrue ? 2 : 3);
                continue; // TCO
            }

            if (special == "let*") {
                checkArgsIs("let*", 2, argCount);
                const malSequence* bindings =
                    OBJECT_CAST(malSequence, list->item(1));
                int count = checkArgsEven("let*", bindings->count());
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
                checkArgsIs("macroexpand", 1, argCount);
                return macroExpand(list->item(1), env);
            }

            if (special == "quasiquote") {
                checkArgsIs("quasiquote", 1, argCount);
                ast = quasiquote(list->item(1));
                continue; // TCO
            }

            if (special == "quote") {
                checkArgsIs("quote", 1, argCount);
                return list->item(1);
            }

            if (special == "try*") {
                checkArgsIs("try*", 2, argCount);
                malObjectPtr tryBody = list->item(1);
                const malList* catchBlock = OBJECT_CAST(malList, list->item(2));

                checkArgsIs("catch*", 2, catchBlock->count() - 1);
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
        std::unique_ptr<malObjectVec> items(list->evalItems(env));
        malObjectPtr op = items->at(0);
        if (const malLambda* lambda = DYNAMIC_CAST(malLambda, op)) {
            ast = lambda->getBody();
            env = lambda->makeEnv(items->begin()+1, items->end());
            continue; // TCO
        }
        else {
            return APPLY(op, items->begin()+1, items->end(), env);
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
        return mal::list(mal::symbol("quote"), obj);
    }

    if (isSymbol(seq->item(0), "unquote")) {
        // (qq (uq form)) -> form
        checkArgsIs("unquote", 1, seq->count() - 1);
        return seq->item(1);
    }

    const malSequence* innerSeq = isPair(seq->item(0));
    if (innerSeq && isSymbol(innerSeq->item(0), "splice-unquote")) {
        checkArgsIs("splice-unquote", 1, innerSeq->count() - 1);
        // (qq (sq '(a b c))) -> a b c
        return mal::list(
            mal::symbol("concat"),
            innerSeq->item(1),
            quasiquote(seq->rest())
        );
    }
    else {
        // (qq (a b c)) -> (list (qq a) (qq b) (qq c))
        // (qq xs     ) -> (cons (qq (car xs)) (qq (cdr xs)))
        return mal::list(
            mal::symbol("cons"),
            quasiquote(seq->first()),
            quasiquote(seq->rest())
        );
    }
}

static const malLambda* isMacroApplication(malObjectPtr obj, malEnvPtr env)
{
    if (const malSequence* seq = isPair(obj)) {
        if (const malSymbol* sym = DYNAMIC_CAST(malSymbol, seq->first())) {
            if (malEnvPtr symEnv = env->find(sym->value())) {
                malObjectPtr value = sym->eval(symEnv);
                if (const malLambda* lambda = DYNAMIC_CAST(malLambda, value)) {
                    return lambda->isMacro() ? lambda : NULL;
                }
            }
        }
    }
    return NULL;
}

static malObjectPtr macroExpand(malObjectPtr obj, malEnvPtr env)
{
    while (const malLambda* macro = isMacroApplication(obj, env)) {
        const malSequence* seq = STATIC_CAST(malSequence, obj);
        obj = macro->apply(seq->begin() + 1, seq->end(), env);
    }
    return obj;
}

static const char* macroTable[] = {
    "(defmacro! cond (fn* (& xs) (if (> (count xs) 0) (list 'if (first xs) (if (> (count xs) 1) (nth xs 1) (throw \"odd number of forms to cond\")) (cons 'cond (rest (rest xs)))))))",
    "(defmacro! or (fn* (& xs) (if (empty? xs) nil (if (= 1 (count xs)) (first xs) `(let* (or_FIXME ~(first xs)) (if or_FIXME or_FIXME (or ~@(rest xs))))))))",
};

static void installMacros(malEnvPtr env)
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
