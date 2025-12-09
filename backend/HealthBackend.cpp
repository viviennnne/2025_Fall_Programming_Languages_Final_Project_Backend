#include "HealthBackend.hpp"
#include "../helpers/validation.hpp"
#include "../external/json.hpp"

#include <fstream>
#include <iostream>

// 簡單 alias
using json = nlohmann::json;

// ===== 私有工具 =====

// 從 token 找 userName，失敗回傳空字串
std::string HealthBackend::getUserNameFromToken(const std::string& token) const {
    return userBackend.getUserNameByToken(token);
}

// 建構子：啟動時自動從 data/storage.json 載入
HealthBackend::HealthBackend() {
    loadAll();
}

// 從 data/storage.json 載入全部資料
void HealthBackend::loadAll() {
    std::ifstream in("data/storage.json");   // ✅ 存在 data 資料夾裡
    if (!in.good()) {
        // 第一次啟動可能沒有檔案，直接略過
        return;
    }

    try {
        json j;
        in >> j;

        if (j.contains("users")) {
            userBackend.fromJson(j["users"]);
        }
        if (j.contains("water")) {
            waterManager.fromJson(j["water"]);
        }
        if (j.contains("sleep")) {
            sleepManager.fromJson(j["sleep"]);
        }
        if (j.contains("activity")) {
            activityManager.fromJson(j["activity"]);
        }
        if (j.contains("other")) {
            otherManager.fromJson(j["other"]);
        }
    } catch (const std::exception& e) {
        std::cerr << "[HealthBackend::loadAll] JSON parse error: "
                  << e.what() << std::endl;
        // 讀檔失敗就當成沒有資料，不讓整個程式炸掉
    }
}

// 把全部資料寫回 data/storage.json
void HealthBackend::saveAll() const {
    json j;
    j["users"]    = userBackend.toJson();
    j["water"]    = waterManager.toJson();
    j["sleep"]    = sleepManager.toJson();
    j["activity"] = activityManager.toJson();
    j["other"]    = otherManager.toJson();

    std::ofstream out("data/storage.json");   // ✅ 存在 data 資料夾裡
    if (!out.good()) {
        std::cerr << "[HealthBackend::saveAll] cannot open data/storage.json for write\n";
        return;
    }

    out << j.dump(2); // 排版漂亮一點
}

// ===== 使用者相關 =====
bool HealthBackend::registerUser(const std::string& name,
                                 int age,
                                 double weightKg,
                                 double heightM,
                                 const std::string& password) {
    if (!Validation::isValidName(name))         return false;
    if (!Validation::isValidAge(age))          return false;
    if (!Validation::isValidWeight(weightKg))  return false;
    if (!Validation::isValidHeight(heightM))   return false;
    if (!Validation::isValidPassword(password)) return false;

    bool ok = userBackend.registerUser(name, age, weightKg, heightM, password);
    if (ok) saveAll();
    return ok;
}

std::string HealthBackend::login(const std::string& name,
                                 const std::string& password) {
    // token 不寫入檔案，所以不用 saveAll()
    return userBackend.login(name, password);
}

bool HealthBackend::updateUser(const std::string& token,
                               int newAge,
                               double newWeightKg,
                               double newHeightM,
                               const std::string& newPassword) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    if (!Validation::isValidAge(newAge))          return false;
    if (!Validation::isValidWeight(newWeightKg))  return false;
    if (!Validation::isValidHeight(newHeightM))   return false;
    if (!Validation::isValidPassword(newPassword)) return false;

    bool ok = userBackend.updateUser(userName,
                                     newAge,
                                     newWeightKg,
                                     newHeightM,
                                     newPassword);
    if (ok) saveAll();
    return ok;
}

bool HealthBackend::deleteUser(const std::string& token) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    bool ok = userBackend.deleteUser(userName);
    // 目前沒有同步刪除該 user 的 water/sleep/activity/other 記錄，
    // 如果之後要做「整個人全部刪光」可以在這邊再處理。
    if (ok) saveAll();
    return ok;
}

double HealthBackend::getBMI(const std::string& token) const {
    return userBackend.getUserBMI(token);
}

// ===== 水紀錄 =====
bool HealthBackend::addWater(const std::string& token,
                             const std::string& date,
                             double amountMl) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    if (!Validation::isValidDate(date))       return false;
    if (!Validation::isNonNegative(amountMl)) return false;

    bool ok = waterManager.addRecord(userName, date, amountMl);
    if (ok) saveAll();
    return ok;
}

bool HealthBackend::updateWater(const std::string& token,
                                std::size_t index,
                                const std::string& newDate,
                                double newAmountMl) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    if (!Validation::isValidDate(newDate))        return false;
    if (!Validation::isNonNegative(newAmountMl))  return false;

    bool ok = waterManager.updateRecord(userName, index, newDate, newAmountMl);
    if (ok) saveAll();
    return ok;
}

bool HealthBackend::deleteWater(const std::string& token,
                                std::size_t index) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    bool ok = waterManager.deleteRecord(userName, index);
    if (ok) saveAll();
    return ok;
}

std::vector<WaterRecord> HealthBackend::getAllWater(const std::string& token) const {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return {};
    return waterManager.getAll(userName);
}

double HealthBackend::getWeeklyAverageWater(const std::string& token) const {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return 0.0;
    return waterManager.getWeeklyAverage(userName);
}

bool HealthBackend::isWaterEnough(const std::string& token,
                                  double dailyGoalMl) const {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;
    return waterManager.isEnoughForWeek(userName, dailyGoalMl);
}

// ===== 睡眠紀錄 =====
bool HealthBackend::addSleep(const std::string& token,
                             const std::string& date,
                             double hours) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    if (!Validation::isValidDate(date))      return false;
    if (!Validation::isNonNegative(hours))   return false;

    bool ok = sleepManager.addRecord(userName, date, hours);
    if (ok) saveAll();
    return ok;
}

bool HealthBackend::updateSleep(const std::string& token,
                                std::size_t index,
                                const std::string& newDate,
                                double newHours) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    if (!Validation::isValidDate(newDate))     return false;
    if (!Validation::isNonNegative(newHours))  return false;

    bool ok = sleepManager.updateRecord(userName, index, newDate, newHours);
    if (ok) saveAll();
    return ok;
}

bool HealthBackend::deleteSleep(const std::string& token,
                                std::size_t index) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    bool ok = sleepManager.deleteRecord(userName, index);
    if (ok) saveAll();
    return ok;
}

std::vector<SleepRecord> HealthBackend::getAllSleep(const std::string& token) const {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return {};
    return sleepManager.getAll(userName);
}

double HealthBackend::getLastSleepHours(const std::string& token) const {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return 0.0;

    return sleepManager.getLastSleepHours(userName);
}

bool HealthBackend::isSleepEnough(const std::string& token,
                                  double minHours) const {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    return sleepManager.isSleepEnough(userName, minHours);
}

// ===== 活動紀錄 =====
bool HealthBackend::addActivity(const std::string& token,
                                const std::string& date,
                                int minutes,
                                const std::string& intensity) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    if (!Validation::isValidDate(date)) return false;
    if (minutes < 0)                    return false;

    bool ok = activityManager.addRecord(userName, date, minutes, intensity);
    if (ok) saveAll();
    return ok;
}

bool HealthBackend::updateActivity(const std::string& token,
                                   std::size_t index,
                                   const std::string& newDate,
                                   int newMinutes,
                                   const std::string& newIntensity) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    if (!Validation::isValidDate(newDate)) return false;
    if (newMinutes < 0)                    return false;

    bool ok = activityManager.updateRecord(userName, index, newDate, newMinutes, newIntensity);
    if (ok) saveAll();
    return ok;
}

bool HealthBackend::deleteActivity(const std::string& token,
                                   std::size_t index) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    bool ok = activityManager.deleteRecord(userName, index);
    if (ok) saveAll();
    return ok;
}

std::vector<ActivityRecord> HealthBackend::getAllActivity(const std::string& token) const {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return {};
    return activityManager.getAll(userName);
}

void HealthBackend::sortActivityByDuration(const std::string& token) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return;
    activityManager.sortByDuration(userName);
    saveAll();
}

// ===== 其他分類 =====
bool HealthBackend::addOtherRecord(const std::string& token,
                                   const std::string& categoryName,
                                   const std::string& date,
                                   double value,
                                   const std::string& note) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    if (!Validation::isValidName(categoryName)) return false;
    if (!Validation::isValidDate(date))         return false;
    if (!Validation::isNonNegative(value))      return false;

    bool ok = otherManager.addRecord(userName, categoryName, date, value, note);
    if (ok) saveAll();
    return ok;
}

bool HealthBackend::updateOtherRecord(const std::string& token,
                                      const std::string& categoryName,
                                      std::size_t index,
                                      const std::string& newDate,
                                      double newValue,
                                      const std::string& newNote) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    if (!Validation::isValidName(categoryName)) return false;
    if (!Validation::isValidDate(newDate))      return false;
    if (!Validation::isNonNegative(newValue))   return false;

    bool ok = otherManager.updateRecord(userName, categoryName, index, newDate, newValue, newNote);
    if (ok) saveAll();
    return ok;
}

bool HealthBackend::deleteOtherRecord(const std::string& token,
                                      const std::string& categoryName,
                                      std::size_t index) {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return false;

    bool ok = otherManager.deleteRecord(userName, categoryName, index);
    if (ok) saveAll();
    return ok;
}

std::vector<std::string> HealthBackend::getOtherCategories(const std::string& token) const {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return {};

    return otherManager.getCategories(userName);
}

std::vector<OtherRecord> HealthBackend::getOtherRecords(const std::string& token,
                                                        const std::string& categoryName) const {
    std::string userName = getUserNameFromToken(token);
    if (userName.empty()) return {};

    return otherManager.getRecords(userName, categoryName);
}