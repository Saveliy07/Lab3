#pragma once
#include "lexer.hpp"
#include "ast.hpp"
#include <string>
#include <memory>

class Parser {
    Lexer lexer;
    Token curr;

    void next();
    bool match(char op);

    std::unique_ptr<Node> parseExpression();
    std::unique_ptr<Node> parseTerm();
    std::unique_ptr<Node> parseUnary();
    std::unique_ptr<Node> parseFactor();
    std::unique_ptr<Node> parsePrimary();

public:
    static bool isReservedFunction(const std::string& name);
    Parser(const std::string& expr);
    std::unique_ptr<Node> parse();
};