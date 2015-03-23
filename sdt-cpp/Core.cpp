#include "MAL.h"
#include "Environment.h"
#include "Types.h"

#include <fstream>
#include <iostream>

#define CHECK_ARGS_IS(expected) \
    check_args_is(name.c_str(), expected, \
                  std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_BETWEEN(min, max) \
    check_args_between(name.c_str(), min, max, \
                       std::distance(argsBegin, argsEnd))

#define CHECK_ARGS_AT_LEAST(expected) \
    check_args_at_least(name.c_str(), expected, \
                        std::distance(argsBegin, argsEnd))

static String printObjects(malObjectIter begin, malObjectIter end,
                           const String& sep, bool readably);

class HandlerRecord {
public:
    HandlerRecord(const char* name, malBuiltIn::ApplyFunc * handler)
    : name(name), handler(handler), next(first)
    {
        first = this;
    }

    static void installAll(malEnvPtr env) {
        for (HandlerRecord* it = first; it != NULL; it = it->next) {
            env->set(it->name, mal::builtin(it->name, it->handler));
        }
    }

private:
    const char* name;
    malBuiltIn::ApplyFunc* handler;
    static HandlerRecord* first;
    HandlerRecord* next;
};

HandlerRecord* HandlerRecord::first = NULL;

#define ARG(type, name) const type* name = OBJECT_CAST(type, *argsBegin++)

#define FUNCNAME(uniq) builtIn ## uniq
#define HRECNAME(uniq) handler ## uniq
#define BUILTIN_DEF(uniq, symbol) \
    static malBuiltIn::ApplyFunc FUNCNAME(uniq); \
    static HandlerRecord HRECNAME(uniq)(symbol, FUNCNAME(uniq)); \
    malObjectPtr FUNCNAME(uniq)(const String& name, \
        malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)

#define BUILTIN(symbol)  BUILTIN_DEF(__LINE__, symbol)

#define BUILTIN_ISA(symbol, type) \
    BUILTIN(symbol) { \
        CHECK_ARGS_IS(1); \
        return mal::boolean(DYNAMIC_CAST(type, *argsBegin)); \
    }

#define BUILTIN_IS(op, constant) \
    BUILTIN(op) { \
        CHECK_ARGS_IS(1); \
        return mal::boolean(*argsBegin == mal::constant()); \
    }

#define BUILTIN_INTOP(op, checkDivByZero) \
    BUILTIN(#op) { \
        CHECK_ARGS_IS(2); \
        ARG(malInteger, lhs); \
        ARG(malInteger, rhs); \
        if (checkDivByZero) { \
            ASSERT(rhs->value() != 0, "Division by zero"); \
        } \
        return mal::integer(lhs->value() op rhs->value()); \
    }

BUILTIN_ISA("atom?",                malAtom);
BUILTIN_ISA("keyword?",             malKeyword);
BUILTIN_ISA("list?",                malList);
BUILTIN_ISA("map?",                 malHash);
BUILTIN_ISA("sequential?",          malSequence);
BUILTIN_ISA("symbol?",              malSymbol);
BUILTIN_ISA("vector?",              malVector);

BUILTIN_INTOP(+, false);
BUILTIN_INTOP(/, true);
BUILTIN_INTOP(*, false);
BUILTIN_INTOP(%, true);

BUILTIN_IS("true?",     trueObject);
BUILTIN_IS("false?",    falseObject);
BUILTIN_IS("nil?",      nil);

BUILTIN("-")
{
    int argCount = CHECK_ARGS_BETWEEN(1, 2);
    ARG(malInteger, lhs);
    if (argCount == 1) {
        return mal::integer(- lhs->value());
    }

    ARG(malInteger, rhs);
    return mal::integer(lhs->value() - rhs->value());
}

BUILTIN("<=")
{
    CHECK_ARGS_IS(2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::boolean(lhs->value() <= rhs->value());
}

BUILTIN("=")
{
    CHECK_ARGS_IS(2);
    const malObject* lhs = (*argsBegin++).ptr();
    const malObject* rhs = (*argsBegin++).ptr();

    return mal::boolean(lhs->isEqualTo(rhs));
}

BUILTIN("apply")
{
    CHECK_ARGS_AT_LEAST(2);
    malObjectPtr op = *argsBegin++; // this gets checked in APPLY

    // Copy the first N-1 arguments in.
    malObjectVec args(argsBegin, argsEnd-1);

    // Then append the argument as a list.
    const malSequence* lastArg = OBJECT_CAST(malSequence, *(argsEnd-1));
    for (int i = 0; i < lastArg->count(); i++) {
        args.push_back(lastArg->item(i));
    }

    return APPLY(op, args.begin(), args.end(), env->getRoot());
}

BUILTIN("assoc")
{
    CHECK_ARGS_AT_LEAST(1);
    ARG(malHash, hash);

    return hash->assoc(argsBegin, argsEnd);
}

BUILTIN("atom")
{
    CHECK_ARGS_IS(1);

    return mal::atom(*argsBegin);
}

BUILTIN("concat")
{
    int count = 0;
    for (auto it = argsBegin; it != argsEnd; ++it) {
        const malSequence* seq = OBJECT_CAST(malSequence, *it);
        count += seq->count();
    }

    malObjectVec items(count);
    int offset = 0;
    for (auto it = argsBegin; it != argsEnd; ++it) {
        const malSequence* seq = STATIC_CAST(malSequence, *it);
        std::copy(seq->begin(), seq->end(), items.begin() + offset);
        offset += seq->count();
    }

    return mal::list(items);
}

BUILTIN("conj")
{
    CHECK_ARGS_AT_LEAST(1);
    ARG(malSequence, seq);

    return seq->conj(argsBegin, argsEnd);
}

BUILTIN("cons")
{
    CHECK_ARGS_IS(2);
    malObjectPtr first = *argsBegin++;
    ARG(malSequence, rest);

    malObjectVec items(1 + rest->count());
    items[0] = first;
    std::copy(rest->begin(), rest->end(), items.begin() + 1);

    return mal::list(items);
}

BUILTIN("contains?")
{
    CHECK_ARGS_IS(2);
    if (*argsBegin == mal::nil()) {
        return *argsBegin;
    }
    ARG(malHash, hash);
    return mal::boolean(hash->contains(*argsBegin));
}

BUILTIN("count")
{
    CHECK_ARGS_IS(1);
    if (*argsBegin == mal::nil()) {
        return mal::integer(0);
    }

    ARG(malSequence, seq);
    return mal::integer(seq->count());
}

BUILTIN("deref")
{
    CHECK_ARGS_IS(1);
    ARG(malAtom, atom);

    return atom->deref();
}

BUILTIN("dissoc")
{
    CHECK_ARGS_AT_LEAST(1);
    ARG(malHash, hash);

    return hash->dissoc(argsBegin, argsEnd);
}

BUILTIN("empty?")
{
    CHECK_ARGS_IS(1);
    ARG(malSequence, seq);

    return mal::boolean(seq->isEmpty());
}

BUILTIN("eval")
{
    CHECK_ARGS_IS(1);
    return EVAL(*argsBegin, env->getRoot());
}

BUILTIN("first")
{
    CHECK_ARGS_IS(1);
    ARG(malSequence, seq);
    return seq->first();
}

BUILTIN("get")
{
    CHECK_ARGS_IS(2);
    if (*argsBegin == mal::nil()) {
        return *argsBegin;
    }
    ARG(malHash, hash);
    return hash->get(*argsBegin);
}

BUILTIN("hash-map")
{
    return mal::hash(argsBegin, argsEnd);
}

BUILTIN("keys")
{
    CHECK_ARGS_IS(1);
    ARG(malHash, hash);
    return hash->keys();
}

BUILTIN("keyword")
{
    CHECK_ARGS_IS(1);
    ARG(malString, token);
    return mal::keyword(":" + token->value());
}

BUILTIN("meta")
{
    CHECK_ARGS_IS(1);
    malObjectPtr obj = *argsBegin++;

    return obj->meta();
}

BUILTIN("nth")
{
    CHECK_ARGS_IS(2);
    ARG(malSequence, seq);
    ARG(malInteger,  index);

    int i = index->value();
    ASSERT(i >= 0 && i < seq->count(), "Index out of range");

    return seq->item(i);
}

BUILTIN("pr-str")
{
    return mal::string(printObjects(argsBegin, argsEnd, " ", true));
}

BUILTIN("println")
{
    std::cout << printObjects(argsBegin, argsEnd, " ", false) << "\n";
    return mal::nil();
}

BUILTIN("prn")
{
    std::cout << printObjects(argsBegin, argsEnd, " ", true) << "\n";
    return mal::nil();
}

BUILTIN("read-string")
{
    CHECK_ARGS_IS(1);
    ARG(malString, str);

    return read_str(str->value());
}

BUILTIN("readline")
{
    CHECK_ARGS_IS(1);
    ARG(malString, str);

    return readline(str->value());
}

BUILTIN("reset!")
{
    CHECK_ARGS_IS(2);
    ARG(malAtom, atom);
    return atom->reset(*argsBegin);
}

BUILTIN("rest")
{
    CHECK_ARGS_IS(1);
    ARG(malSequence, seq);
    return seq->rest();
}

BUILTIN("slurp")
{
    CHECK_ARGS_IS(1);
    ARG(malString, filename);

    std::ios_base::openmode openmode =
        std::ios::ate | std::ios::in | std::ios::binary;
    std::ifstream file(filename->value().c_str(), openmode);
    ASSERT(!file.fail(), "Cannot open %s", filename->value().c_str());

    String data;
    data.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    data.append(std::istreambuf_iterator<char>(file.rdbuf()),
                std::istreambuf_iterator<char>());

    return mal::string(data);
}

BUILTIN("str")
{
    return mal::string(printObjects(argsBegin, argsEnd, "", false));
}

BUILTIN("symbol")
{
    CHECK_ARGS_IS(1);
    ARG(malString, token);
    return mal::symbol(token->value());
}

BUILTIN("throw")
{
    CHECK_ARGS_IS(1);
    throw *argsBegin;
}

BUILTIN("vals")
{
    CHECK_ARGS_IS(1);
    ARG(malHash, hash);
    return hash->values();
}

BUILTIN("vector")
{
    return mal::vector(argsBegin, argsEnd);
}

BUILTIN("with-meta")
{
    CHECK_ARGS_IS(2);
    malObjectPtr obj  = *argsBegin++;
    malObjectPtr meta = *argsBegin++;
    return obj->withMeta(meta);
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
    "(def! *host-language* \"sdt-cpp\")",
};

void install_core(malEnvPtr env) {
    HandlerRecord::installAll(env);

    for (int i = 0; i < ARRAY_SIZE(malFunctionTable); i++) {
        rep(malFunctionTable[i], env);
    }
}

static String printObjects(malObjectIter begin, malObjectIter end,
                           const String& sep, bool readably)
{
    String out;

    if (begin != end) {
        out += (*begin)->print(readably);
        ++begin;
    }

    for ( ; begin != end; ++begin) {
        out += sep;
        out += (*begin)->print(readably);
    }

    return out;
}
