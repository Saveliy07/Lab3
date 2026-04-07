#pragma once
#include <string>

enum class lexem_t { OP, NUM, ID, EOEX };

struct Token {
    lexem_t type;
    std::string value;
    double numValue = 0.0;
};

class Lexer {
    std::string expr;
    size_t pos;

    char peek() const;
    char get();

public:
    Lexer(const std::string& expression);
    Token next();
};