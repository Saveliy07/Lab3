#include "lexer.hpp"
#include <cctype>
#include <stdexcept>

using namespace std;

Lexer::Lexer(const string& expression) : expr(expression), pos(0) {}

char Lexer::peek() const { return pos < expr.length() ? expr[pos] : '\0'; }
char Lexer::get() { return pos < expr.length() ? expr[pos++] : '\0'; }

Token Lexer::next() {
    while (isspace(peek())) get();
    char c = peek();
    if (c == '\0') return {lexem_t::EOEX, "", 0.0};

    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '(' || c == ')') {
        return {lexem_t::OP, string(1, get()), 0.0};
    }

    if (isdigit(c)) {
        string num;
        bool has_dot = false;
        while (isdigit(peek()) || peek() == '.') {
            if (peek() == '.') {
                if (has_dot) throw runtime_error("ERROR: multiple dots in a number");
                has_dot = true;
            }
            num += get();
        }
        if (num.length() > 1 && num[0] == '0' && num[1] != '.') throw runtime_error("ERROR: leading zeros are not allowed");
        if (!num.empty() && num.back() == '.') throw runtime_error("ERROR: number cannot end with a dot");
        if (isalpha(peek()) || peek() == '_') throw runtime_error("ERROR: number cannot be directly followed by a letter");
        
        return {lexem_t::NUM, num, stod(num)};
    }

    if (isalpha(c) || c == '_') {
        string id;
        while (isalnum(peek()) || peek() == '_') id += tolower(get());
        return {lexem_t::ID, id, 0.0};
    }

    throw runtime_error(string("ERROR: unknown character '") + c + "'");
}