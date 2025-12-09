#include "UserBackend.hpp"
#include <random>

using json = nlohmann::json;

// 產生隨機 token（長度 24，只包含 A-Z a-z 0-9）
std::string UserBackend::generateToken() const {
    static const char chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";

    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, static_cast<int>(sizeof(chars) - 2));

    std::string token;
    token.reserve(24);
    for (int i = 0; i < 24; ++i) {
        token.push_back(chars[dist(rng)]);
    }
    return token;
}

// 註冊新使用者（只建立帳號，不產生 token）
bool UserBackend::registerUser(const std::string& name,
                               int age,
                               double weightKg,
                               double heightM,
                               const std::string& password) {
    // 已存在
    if (users.find(name) != users.end()) {
        return false;
    }

    User u{name, age, weightKg, heightM, password};
    users[name] = u;
    return true;
}

// 登入：驗證密碼 → 產生新 token
std::string UserBackend::login(const std::string& name,
                               const std::string& password) {
    auto it = users.find(name);
    if (it == users.end()) {
        return "INVALID";
    }
    if (it->second.getPassword() != password) {
        return "INVALID";
    }

    // 清掉這個 user 以前的 token（如果有）
    for (auto itTok = tokenToName.begin(); itTok != tokenToName.end(); ) {
        if (itTok->second == name) {
            itTok = tokenToName.erase(itTok);
        } else {
            ++itTok;
        }
    }

    std::string token = generateToken();
    it->second.setToken(token);
    tokenToName[token] = name;
    return token;
}

// 更新使用者（HealthBackend 已經先驗證過資料）
bool UserBackend::updateUser(const std::string& name,
                             int newAge,
                             double newWeightKg,
                             double newHeightM,
                             const std::string& newPassword) {
    auto it = users.find(name);
    if (it == users.end()) {
        return false;
    }

    it->second.setAge(newAge);
    it->second.setWeightKg(newWeightKg);  // ★ 用 setWeightKg
    it->second.setHeightM(newHeightM);   // ★ 用 setHeightM
    it->second.setPassword(newPassword);
    return true;
}

// 刪除使用者（順便把相關 token 清掉）
bool UserBackend::deleteUser(const std::string& name) {
    auto it = users.find(name);
    if (it == users.end()) {
        return false;
    }

    // 清除 tokenToName 裡對應的 token
    for (auto itTok = tokenToName.begin(); itTok != tokenToName.end(); ) {
        if (itTok->second == name) {
            itTok = tokenToName.erase(itTok);
        } else {
            ++itTok;
        }
    }

    users.erase(it);
    return true;
}

// 用 token 算 BMI
double UserBackend::getUserBMI(const std::string& token) const {
    auto itTok = tokenToName.find(token);
    if (itTok == tokenToName.end()) {
        return 0.0;
    }

    auto itUser = users.find(itTok->second);
    if (itUser == users.end()) {
        return 0.0;
    }
    return itUser->second.getBMI();
}

// 從 token 找 user 名字
std::string UserBackend::getUserNameByToken(const std::string& token) const {
    auto itTok = tokenToName.find(token);
    if (itTok == tokenToName.end()) {
        return "";
    }
    return itTok->second;
}

// 把目前所有使用者存成 JSON 陣列
// [
//   { "name": "...", "age": ..., "weightKg": ..., "heightM": ..., "password": "..." },
//   ...
// ]
json UserBackend::toJson() const {
    json arr = json::array();

    for (const auto& [name, u] : users) {
        json ju;
        ju["name"]     = u.getName();
        ju["age"]      = u.getAge();
        ju["weightKg"] = u.getWeightKg();  // ★ getWeightKg
        ju["heightM"]  = u.getHeightM();   // ★ getHeightM
        ju["password"] = u.getPassword();
        // token 不存，重開 server 之後要重新登入拿新 token
        arr.push_back(ju);
    }

    return arr;
}

// 從 JSON 載回使用者資料（不含 token）
void UserBackend::fromJson(const json& j) {
    users.clear();
    tokenToName.clear();

    if (!j.is_array()) {
        return;
    }

    for (const auto& ju : j) {
        std::string name     = ju.value("name", "");
        int         age      = ju.value("age", 0);
        double      weightKg = ju.value("weightKg", 0.0);
        double      heightM  = ju.value("heightM", 0.0);
        std::string password = ju.value("password", "");

        if (name.empty()) continue;

        users.emplace(name, User{name, age, weightKg, heightM, password});
    }
}