#include "validation.hpp"

#include <cctype>

bool Validation::isValidName(const std::string& name) {
    return !name.empty() && name.size() <= 50;
}

bool Validation::isValidAge(int age) {
    return age >= 0 && age <= 120;
}

bool Validation::isValidWeight(double w) {
    return w > 0.0 && w < 500.0;
}

bool Validation::isValidHeight(double h) {
    return h > 0.0 && h < 3.0; // 3 公尺以上就當怪物了 XD
}

bool Validation::isValidPassword(const std::string& pw) {
    return pw.size() >= 3 && pw.size() <= 100;
}

bool Validation::isValidDate(const std::string& date) {
    // 超簡單檢查：YYYY-MM-DD 長度 10，位置 4 & 7 是 '-'
    if (date.size() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;
    // 其他位全部是數字
    for (std::size_t i = 0; i < date.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(date[i]))) return false;
    }
    return true;
}

bool Validation::isNonNegative(double x) {
    return x >= 0.0;
}