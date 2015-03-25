#include "ReadLine.h"
#include "String.h"
#include "Types.h"

#include <iostream>

malValuePtr READ(const String& input);
malValuePtr EVAL(malValuePtr ast);
String PRINT(malValuePtr ast);
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

malValuePtr READ(const String& input)
{
    return readStr(input);
}

malValuePtr EVAL(malValuePtr ast)
{
    return ast;
}

String PRINT(malValuePtr ast)
{
    return ast->print(true);
}

// Adding these just to keep the linker happy
malValuePtr EVAL(malValuePtr ast, malEnvPtr env) { return NULL; }
malValuePtr APPLY(malValuePtr op, malValueIter argsBegin, malValueIter argsEnd,
                  malEnvPtr env) { return NULL; }
