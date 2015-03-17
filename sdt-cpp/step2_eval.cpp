#include "Environment.h"
#include "Readline.h"
#include "String.h"
#include "Types.h"

#include <iostream>

malObjectPtr READ(const String& input);
malObjectPtr EVAL(malObjectPtr ast, Environment* env);
String PRINT(malObjectPtr ast);
String rep(const String& input, Environment* env);
malObjectPtr read_str(const String& input);

int main(int argc, char* argv[])
{
    ReadLine readline("~/.mal-history");
    String prompt = "user> ";
    String input;
    Environment repl_env;
    repl_env.set("aaa", mal::integer("123"));
    repl_env.set("bbb", mal::integer("234"));
    while (readline.get(prompt, input)) {
        String out;
        try {
            out = rep(input, &repl_env);
        }
        catch (String& s) {
            out = s;
        };
        std::cout << out << "\n";
    }
}

String rep(const String& input, Environment* env)
{
    return PRINT(EVAL(READ(input), env));
}

malObjectPtr READ(const String& input)
{
    return read_str(input);
}

malObjectPtr EVAL(malObjectPtr ast, Environment* env)
{
    return ast->eval(env);
}

String PRINT(malObjectPtr ast)
{
    return ast->print();
}
