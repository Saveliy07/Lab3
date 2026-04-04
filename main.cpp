#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <memory>
#include <algorithm>

using namespace std;

// ================= ФОРМАТИРОВАНИЕ ЧИСЕЛ =================
string formatDouble(double v) {
    string s = to_string(v);
    s.erase(s.find_last_not_of('0') + 1, string::npos);
    if (!s.empty() && s.back() == '.') s.pop_back();
    return s;
}

// ================= ЛЕКСЕР (LEXER) =================
enum class lexem_t { OP, NUM, ID, EOEX };

struct Token {
    lexem_t type;
    string value;
    double numValue = 0.0;
};

class Lexer {
    string expr;
    size_t pos;

    char peek() const { return pos < expr.length() ? expr[pos] : '\0'; }
    char get() { return pos < expr.length() ? expr[pos++] : '\0'; }

public:
    Lexer(const string& expression) : expr(expression), pos(0) {}

    Token next() {
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
};

// ================= АБСТРАКТНОЕ ДЕРЕВО (AST) =================
class Node {
public:
    virtual ~Node() = default;
    virtual double eval(const map<string, double>& env) const = 0;
    virtual unique_ptr<Node> clone() const = 0;
    virtual unique_ptr<Node> derive(const string& var) const = 0;
    virtual string toString() const = 0;
    virtual bool isConstant() const = 0; 
};

class NumNode : public Node {
    double val;
public:
    NumNode(double v) : val(v) {}
    double eval(const map<string, double>&) const override { return val; }
    unique_ptr<Node> clone() const override { return make_unique<NumNode>(val); }
    unique_ptr<Node> derive(const string&) const override { return make_unique<NumNode>(0); }
    string toString() const override { return formatDouble(val); }
    bool isConstant() const override { return true; }
};

class VarNode : public Node {
    string name;
public:
    VarNode(const string& n) : name(n) {}
    double eval(const map<string, double>& env) const override {
        auto it = env.find(name);
        if (it == env.end()) throw runtime_error("ERROR Unknown variable: " + name);
        return it->second;
    }
    unique_ptr<Node> clone() const override { return make_unique<VarNode>(name); }
    unique_ptr<Node> derive(const string& var) const override { return make_unique<NumNode>(name == var ? 1 : 0); }
    string toString() const override { return name; }
    bool isConstant() const override { return false; }
};

class UnaryNode : public Node {
    char op;
    unique_ptr<Node> child;
public:
    UnaryNode(char o, unique_ptr<Node> c) : op(o), child(move(c)) {}
    double eval(const map<string, double>& env) const override {
        double v = child->eval(env);
        return op == '-' ? -v : v;
    }
    unique_ptr<Node> clone() const override { return make_unique<UnaryNode>(op, child->clone()); }
    unique_ptr<Node> derive(const string& var) const override {
        return make_unique<UnaryNode>(op, child->derive(var));
    }
    string toString() const override { return "(" + string(1, op) + child->toString() + ")"; }
    bool isConstant() const override { return child->isConstant(); }
};

class BinaryNode : public Node {
    char op;
    unique_ptr<Node> left, right;
public:
    BinaryNode(char o, unique_ptr<Node> l, unique_ptr<Node> r) : op(o), left(move(l)), right(move(r)) {}
    double eval(const map<string, double>& env) const override {
        double l = left->eval(env);
        double r = right->eval(env);
        
        if (op == '+') return l + r;
        if (op == '-') return l - r;
        if (op == '*') return l * r;
        if (op == '/') {
            if (l == 0 && r == 0) throw runtime_error("ERROR Domain error: 0/0 is undefined");
            return l / r; // Возвращает inf / -inf, если r == 0, а l != 0
        }
        if (op == '^') {
            if (l < 0 && floor(r) != r) throw runtime_error("ERROR Domain error: fractional power of negative number");
            return pow(l, r);
        }
        return 0;
    }
    unique_ptr<Node> clone() const override { return make_unique<BinaryNode>(op, left->clone(), right->clone()); }
    string toString() const override { return "(" + left->toString() + " " + op + " " + right->toString() + ")"; }
    bool isConstant() const override { return left->isConstant() && right->isConstant(); }
    
    unique_ptr<Node> derive(const string& var) const override; 
};

class FuncNode : public Node {
    string name;
    unique_ptr<Node> arg;
public:
    FuncNode(const string& n, unique_ptr<Node> a) : name(n), arg(move(a)) {}
    double eval(const map<string, double>& env) const override {
        double v = arg->eval(env);
        
        if (name == "sin") return sin(v);
        if (name == "cos") return cos(v);
        if (name == "tan") return tan(v);
        if (name == "exp") return exp(v);
        if (name == "log") {
            if (v < 0) throw runtime_error("ERROR Domain error: log(x) requires x >= 0");
            return log(v); // log(0) вернет -inf, как хочет чекер
        }
        if (name == "sqrt") {
            if (v < 0) throw runtime_error("ERROR Domain error: sqrt(-1)"); // Строгая ошибка по методичке
            return sqrt(v);
        }
        if (name == "asin") {
            if (v < -1 || v > 1) throw runtime_error("ERROR Domain error: asin(x) requires |x| <= 1");
            return asin(v);
        }
        if (name == "acos") {
            if (v < -1 || v > 1) throw runtime_error("ERROR Domain error: acos(x) requires |x| <= 1");
            return acos(v);
        }
        if (name == "atan") return atan(v);
        
        throw runtime_error("ERROR Unknown function: " + name);
    }
    unique_ptr<Node> clone() const override { return make_unique<FuncNode>(name, arg->clone()); }
    string toString() const override { return name + "(" + arg->toString() + ")"; }
    bool isConstant() const override { return arg->isConstant(); }
    
    unique_ptr<Node> derive(const string& var) const override; 
};

// ================= РЕАЛИЗАЦИЯ ПРОИЗВОДНЫХ =================

unique_ptr<Node> BinaryNode::derive(const string& var) const {
    auto du = left->derive(var);
    auto dv = right->derive(var);

    if (op == '+') return make_unique<BinaryNode>('+', move(du), move(dv));
    if (op == '-') return make_unique<BinaryNode>('-', move(du), move(dv));
    if (op == '*') {
        return make_unique<BinaryNode>('+',
            make_unique<BinaryNode>('*', move(du), right->clone()),
            make_unique<BinaryNode>('*', left->clone(), move(dv))
        );
    }
    if (op == '/') {
        auto num = make_unique<BinaryNode>('-',
            make_unique<BinaryNode>('*', move(du), right->clone()),
            make_unique<BinaryNode>('*', left->clone(), move(dv))
        );
        auto den = make_unique<BinaryNode>('^', right->clone(), make_unique<NumNode>(2));
        return make_unique<BinaryNode>('/', move(num), move(den));
    }
    if (op == '^') {
        if (right->isConstant()) {
            double c = right->eval({});
            auto c_node = make_unique<NumNode>(c);
            auto c_minus_1 = make_unique<NumNode>(c - 1.0);
            auto u_pow = make_unique<BinaryNode>('^', left->clone(), move(c_minus_1));
            auto mul1 = make_unique<BinaryNode>('*', move(c_node), move(u_pow));
            return make_unique<BinaryNode>('*', move(mul1), move(du));
        } else if (left->isConstant()) {
            auto c_pow_v = make_unique<BinaryNode>('^', left->clone(), right->clone());
            auto ln_c = make_unique<FuncNode>("log", left->clone());
            auto mul1 = make_unique<BinaryNode>('*', move(c_pow_v), move(ln_c));
            return make_unique<BinaryNode>('*', move(mul1), move(dv));
        } else {
            auto f_pow_g = make_unique<BinaryNode>('^', left->clone(), right->clone());
            auto ln_f = make_unique<FuncNode>("log", left->clone());
            auto term1 = make_unique<BinaryNode>('*', move(dv), move(ln_f));
            auto f_prime_div_f = make_unique<BinaryNode>('/', move(du), left->clone());
            auto term2 = make_unique<BinaryNode>('*', right->clone(), move(f_prime_div_f));
            auto sum = make_unique<BinaryNode>('+', move(term1), move(term2));
            return make_unique<BinaryNode>('*', move(f_pow_g), move(sum));
        }
    }
    return make_unique<NumNode>(0);
}

unique_ptr<Node> FuncNode::derive(const string& var) const {
    auto da = arg->derive(var);
    unique_ptr<Node> df;
    if (name == "sin") df = make_unique<FuncNode>("cos", arg->clone());
    else if (name == "cos") df = make_unique<UnaryNode>('-', make_unique<FuncNode>("sin", arg->clone()));
    else if (name == "tan") df = make_unique<BinaryNode>('/', make_unique<NumNode>(1), make_unique<BinaryNode>('^', make_unique<FuncNode>("cos", arg->clone()), make_unique<NumNode>(2)));
    else if (name == "log") df = make_unique<BinaryNode>('/', make_unique<NumNode>(1), arg->clone());
    else if (name == "exp") df = make_unique<FuncNode>("exp", arg->clone());
    else if (name == "sqrt") {
        df = make_unique<BinaryNode>('/', make_unique<NumNode>(1), 
             make_unique<BinaryNode>('*', make_unique<NumNode>(2), make_unique<FuncNode>("sqrt", arg->clone())));
    }
    else if (name == "asin") df = make_unique<BinaryNode>('/', make_unique<NumNode>(1), make_unique<FuncNode>("sqrt", make_unique<BinaryNode>('-', make_unique<NumNode>(1), make_unique<BinaryNode>('^', arg->clone(), make_unique<NumNode>(2)))));
    else if (name == "acos") df = make_unique<UnaryNode>('-', make_unique<BinaryNode>('/', make_unique<NumNode>(1), make_unique<FuncNode>("sqrt", make_unique<BinaryNode>('-', make_unique<NumNode>(1), make_unique<BinaryNode>('^', arg->clone(), make_unique<NumNode>(2))))));
    else if (name == "atan") df = make_unique<BinaryNode>('/', make_unique<NumNode>(1), make_unique<BinaryNode>('+', make_unique<NumNode>(1), make_unique<BinaryNode>('^', arg->clone(), make_unique<NumNode>(2))));
    else df = make_unique<NumNode>(0);
    
    return make_unique<BinaryNode>('*', move(df), move(da));
}

// ================= ПАРСЕР (PARSER) =================
class Parser {
    Lexer lexer;
    Token curr;

    void next() { curr = lexer.next(); }
    bool match(char op) {
        if (curr.type == lexem_t::OP && curr.value[0] == op) { next(); return true; }
        return false;
    }

public:
    static bool isReservedFunction(const string& name) {
        static const vector<string> funcs = {"sin", "cos", "tan", "asin", "acos", "atan", "exp", "log", "sqrt"};
        return find(funcs.begin(), funcs.end(), name) != funcs.end();
    }

    Parser(const string& expr) : lexer(expr) { next(); }

    unique_ptr<Node> parse() {
        auto node = parseExpression();
        if (curr.type != lexem_t::EOEX) throw runtime_error("ERROR: unexpected token at end");
        return node;
    }

private:
    unique_ptr<Node> parseExpression() {
        auto node = parseTerm();
        while (curr.type == lexem_t::OP && (curr.value == "+" || curr.value == "-")) {
            char op = curr.value[0]; next();
            node = make_unique<BinaryNode>(op, move(node), parseTerm());
        }
        return node;
    }

    unique_ptr<Node> parseTerm() {
        auto node = parseUnary();
        while (curr.type == lexem_t::OP && (curr.value == "*" || curr.value == "/")) {
            char op = curr.value[0]; next();
            node = make_unique<BinaryNode>(op, move(node), parseUnary());
        }
        return node;
    }

    unique_ptr<Node> parseUnary() {
        if (curr.type == lexem_t::OP && (curr.value == "+" || curr.value == "-")) {
            char op = curr.value[0]; next();
            return make_unique<UnaryNode>(op, parseUnary());
        }
        return parseFactor();
    }

    unique_ptr<Node> parseFactor() {
        auto node = parsePrimary();
        if (curr.type == lexem_t::OP && curr.value == "^") {
            next();
            node = make_unique<BinaryNode>('^', move(node), parseUnary()); 
        }
        return node;
    }

    unique_ptr<Node> parsePrimary() {
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
};

// ================= ТОЧКА ВХОДА (MAIN) =================
void printResult(double res) {
    if (std::isinf(res)) {
        cout << (res > 0 ? "inf" : "-inf") << "\n";
    } else if (std::isnan(res)) {
        cout << "nan\n";
    } else {
        cout << res << "\n";
    }
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    string first_word;
    if (!(cin >> first_word)) return 0;

    if (first_word == "evaluate" || first_word == "derivative" || first_word == "evaluate_derivative") {
        try {
            int n;
            if (!(cin >> n)) throw runtime_error("ERROR: bad variables count");
            if (n < 0) throw runtime_error("ERROR: negative variable count");

            vector<string> vars(n);
            set<string> seen_vars;
            
            for (int i = 0; i < n; ++i) {
                cin >> vars[i];
                transform(vars[i].begin(), vars[i].end(), vars[i].begin(), ::tolower);
                
                if (Parser::isReservedFunction(vars[i])) {
                    throw runtime_error("ERROR: Variable name cannot be a reserved function name (" + vars[i] + ")");
                }
                
                if (!seen_vars.insert(vars[i]).second) {
                    throw runtime_error("ERROR: Duplicate variable defined: " + vars[i]);
                }
            }

            map<string, double> env;
            for (int i = 0; i < n; ++i) {
                double val; cin >> val;
                env[vars[i]] = val;
            }

            string expr;
            getline(cin >> ws, expr);
            
            if (expr.empty()) throw runtime_error("ERROR: Empty expression");
            if (expr.length() > 10000) throw runtime_error("ERROR: Expression too long (exceeds 10000 chars)");

            Parser parser(expr);
            unique_ptr<Node> ast = parser.parse();

            if (first_word == "evaluate") {
                printResult(ast->eval(env));
            } 
            else if (first_word == "derivative") {
                if (vars.empty()) throw runtime_error("ERROR: No variables provided");
                unique_ptr<Node> derivative_ast = ast->derive(vars[0]);
                cout << derivative_ast->toString() << "\n";
            }
            else if (first_word == "evaluate_derivative") {
                if (vars.empty()) throw runtime_error("ERROR: No variables provided");
                unique_ptr<Node> derivative_ast = ast->derive(vars[0]);
                printResult(derivative_ast->eval(env));
            }
        } catch (const exception& e) {
            cout << e.what() << "\n";
        } catch (...) {
            cout << "ERROR: Critical failure\n";
        }
    } 
    else {
        // Режим тестирования лексера
        string rest;
        getline(cin, rest);
        string expr = first_word + rest;

        try {
            if (expr.length() > 10000) throw runtime_error("ERROR: Expression too long (exceeds 10000 chars)");
            
            vector<Token> tokens;
            Lexer lexer(expr);
            while (true) {
                Token t = lexer.next();
                if (t.type == lexem_t::EOEX) break;
                tokens.push_back(t);
            }
            for (const auto& t : tokens) {
                cout << t.value << "\n";
            }
        } catch (const exception& e) {
            cout << e.what() << "\n";
        }
    }

    return 0;
}