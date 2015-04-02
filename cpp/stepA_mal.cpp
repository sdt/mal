#include "MAL.h"

#include "Environment.h"
#include "ReadLine.h"
#include "Types.h"

#include <iostream>
#include <memory>

malValuePtr READ(const String& input);
String PRINT(malValuePtr ast);
static void installFunctions(malEnvPtr env);

static void makeArgv(malEnvPtr env, int argc, char* argv[]);
static void safeRep(const String& input, malEnvPtr env);
static malValuePtr quasiquote(malValuePtr obj);
static malValuePtr macroExpand(malValuePtr obj, malEnvPtr env);
static void installMacros(malEnvPtr env);

static ReadLine s_readLine("~/.mal-history");

int main(int argc, char* argv[])
{
    String prompt = "user> ";
    String input;
    malEnvPtr replEnv(new malEnv);
    installCore(replEnv);
    installFunctions(replEnv);
    installMacros(replEnv);
    makeArgv(replEnv, argc - 2, argv + 2);
    if (argc > 1) {
        String filename = escape(argv[1]);
        safeRep(STRF("(load-file %s)", filename.c_str()), replEnv);
        return 0;
    }
#ifdef DEBUG_MEMORY_AUDITING
    //RefCounted::memoryReport(replEnv.ptr());
#endif
    while (s_readLine.get(prompt, input)) {
        safeRep(input, replEnv);
#ifdef DEBUG_MEMORY_AUDITING
        RefCounted::memoryReport(replEnv.ptr());
#endif
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
    malValueVec* args = new malValueVec();
    for (int i = 0; i < argc; i++) {
        args->push_back(mal::string(argv[i]));
    }
    env->set("*ARGV*", mal::list(args));
}

String rep(const String& input, malEnvPtr env)
{
    return PRINT(EVAL(READ(input), env));
}

malValuePtr READ(const String& input)
{
    return readStr(input);
}

#define ARGS(sym, expected)  \
            malValuePtr arg[expected]; \
            argList = shiftArgs(sym, argList, expected, expected, arg);

#define ARGS_BETWEEN(sym, min, max)  \
            malValuePtr arg[max]; \
            argList = shiftArgs(sym, argList, min, max, arg);

#define ARGS_AT_LEAST(sym, min) \
            malValuePtr arg[min]; \
            argList = shiftArgs(sym, argList, min, -1, arg);

malValuePtr EVAL(malValuePtr ast, malEnvPtr env)
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
        if (const malSymbol* symbol = DYNAMIC_CAST(malSymbol, list->first())) {
            String special = symbol->value();
            malValuePtr argList = list->rest();

            if (special == "def!") {
                ARGS("def!", 2);

                const malSymbol* id = VALUE_CAST(malSymbol, arg[0]);
                return env->set(id->value(), EVAL(arg[1], env));
            }

            if (special == "defmacro!") {
                ARGS("defmacro!", 2);

                const malSymbol* id = VALUE_CAST(malSymbol, arg[0]);
                malValuePtr body = EVAL(arg[1], env);
                const malLambda* lambda = VALUE_CAST(malLambda, body);
                return env->set(id->value(), mal::macro(*lambda));
            }

            if (special == "do") {
                ARGS_AT_LEAST("do", 1);

                malList* list = VALUE_CAST(malList, argList);
                while (!list->isEmpty()) {
                    EVAL(arg[0], env);
                    arg[0] = list->first();
                    argList = list->rest();
                    list = VALUE_CAST(malList, argList);
                }
                ast = arg[0];
                continue; // TCO
            }

            if (special == "fn*") {
                ARGS("fn*", 2);

                //LIST: convert this to malVector
                const malSequence* bindings = VALUE_CAST(malSequence, arg[0]);
                StringVec params;
                for (int i = 0; i < bindings->count(); i++) {
                    const malSymbol* sym =
                        VALUE_CAST(malSymbol, bindings->item(i));
                    params.push_back(sym->value());
                }

                return mal::lambda(params, arg[1], env);
            }

            if (special == "if") {
                ARGS_BETWEEN("if", 2, 3);

                bool isTrue = EVAL(arg[0], env)->isTrue();
                if (!isTrue && !arg[2]) {
                    return mal::nilValue();
                }
                ast = arg[isTrue ? 1 : 2];
                continue; // TCO
            }


            if (special == "let*") {
                ARGS("let*", 2);

                //LIST: convert this to malVector
                const malSequence* bindings = VALUE_CAST(malSequence, arg[0]);
                int count = checkArgsEven("let*", bindings->count());
                malEnvPtr inner(new malEnv(env));
                for (int i = 0; i < count; i += 2) {
                    const malSymbol* var =
                        VALUE_CAST(malSymbol, bindings->item(i));
                    inner->set(var->value(), EVAL(bindings->item(i+1), inner));
                }
                ast = arg[1];
                env = inner;
                continue; // TCO
            }

            if (special == "macroexpand") {
                ARGS("macroexpand", 1);

                return macroExpand(arg[0], env);
            }

            if (special == "quasiquote") {
                ARGS("quasiquote", 1);

                ast = quasiquote(arg[0]);
                continue; // TCO
            }

            if (special == "quote") {
                ARGS("quote", 1);

                return arg[0];
            }

            if (special == "try*") {
                ARGS("try*", 2);

                malValuePtr tryBody = list->item(1);
                const malList* catchBlock = VALUE_CAST(malList, list->item(2));

                checkArgsIs("catch*", 2, catchBlock->count() - 1);
                MAL_CHECK(VALUE_CAST(malSymbol,
                    catchBlock->item(0))->value() == "catch*",
                    "catch block must begin with catch*");

                // We don't need excSym at this scope, but we want to check
                // that the catch block is valid always, not just in case of
                // an exception.
                const malSymbol* excSym =
                    VALUE_CAST(malSymbol, catchBlock->item(1));

                malValuePtr excVal;

                try {
                    ast = EVAL(tryBody, env);
                }
                catch(String& s) {
                    excVal = mal::string(s);
                }
                catch (malEmptyInputException&) {
                    // Not an error, continue as if we got nil
                    ast = mal::nilValue();
                }
                catch(malValuePtr& o) {
                    excVal = o;
                };

                if (excVal) {
                    // we got some exception
                    env = malEnvPtr(new malEnv(env));
                    env->set(excSym->value(), excVal);
                    ast = catchBlock->item(2);
                }
                continue; // TCO
            }
        }

        // Now we're left with the case of a regular list to be evaluated.
        std::unique_ptr<malValueVec> items(list->evalItems(env));
        malValuePtr op = items->at(0);
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

String PRINT(malValuePtr ast)
{
    return ast->print(true);
}

malValuePtr APPLY(malValuePtr op, malValueIter argsBegin, malValueIter argsEnd,
                  malEnvPtr env)
{
    const malApplicable* handler = DYNAMIC_CAST(malApplicable, op);
    MAL_CHECK(handler != NULL,
              "\"%s\" is not applicable", op->print(true).c_str());

    return handler->apply(argsBegin, argsEnd, env);
}

static bool isSymbol(malValuePtr obj, const String& text)
{
    const malSymbol* sym = DYNAMIC_CAST(malSymbol, obj);
    return sym && (sym->value() == text);
}

static malValuePtr isPair(malValuePtr obj)
{
    const malSequence* seq = DYNAMIC_CAST(malSequence, obj);
    return seq && !seq->isEmpty() ? seq->toList : NULL;
}

static malValuePtr quasiquote(malValuePtr obj)
{
    malValuePtr seq = isPair(obj);
    if (!seq) {
        return mal::list(mal::symbol("quote"), obj);
    }

    malList* list = STATIC_CAST(malList, seq);
    if (isSymbol(list->item(0), "unquote")) {
        // (qq (uq form)) -> form
        checkArgsIs("unquote", 1, list->count() - 1);
        return list->item(1);
    }

    const malList* innerList = isPair(seq->item(0));
    malValuePtr innerSeq = isPair(seq->
    if (innerList && isSymbol(innerList->item(0), "splice-unquote")) {
        checkArgsIs("splice-unquote", 1, innerList->count() - 1);
        // (qq (sq '(a b c))) -> a b c
        return mal::list(
            mal::symbol("concat"),
            innerList->item(1),
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

static const malLambda* isMacroApplication(malValuePtr obj, malEnvPtr env)
{
    if (const malSequence* seq = isPair(obj)) {
        if (malSymbol* sym = DYNAMIC_CAST(malSymbol, seq->first())) {
            if (malEnvPtr symEnv = env->find(sym->value())) {
                malValuePtr value = sym->eval(symEnv);
                if (malLambda* lambda = DYNAMIC_CAST(malLambda, value)) {
                    return lambda->isMacro() ? lambda : NULL;
                }
            }
        }
    }
    return NULL;
}

static malValuePtr macroExpand(malValuePtr obj, malEnvPtr env)
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
    for (auto &macro : macroTable) {
        rep(macro, env);
    }
}

malValuePtr readline(const String& prompt)
{
    String input;
    if (s_readLine.get(prompt, input)) {
        return mal::string(input);
    }
    return mal::nilValue();
}

static const char* malFunctionTable[] = {
    "(def! list (fn* (& items) items))",
    "(def! not (fn* (cond) (if cond false true)))",
    "(def! >= (fn* (a b) (<= b a)))",
    "(def! < (fn* (a b) (not (<= b a))))",
    "(def! > (fn* (a b) (not (<= a b))))",
    "(def! load-file (fn* (filename) \
        (eval (read-string (str \"(do \" (slurp filename) \")\")))))",
    "(def! map (fn* (f xs) (if (empty? xs) xs \
        (cons (f (first xs)) (map f (rest xs))))))",
    "(def! swap! (fn* (atom f & args) (reset! atom (apply f @atom args))))",
    "(def! *host-language* \"c++\")",
};

static void installFunctions(malEnvPtr env) {
    for (auto &function : malFunctionTable) {
        rep(function, env);
    }
}
