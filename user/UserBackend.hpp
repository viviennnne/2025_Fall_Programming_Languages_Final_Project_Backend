#ifndef USER_BACKEND_HPP
#define USER_BACKEND_HPP

#include <string>
#include <map>
#include <unordered_map>
#include "../external/json.hpp"
#include "User.hpp"

// 負責管理所有 User + token
class UserBackend {
private:
    // 用 name 當 key 存 User
    std::map<std::string, User> users;

    // token -> name
    std::unordered_map<std::string, std::string> tokenToName;

    // 產生隨機 token
    std::string generateToken() const;

public:
    UserBackend() = default;

    bool registerUser(const std::string& name,
                      int age,
                      double weightKg,
                      double heightM,
                      const std::string& password);

    std::string login(const std::string& name,
                      const std::string& password);

    bool updateUser(const std::string& name,
                    int newAge,
                    double newWeightKg,
                    double newHeightM,
                    const std::string& newPassword);

    bool deleteUser(const std::string& name);

    // 用 token 算 BMI（給 HealthBackend 用）
    double getUserBMI(const std::string& token) const;

    // 從 token 找 name（HealthBackend 裡用來找 user）
    std::string getUserNameByToken(const std::string& token) const;

    // 存到 JSON / 從 JSON 載入
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

#endif // USER_BACKEND_HPP