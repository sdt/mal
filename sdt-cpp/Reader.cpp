#include "String.h"
#include "Types.h"

#include <regex>

typedef std::regex              Regex;
typedef std::sregex_iterator    RegexIter;

static const Regex tokenRegex("[\\s,]*(~@|[\\[\\]{}()'`~^@]|\"(?:\\\\.|[^\\\\\"])*\"|;.*|[^\\s\\[\\]{}('\"`,;)]+)");
static const Regex intRegex("^[-+]?\\d+$");

class Tokeniser
{
public:
    Tokeniser(const String& input);

    String peek() const {
        return m_iter->str(1);
    }

    String next() {
        String ret = peek();
        ++m_iter;
        return ret;
    }

    bool eof() const {
        return m_iter == m_end;
    }

private:
    RegexIter   m_iter;
    RegexIter   m_end;
};

Tokeniser::Tokeniser(const String& input)
: m_iter(input.begin(), input.end(), tokenRegex)
, m_end()
{
}

static malObjectPtr read_atom(Tokeniser& tokeniser);
static malObjectPtr read_form(Tokeniser& tokeniser);
static malObjectVec read_list(Tokeniser& tokeniser, const String& end);

malObjectPtr read_str(const String& input)
{
    Tokeniser tokeniser(input);
    return read_form(tokeniser);
}

static malObjectPtr read_form(Tokeniser& tokeniser)
{
    String token = tokeniser.peek();

    if (token == ")") {
        throw STR("Unexpected \")\"");
    }
    if (token == "(") {
        tokeniser.next();
        malObjectVec items = read_list(tokeniser, ")");
        return mal::list(items);
    }
    return read_atom(tokeniser);
}

static malObjectPtr read_atom(Tokeniser& tokeniser)
{
    String token = tokeniser.next();
    if (std::regex_match(token, intRegex)) {
        return mal::integer(token);
    }
    return mal::symbol(token);
}

static malObjectVec read_list(Tokeniser& tokeniser, const String& end)
{
    malObjectVec items;
    while (1) {
        if (tokeniser.eof()) {
            throw STR("Expected \"%s\", got EOF", end.c_str());
        }
        if (tokeniser.peek() == end) {
            tokeniser.next();
            return items;
        }
        items.push_back(read_form(tokeniser));
    }
}
