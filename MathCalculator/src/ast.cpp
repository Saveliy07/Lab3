#include "ast.hpp"
#include "utils.hpp"
#include <cmath>

using namespace std;

// === NumNode ===
NumNode::NumNode(double v) : val(v) {}
double NumNode::eval(const map<string, double>&) const { return val; }
unique_ptr<Node> NumNode::clone() const { return make_unique<NumNode>(val); }
unique_ptr<Node> NumNode::derive(const string&) const { return make_unique<NumNode>(0); }
string NumNode::toString() const { return formatDouble(val); }
bool NumNode::isConstant() const { return true; }

// === VarNode ===
VarNode::VarNode(const string& n) : name(n) {}
double VarNode::eval(const map<string, double>& env) const {
    auto it = env.find(name);
    if (it == env.end()) throw runtime_error("ERROR Unknown variable: " + name);
    return it->second;
}
unique_ptr<Node> VarNode::clone() const { return make_unique<VarNode>(name); }
unique_ptr<Node> VarNode::derive(const string& var) const { return make_unique<NumNode>(name == var ? 1 : 0); }
string VarNode::toString() const { return name; }
bool VarNode::isConstant() const { return false; }

// === UnaryNode ===
UnaryNode::UnaryNode(char o, unique_ptr<Node> c) : op(o), child(move(c)) {}
double UnaryNode::eval(const map<string, double>& env) const {
    double v = child->eval(env);
    return op == '-' ? -v : v;
}
unique_ptr<Node> UnaryNode::clone() const { return make_unique<UnaryNode>(op, child->clone()); }
unique_ptr<Node> UnaryNode::derive(const string& var) const {
    return make_unique<UnaryNode>(op, child->derive(var));
}
string UnaryNode::toString() const { return "(" + string(1, op) + child->toString() + ")"; }
bool UnaryNode::isConstant() const { return child->isConstant(); }

// === BinaryNode ===
BinaryNode::BinaryNode(char o, unique_ptr<Node> l, unique_ptr<Node> r) : op(o), left(move(l)), right(move(r)) {}
double BinaryNode::eval(const map<string, double>& env) const {
    double l = left->eval(env);
    double r = right->eval(env);
    if (op == '+') return l + r;
    if (op == '-') return l - r;
    if (op == '*') return l * r;
    if (op == '/') {
        if (l == 0 && r == 0) throw runtime_error("ERROR Domain error: 0/0 is undefined");
        return l / r; 
    }
    if (op == '^') {
        if (l < 0 && floor(r) != r) throw runtime_error("ERROR Domain error: fractional power of negative number");
        return pow(l, r);
    }
    return 0;
}
unique_ptr<Node> BinaryNode::clone() const { return make_unique<BinaryNode>(op, left->clone(), right->clone()); }
string BinaryNode::toString() const { return "(" + left->toString() + " " + op + " " + right->toString() + ")"; }
bool BinaryNode::isConstant() const { return left->isConstant() && right->isConstant(); }

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

// === FuncNode ===
FuncNode::FuncNode(const string& n, unique_ptr<Node> a) : name(n), arg(move(a)) {}
double FuncNode::eval(const map<string, double>& env) const {
    double v = arg->eval(env);
    if (name == "sin") return sin(v);
    if (name == "cos") return cos(v);
    if (name == "tan") return tan(v);
    if (name == "exp") return exp(v);
    if (name == "log") {
        if (v < 0) throw runtime_error("ERROR Domain error: log(x) requires x >= 0");
        return log(v);
    }
    if (name == "sqrt") {
        if (v < 0) throw runtime_error("ERROR Domain error: sqrt(-1)");
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
unique_ptr<Node> FuncNode::clone() const { return make_unique<FuncNode>(name, arg->clone()); }
string FuncNode::toString() const { return name + "(" + arg->toString() + ")"; }
bool FuncNode::isConstant() const { return arg->isConstant(); }

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