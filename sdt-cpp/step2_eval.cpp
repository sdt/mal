#include "Environment.h"
#include "ReadLine.h"
#include "String.h"
#include "Types.h"

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

malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env)
{
    return ast->eval(env);
}

String PRINT(malObjectPtr ast)
{
    return ast->print();
}

malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env)
{
    malApplicable* handler = dynamic_cast<malApplicable*>(op.ptr());
    if (handler == NULL) {
        throw STR("\"%s\" is not applicable", op->print().c_str());
    }

    return handler->apply(argsBegin, argsEnd, env);
}
