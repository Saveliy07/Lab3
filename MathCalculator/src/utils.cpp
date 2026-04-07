#include "utils.hpp"
#include <iostream>
#include <cmath>

using namespace std;

string formatDouble(double v) {
    string s = to_string(v);
    s.erase(s.find_last_not_of('0') + 1, string::npos);
    if (!s.empty() && s.back() == '.') s.pop_back();
    return s;
}

void printResult(double res) {
    if (std::isinf(res)) {
        cout << (res > 0 ? "inf" : "-inf") << "\n";
    } else if (std::isnan(res)) {
        cout << "nan\n";
    } else {
        cout << res << "\n";
    }
}