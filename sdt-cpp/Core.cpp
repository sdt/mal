#include "Debug.h"
#include "Environment.h"
#include "String.h"
#include "Types.h"
#include "Validation.h"

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

extern malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env);
extern malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env);
extern malObjectPtr read_str(const String& input);

extern String rep(const String& input, malEnvPtr env);
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
#define BUILTIN(func, symbol) \
    static malBuiltIn::ApplyFunc builtIn_##func; \
    static HandlerRecord handlerRecord_##func(symbol, builtIn_##func); \
    malObjectPtr builtIn_##func(const String& name, \
        malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)

BUILTIN(ADD, "+")
{
    CHECK_ARGS_IS(2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::integer(lhs->value() + rhs->value());
}

BUILTIN(APPLY, "apply")
{
    CHECK_ARGS_AT_LEAST(2);
    malObjectPtr op = *argsBegin++; // this gets checked in APPLY

    // Copy the first N-1 arguments in.
    malObjectVec args(argsBegin, argsEnd-1);

    // Then append the argument as a list.
    const malList* lastArg = OBJECT_CAST(malList, *(argsEnd-1));
    for (int i = 0; i < lastArg->count(); i++) {
        args.push_back(lastArg->item(i));
    }

    return APPLY(op, args.begin(), args.end(), env->getRoot());
}

BUILTIN(CONCAT, "concat")
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

BUILTIN(CONS, "cons")
{
    CHECK_ARGS_IS(2);
    malObjectPtr first = *argsBegin++;
    ARG(malSequence, rest);

    malObjectVec items(1 + rest->count());
    items[0] = first;
    std::copy(rest->begin(), rest->end(), items.begin() + 1);

    return mal::list(items);
}

BUILTIN(COUNT, "count")
{
    CHECK_ARGS_IS(1);
    if (*argsBegin == mal::nil()) {
        return mal::integer(0);
    }

    ARG(malSequence, seq);
    return mal::integer(seq->count());
}

BUILTIN(DIV, "/")
{
    CHECK_ARGS_IS(2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::integer(lhs->value() / rhs->value());
}

BUILTIN(EMPTY_Q, "empty?")
{
    CHECK_ARGS_IS(1);
    ARG(malSequence, seq);

    return mal::boolean(seq->isEmpty());
}


BUILTIN(EQUALS, "=")
{
    CHECK_ARGS_IS(2);
    const malObject* lhs = (*argsBegin++).ptr();
    const malObject* rhs = (*argsBegin++).ptr();

    return mal::boolean(lhs->isEqualTo(rhs));
}

BUILTIN(EVAL, "eval")
{
    CHECK_ARGS_IS(1);
    return EVAL(*argsBegin, env->getRoot());
}

BUILTIN(FIRST, "first")
{
    CHECK_ARGS_IS(1);
    ARG(malSequence, seq);
    return seq->first();
}

BUILTIN(HASH_MAP, "hash-map")
{
    return mal::hash(argsBegin, argsEnd);
}

BUILTIN(LE, "<=")
{
    CHECK_ARGS_IS(2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::boolean(lhs->value() <= rhs->value());
}

BUILTIN(LIST_Q, "list?")
{
    CHECK_ARGS_IS(1);
    return mal::boolean(DYNAMIC_CAST(malList, *argsBegin));
}

BUILTIN(MUL, "*")
{
    CHECK_ARGS_IS(2);
    ARG(malInteger, lhs);
    ARG(malInteger, rhs);

    return mal::integer(lhs->value() * rhs->value());
}

BUILTIN(NTH, "nth")
{
    CHECK_ARGS_IS(2);
    ARG(malSequence, seq);
    ARG(malInteger,  index);

    int i = index->value();
    ASSERT(i >= 0 && i < seq->count(), "Index out of range");

    return seq->item(i);
}

BUILTIN(PR_STR, "pr-str")
{
    return mal::string(printObjects(argsBegin, argsEnd, " ", true));
}

BUILTIN(PRINTLN, "println")
{
    std::cout << printObjects(argsBegin, argsEnd, " ", false) << "\n";
    return mal::nil();
}

BUILTIN(PRN, "prn")
{
    std::cout << printObjects(argsBegin, argsEnd, " ", true) << "\n";
    return mal::nil();
}

BUILTIN(READ_STRING, "read-string")
{
    CHECK_ARGS_IS(1);
    ARG(malString, str);

    return read_str(str->value());
}

BUILTIN(REST, "rest")
{
    CHECK_ARGS_IS(1);
    ARG(malSequence, seq);
    return seq->rest();
}

BUILTIN(SLURP, "slurp")
{
    CHECK_ARGS_IS(1);
    ARG(malString, filename);

    std::ios_base::openmode openmode =
        std::ios::ate | std::ios::in | std::ios::binary;
    std::ifstream file(filename->value().c_str(), openmode);

    String data;
    data.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    data.append(std::istreambuf_iterator<char>(file.rdbuf()),
                std::istreambuf_iterator<char>());

    return mal::string(data);
}

BUILTIN(STR, "str")
{
    return mal::string(printObjects(argsBegin, argsEnd, "", false));
}

BUILTIN(SUB, "-")
{
    int argCount = CHECK_ARGS_BETWEEN(1, 2);
    ARG(malInteger, lhs);
    if (argCount == 1) {
        return mal::integer(- lhs->value());
    }

    ARG(malInteger, rhs);
    return mal::integer(lhs->value() - rhs->value());
}

BUILTIN(THROW, "throw")
{
    CHECK_ARGS_IS(1);
    throw *argsBegin;
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
