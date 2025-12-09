#ifndef USER_HPP
#define USER_HPP

#include <string>

class User {
private:
    std::string name;
    int         age{};
    double      weightKg{};
    double      heightM{};
    std::string password;
    std::string token;   // 目前登入用的 token（可空字串）

public:
    User() = default;

    User(const std::string& name,
         int age,
         double weightKg,
         double heightM,
         const std::string& password)
        : name(name),
          age(age),
          weightKg(weightKg),
          heightM(heightM),
          password(password) {}

    // --- getters ---
    const std::string& getName()     const { return name; }
    int                getAge()      const { return age; }
    double             getWeightKg() const { return weightKg; }
    double             getHeightM()  const { return heightM; }
    const std::string& getPassword() const { return password; }
    const std::string& getToken()    const { return token; }

    // --- setters ---
    void setAge(int a)                       { age = a; }
    void setWeightKg(double w)               { weightKg = w; }
    void setHeightM(double h)                { heightM = h; }
    void setPassword(const std::string& pw)  { password = pw; }
    void setToken(const std::string& t)      { token = t; }

    // --- BMI ---
    double getBMI() const {
        if (heightM <= 0.0) return 0.0;
        return weightKg / (heightM * heightM);
    }
};

#endif // USER_HPP