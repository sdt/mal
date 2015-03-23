#include "ReadLine.h"
#include "String.h"
#include "Types.h"

#include <iostream>

malObjectPtr READ(const String& input);
malObjectPtr EVAL(malObjectPtr ast);
String PRINT(malObjectPtr ast);
String rep(const String& input);

int main(int argc, char* argv[])
{
    ReadLine readline("~/.mal-history");
    String prompt = "user> ";
    String input;
    while (readline.get(prompt, input)) {
        String out;
        try {
            out = rep(input);
        }
        catch (String& s) {
            out = s;
        };
        std::cout << out << "\n";
    }
}

String rep(const String& input)
{
    return PRINT(EVAL(READ(input)));
}

malObjectPtr READ(const String& input)
{
    return readStr(input);
}

malObjectPtr EVAL(malObjectPtr ast)
{
    return ast;
}

String PRINT(malObjectPtr ast)
{
    return ast->print(true);
}

// Adding these just to keep the linker happy
malObjectPtr EVAL(malObjectPtr ast, malEnvPtr env) { return NULL; }
malObjectPtr APPLY(malObjectPtr op, malObjectIter argsBegin, malObjectIter argsEnd, malEnvPtr env) { return NULL; }
