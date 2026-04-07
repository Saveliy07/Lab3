#pragma once
#include <string>
#include <map>
#include <memory>
#include <stdexcept>

class Node {
public:
    virtual ~Node() = default;
    virtual double eval(const std::map<std::string, double>& env) const = 0;
    virtual std::unique_ptr<Node> clone() const = 0;
    virtual std::unique_ptr<Node> derive(const std::string& var) const = 0;
    virtual std::string toString() const = 0;
    virtual bool isConstant() const = 0; 
};

class NumNode : public Node {
    double val;
public:
    NumNode(double v);
    double eval(const std::map<std::string, double>& env) const override;
    std::unique_ptr<Node> clone() const override;
    std::unique_ptr<Node> derive(const std::string& var) const override;
    std::string toString() const override;
    bool isConstant() const override;
};

class VarNode : public Node {
    std::string name;
public:
    VarNode(const std::string& n);
    double eval(const std::map<std::string, double>& env) const override;
    std::unique_ptr<Node> clone() const override;
    std::unique_ptr<Node> derive(const std::string& var) const override;
    std::string toString() const override;
    bool isConstant() const override;
};

class UnaryNode : public Node {
    char op;
    std::unique_ptr<Node> child;
public:
    UnaryNode(char o, std::unique_ptr<Node> c);
    double eval(const std::map<std::string, double>& env) const override;
    std::unique_ptr<Node> clone() const override;
    std::unique_ptr<Node> derive(const std::string& var) const override;
    std::string toString() const override;
    bool isConstant() const override;
};

class BinaryNode : public Node {
    char op;
    std::unique_ptr<Node> left, right;
public:
    BinaryNode(char o, std::unique_ptr<Node> l, std::unique_ptr<Node> r);
    double eval(const std::map<std::string, double>& env) const override;
    std::unique_ptr<Node> clone() const override;
    std::string toString() const override;
    bool isConstant() const override;
    std::unique_ptr<Node> derive(const std::string& var) const override; 
};

class FuncNode : public Node {
    std::string name;
    std::unique_ptr<Node> arg;
public:
    FuncNode(const std::string& n, std::unique_ptr<Node> a);
    double eval(const std::map<std::string, double>& env) const override;
    std::unique_ptr<Node> clone() const override;
    std::string toString() const override;
    bool isConstant() const override;
    std::unique_ptr<Node> derive(const std::string& var) const override; 
};