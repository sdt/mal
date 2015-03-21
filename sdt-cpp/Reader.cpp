#include "String.h"
#include "Types.h"
#include "Validation.h"

#include <regex>

typedef std::regex              Regex;
typedef std::sregex_iterator    RegexIter;

static const Regex tokenRegex("[\\s,]*(~@|[\\[\\]{}()'`~^@]|\"(?:\\\\.|[^\\\\\"])*\"|;.*|[^\\s\\[\\]{}('\"`,;)]+)");
static const Regex intRegex("^[-+]?\\d+$");
static const Regex closeRegex("[\\)\\]}]");

class Tokeniser
{
public:
    Tokeniser(const String& input);

    String peek() const {
        ASSERT(!eof(), "Tokeniser reading past EOF in peek");
        return m_iter->str(1);
    }

    String next() {
        ASSERT(!eof(), "Tokeniser reading past EOF in next");
        String ret = peek();
        checkPrefix();
        ++m_iter;
        skipWhitespace();
        return ret;
    }

    bool eof() const {
        return m_iter == m_end;
    }

private:
    void skipWhitespace();
    void checkPrefix();

    RegexIter   m_iter;
    RegexIter   m_end;
};

Tokeniser::Tokeniser(const String& input)
: m_iter(input.begin(), input.end(), tokenRegex)
, m_end()
{
    skipWhitespace();
}

static bool isWhitespace(const String& token)
{
    return token.empty() || (token[0] == ';');
}

void Tokeniser::checkPrefix()
{
    // This is the unmatched portion before the match.
    auto prefix = m_iter->prefix();

    if (prefix.length() == 0) {
        return;
    }

    const String& text = prefix.str();
    if (text == "\"") {
        ASSERT(false, "Expected \", got EOF");
    }
    ASSERT(false, "Unexpected \"%s\"", text.c_str());
}

void Tokeniser::skipWhitespace()
{
    while (!eof() && isWhitespace(peek())) {
        checkPrefix();
        ++m_iter;
    }
}

static malObjectPtr read_atom(Tokeniser& tokeniser);
static malObjectPtr read_form(Tokeniser& tokeniser);
static void read_list(Tokeniser& tokeniser, malObjectVec& items,
                      const String& end);
static malObjectPtr process_macro(Tokeniser& tokeniser, const String& symbol);

malObjectPtr read_str(const String& input)
{
    Tokeniser tokeniser(input);
    if (tokeniser.eof()) {
        throw malEmptyInputException();
    }
    return read_form(tokeniser);
}

static malObjectPtr read_form(Tokeniser& tokeniser)
{
    ASSERT(!tokeniser.eof(), "Expected form, got EOF");
    String token = tokeniser.peek();

    ASSERT(!std::regex_match(token, closeRegex),
            "Unexpected \"%s\"", token.c_str());

    if (token == "(") {
        tokeniser.next();
        malObjectVec items;
        read_list(tokeniser, items, ")");
        return mal::list(items);
    }
    if (token == "[") {
        tokeniser.next();
        malObjectVec items;
        read_list(tokeniser, items, "]");
        return mal::vector(items);
    }
    if (token == "{") {
        tokeniser.next();
        malObjectVec items;
        items.push_back(mal::symbol("hash-map"));
        read_list(tokeniser, items, "}");
        return mal::list(items);
    }
    return read_atom(tokeniser);
}

static malObjectPtr read_atom(Tokeniser& tokeniser)
{
    struct ReaderMacro {
        const char* token;
        const char* symbol;
    };
    ReaderMacro macroTable[] = {
        { "@",   "deref" },
        { "`",   "quasiquote" },
        { "'",   "quote" },
        { "~@",  "splice-unquote" },
        { "~",   "unquote" },
    };
    const ReaderMacro* macroTableEnd = macroTable + ARRAY_SIZE(macroTable);

    struct Constant {
        const char* token;
        malObjectPtr value;
    };
    Constant constTable[] = {
        { "false",  mal::falseObject()  },
        { "nil",    mal::nil()          },
        { "true",   mal::trueObject()   },
    };
    const Constant* constTableEnd = constTable + ARRAY_SIZE(constTable);

    String token = tokeniser.next();
    if (token[0] == '"') {
        return mal::string(unescape(token));
    }
    if (token[0] == ':') {
        return mal::keyword(token);
    }
    if (token == "^") {
        malObjectVec items;
        items.push_back(mal::symbol("with-meta"));
        malObjectPtr meta = read_form(tokeniser);
        malObjectPtr object = read_form(tokeniser);
        items.push_back(object);
        items.push_back(meta);
        return mal::list(items);
    }
    for (Constant* it = constTable; it != constTableEnd; ++it) {
        if (token == it->token) {
            return it->value;
        }
    }
    for (ReaderMacro *it = macroTable; it < macroTableEnd; ++it) {
        if (token == it->token) {
            return process_macro(tokeniser, it->symbol);
        }
    }
    if (std::regex_match(token, intRegex)) {
        return mal::integer(token);
    }
    return mal::symbol(token);
}

static void read_list(Tokeniser& tokeniser, malObjectVec& items,
                      const String& end)
{
    while (1) {
        ASSERT(!tokeniser.eof(), "Expected \"%s\", got EOF", end.c_str());
        if (tokeniser.peek() == end) {
            tokeniser.next();
            return;
        }
        items.push_back(read_form(tokeniser));
    }
}

static malObjectPtr process_macro(Tokeniser& tokeniser, const String& symbol)
{
    malObjectVec items;
    items.push_back(mal::symbol(symbol));
    items.push_back(read_form(tokeniser));
    return mal::list(items);
}
