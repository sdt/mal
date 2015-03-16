#include "MAL.h"

#include <iostream>

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

String READ(const String& input)
{
    return input;
}

String EVAL(const String& input)
{
    return input;
}

String PRINT(const String& input)
{
    return input;
}
