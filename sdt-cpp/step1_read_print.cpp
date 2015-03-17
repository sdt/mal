#include "String.h"
#include "Types.h"

#include <iostream>

malObjectPtr READ(const String& input);
malObjectPtr EVAL(malObjectPtr ast);
String PRINT(malObjectPtr ast);
String rep(const String& input);
malObjectPtr read_str(const String& input);

int main(int argc, char* argv[])
{
    String prompt = "user> ";
    while (1) {
        String input;
        std::cout << prompt;
        std::getline(std::cin, input);
        if (std::cin.eof() || std::cin.fail()) {
            break;
        }

        std::cout << rep(input) << "\n";
    }
}

String rep(const String& input)
{
    return PRINT(EVAL(READ(input)));
}

malObjectPtr READ(const String& input)
{
    return read_str(input);
}

malObjectPtr EVAL(malObjectPtr ast)
{
    return ast;
}

String PRINT(malObjectPtr ast)
{
    return ast->print();
}
