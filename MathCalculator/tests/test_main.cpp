#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "parser.hpp"
#include "ast.hpp"

// Вспомогательная функция для сравнения double
bool is_close(double a, double b) {
    if (std::isnan(a) && std::isnan(b)) return true;
    if (std::isinf(a) && std::isinf(b)) return (a > 0) == (b > 0);
    return std::abs(a - b) < 1e-7;
}

#define RUN_TEST(test_func) \
    try { \
        test_func(); \
        std::cout << "[\033[32mPASS\033[0m] " << #test_func << "\n"; \
    } catch (const std::exception& e) { \
        std::cerr << "[\033[31mFAIL\033[0m] " << #test_func << " - Exception: " << e.what() << "\n"; \
        exit(1); \
    }

// ================= ТЕСТЫ ВЫЧИСЛЕНИЙ =================

void test_basic_arithmetic() {
    Parser p("10 + 5 - 3 * 2 / 1");
    auto ast = p.parse();
    assert(is_close(ast->eval({}), 9.0));
}

void test_operator_precedence() {
    Parser p("2 + 2 * 2");
    assert(is_close(p.parse()->eval({}), 6.0));

    Parser p2("(2 + 2) * 2");
    assert(is_close(p2.parse()->eval({}), 8.0));
    
    Parser p3("2 ^ 3 + 1");
    assert(is_close(p3.parse()->eval({}), 9.0));
}

void test_math_functions() {
    Parser p1("sin(0) + cos(0)");
    assert(is_close(p1.parse()->eval({}), 1.0));

    Parser p2("exp(1)");
    assert(is_close(p2.parse()->eval({}), std::exp(1.0)));

    Parser p3("sqrt(16) + log(1)");
    assert(is_close(p3.parse()->eval({}), 4.0));
}

void test_multiple_variables() {
    Parser p("x * y + z ^ 2");
    auto ast = p.parse();
    std::map<std::string, double> env = {{"x", 2.0}, {"y", 3.0}, {"z", 4.0}};
    assert(is_close(ast->eval(env), 22.0)); // 2*3 + 4^2 = 6 + 16 = 22
}

// ================= ТЕСТЫ ПРОИЗВОДНЫХ =================

void test_simple_derivative() {
    Parser p("x ^ 2");
    auto ast = p.parse();
    auto der = ast->derive("x");
    assert(is_close(der->eval({{"x", 5.0}}), 10.0));
}

void test_complex_derivative() {
    Parser p("sin(x) * exp(x)");
    auto ast = p.parse();
    auto der = ast->derive("x");
    assert(is_close(der->eval({{"x", 0.0}}), 1.0));
}

void test_partial_derivative() {
    Parser p("x ^ 2 * y");
    auto ast = p.parse();
    
    auto der_x = ast->derive("x");
    assert(is_close(der_x->eval({{"x", 2.0}, {"y", 3.0}}), 12.0));
    auto der_y = ast->derive("y");
    assert(is_close(der_y->eval({{"x", 2.0}, {"y", 3.0}}), 4.0));
}

// ================= ТЕСТЫ ОШИБОК (EXCEPTIONS) =================

void test_error_domain() {
    bool caught = false;
    try {
        Parser p("sqrt(-1)");
        p.parse()->eval({});
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught && "Should have thrown domain error for sqrt(-1)");

    caught = false;
    try {
        Parser p("log(-5)");
        p.parse()->eval({});
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught && "Should have thrown domain error for log(-5)");
}

void test_error_division_by_zero() {
    bool caught = false;
    try {
        Parser p("0 / 0");
        p.parse()->eval({});
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught && "Should have thrown error for 0/0");
}

void test_error_syntax() {
    bool caught = false;
    try {
        Parser p("1 + * 2");
        p.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught && "Should have thrown syntax error");
    
    caught = false;
    try {
        Parser p("sin(x"); // Нет закрывающей скобки
        p.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught && "Should have thrown missing parenthesis error");
}

void test_error_unknown_variable() {
    bool caught = false;
    try {
        Parser p("x + 2");
        p.parse()->eval({{"y", 2.0}}); // Передали y, но в выражении x
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught && "Should have thrown unknown variable error");
}

// ================= MAIN =================

int main() {
    std::cout << "--- Запуск юнит-тестов ---\n";
    
    RUN_TEST(test_basic_arithmetic);
    RUN_TEST(test_operator_precedence);
    RUN_TEST(test_math_functions);
    RUN_TEST(test_multiple_variables);
    
    RUN_TEST(test_simple_derivative);
    RUN_TEST(test_complex_derivative);
    RUN_TEST(test_partial_derivative);
    
    RUN_TEST(test_error_domain);
    RUN_TEST(test_error_division_by_zero);
    RUN_TEST(test_error_syntax);
    RUN_TEST(test_error_unknown_variable);
    
    std::cout << "--------------------------\n";
    std::cout << "[\033[32mSUCCESS\033[0m] Все 11 тестов пройдены успешно!\n";
    
    return 0;
}