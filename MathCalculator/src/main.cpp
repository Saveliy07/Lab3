#include "lexer.hpp"
#include "ast.hpp"
#include "parser.hpp"
#include "utils.hpp"
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

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