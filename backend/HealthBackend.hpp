#ifndef HEALTH_BACKEND_HPP
#define HEALTH_BACKEND_HPP

#include <string>
#include <vector>
#include <cstddef>

#include "../user/UserBackend.hpp"
#include "../records/Water.hpp"
#include "../records/Sleep.hpp"
#include "../records/Activity.hpp"
#include "../records/OtherCategory.hpp"

class HealthBackend {
private:
    UserBackend         userBackend;
    WaterManager        waterManager;
    SleepManager        sleepManager;
    ActivityManager     activityManager;
    OtherCategoryManager otherManager;

    // 從 token 找 userName，失敗回傳空字串
    std::string getUserNameFromToken(const std::string& token) const;

    // 檔案存取（啟動讀、每次修改寫）
    void loadAll();
    void saveAll() const;

public:
    // 建構子：啟動時自動從 data/storage.json 載入
    HealthBackend();

    // ===== 使用者相關 =====
    bool registerUser(const std::string& name,
                      int age,
                      double weightKg,
                      double heightM,
                      const std::string& password);

    std::string login(const std::string& name,
                      const std::string& password);

    bool updateUser(const std::string& token,
                    int newAge,
                    double newWeightKg,
                    double newHeightM,
                    const std::string& newPassword);

    bool deleteUser(const std::string& token);

    double getBMI(const std::string& token) const;

    // ===== 水紀錄 =====
    bool addWater(const std::string& token,
                  const std::string& date,
                  double amountMl);

    bool updateWater(const std::string& token,
                     std::size_t index,
                     const std::string& newDate,
                     double newAmountMl);

    bool deleteWater(const std::string& token,
                     std::size_t index);

    std::vector<WaterRecord> getAllWater(const std::string& token) const;

    double getWeeklyAverageWater(const std::string& token) const;

    bool isWaterEnough(const std::string& token,
                       double dailyGoalMl) const;

    // ===== 睡眠紀錄 =====
    bool addSleep(const std::string& token,
                  const std::string& date,
                  double hours);

    bool updateSleep(const std::string& token,
                     std::size_t index,
                     const std::string& newDate,
                     double newHours);

    bool deleteSleep(const std::string& token,
                     std::size_t index);

    std::vector<SleepRecord> getAllSleep(const std::string& token) const;

    double getLastSleepHours(const std::string& token) const;

    bool isSleepEnough(const std::string& token,
                       double minHours) const;

    // ===== 活動紀錄 =====
    bool addActivity(const std::string& token,
                     const std::string& date,
                     int minutes,
                     const std::string& intensity);

    bool updateActivity(const std::string& token,
                        std::size_t index,
                        const std::string& newDate,
                        int newMinutes,
                        const std::string& newIntensity);

    bool deleteActivity(const std::string& token,
                        std::size_t index);

    std::vector<ActivityRecord> getAllActivity(const std::string& token) const;

    void sortActivityByDuration(const std::string& token);

    // ===== 其他分類 =====
    bool addOtherRecord(const std::string& token,
                        const std::string& categoryName,
                        const std::string& date,
                        double value,
                        const std::string& note);

    bool updateOtherRecord(const std::string& token,
                           const std::string& categoryName,
                           std::size_t index,
                           const std::string& newDate,
                           double newValue,
                           const std::string& newNote);

    bool deleteOtherRecord(const std::string& token,
                           const std::string& categoryName,
                           std::size_t index);

    std::vector<std::string> getOtherCategories(const std::string& token) const;

    std::vector<OtherRecord> getOtherRecords(const std::string& token,
                                             const std::string& categoryName) const;
};

#endif