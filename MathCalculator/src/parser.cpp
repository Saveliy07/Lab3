#include "parser.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>

using namespace std;

void Parser::next() { curr = lexer.next(); }

bool Parser::match(char op) {
    if (curr.type == lexem_t::OP && curr.value[0] == op) { next(); return true; }
    return false;
}

bool Parser::isReservedFunction(const string& name) {
    static const vector<string> funcs = {"sin", "cos", "tan", "asin", "acos", "atan", "exp", "log", "sqrt"};
    return find(funcs.begin(), funcs.end(), name) != funcs.end();
}

Parser::Parser(const string& expr) : lexer(expr) { next(); }

unique_ptr<Node> Parser::parse() {
    auto node = parseExpression();
    if (curr.type != lexem_t::EOEX) throw runtime_error("ERROR: unexpected token at end");
    return node;
}

unique_ptr<Node> Parser::parseExpression() {
    auto node = parseTerm();
    while (curr.type == lexem_t::OP && (curr.value == "+" || curr.value == "-")) {
        char op = curr.value[0]; next();
        node = make_unique<BinaryNode>(op, move(node), parseTerm());
    }
    return node;
}

unique_ptr<Node> Parser::parseTerm() {
    auto node = parseUnary();
    while (curr.type == lexem_t::OP && (curr.value == "*" || curr.value == "/")) {
        char op = curr.value[0]; next();
        node = make_unique<BinaryNode>(op, move(node), parseUnary());
    }
    return node;
}

unique_ptr<Node> Parser::parseUnary() {
    if (curr.type == lexem_t::OP && (curr.value == "+" || curr.value == "-")) {
        char op = curr.value[0]; next();
        return make_unique<UnaryNode>(op, parseUnary());
    }
    return parseFactor();
}

unique_ptr<Node> Parser::parseFactor() {
    auto node = parsePrimary();
    if (curr.type == lexem_t::OP && curr.value == "^") {
        next();
        node = make_unique<BinaryNode>('^', move(node), parseUnary()); 
    }
    return node;
}

unique_ptr<Node> Parser::parsePrimary() {
    if (curr.type == lexem_t::NUM) {
        auto node = make_unique<NumNode>(curr.numValue); next(); return node;
    }
    if (curr.type == lexem_t::ID) {
        string id = curr.value; next();
        
        bool is_func = isReservedFunction(id);
        
        if (is_func && (curr.type != lexem_t::OP || curr.value != "(")) {
            throw runtime_error("ERROR: Function " + id + " must be followed by '('");
        }

        if (curr.type == lexem_t::OP && curr.value == "(") {
            next(); 
            
            if (curr.type == lexem_t::OP && curr.value == ")") {
                 throw runtime_error("ERROR: Empty parentheses");
            }
            
            auto arg = parseExpression();
            if (!match(')')) throw runtime_error("ERROR: Expected )");
            
            if (is_func) {
                return make_unique<FuncNode>(id, move(arg));
            } else {
                throw runtime_error("ERROR Unknown function: " + id);
            }
        }
        return make_unique<VarNode>(id);
    }
    if (curr.type == lexem_t::OP && curr.value == "(") {
        next();
        if (curr.type == lexem_t::OP && curr.value == ")") {
             throw runtime_error("ERROR: Empty parentheses");
        }
        auto node = parseExpression();
        if (!match(')')) throw runtime_error("ERROR: Expected )");
        return node;
    }
    throw runtime_error("ERROR: Syntax error in primary");
}