#include <iostream>
#include <string>

#include "httplib.h"                // cpp-httplib 單一 header
#include "backend/HealthBackend.hpp"
#include "external/json.hpp"

using json = nlohmann::json;

// 從 Header 取出 X-Auth-Token
std::string getTokenFromHeader(const httplib::Request& req) {
    auto it = req.headers.find("X-Auth-Token");
    if (it == req.headers.end()) {
        return "";
    }
    return it->second;
}

int main() {
    HealthBackend backend;   // 核心後端（裡面會自動 load data/storage.json）
    httplib::Server svr;

    // --- Health check ---
    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json j;
        j["status"]  = "ok";
        j["message"] = "health_backend server running";
        res.status = 200;
        res.set_content(j.dump(), "application/json");
    });

    // =======================
    //        User APIs
    // =======================

    // POST /register
    // body: { "name","age","weightKg","heightM","password" }
    svr.Post("/register", [&backend](const httplib::Request& req, httplib::Response& res) {
        try {
            json j = json::parse(req.body);

            if (!j.contains("name") || !j.contains("age") ||
                !j.contains("weightKg") || !j.contains("heightM") ||
                !j.contains("password")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing fields";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::string name     = j["name"].get<std::string>();
            int         age      = j["age"].get<int>();
            double      weightKg = j["weightKg"].get<double>();
            double      heightM  = j["heightM"].get<double>();
            std::string password = j["password"].get<std::string>();

            bool ok = backend.registerUser(name, age, weightKg, heightM, password);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "User already exists or invalid input";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            // 註冊完順便登入一次拿 token
            std::string token = backend.login(name, password);

            json out;
            out["status"] = "ok";
            out["token"]  = token;
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // POST /login
    // body: { "name","password" }
    svr.Post("/login", [&backend](const httplib::Request& req, httplib::Response& res) {
        try {
            json j = json::parse(req.body);

            if (!j.contains("name") || !j.contains("password")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing name or password";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::string name     = j["name"].get<std::string>();
            std::string password = j["password"].get<std::string>();

            std::string token = backend.login(name, password);
            if (token == "INVALID") {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Invalid name or password";
                res.status = 401;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"] = "ok";
            out["token"]  = token;
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // GET /user/bmi
    // header: X-Auth-Token
    svr.Get("/user/bmi", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        double bmi = backend.getBMI(token);

        json out;
        out["status"] = "ok";
        out["bmi"]    = bmi;
        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    // =======================
    //       Water APIs
    // =======================

    // POST /water/add
    // body: { "date":"YYYY-MM-DD", "amountMl":1500 }
    svr.Post("/water/add", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("date") || !j.contains("amountMl")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing date or amountMl";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::string date     = j["date"].get<std::string>();
            double      amountMl = j["amountMl"].get<double>();

            bool ok = backend.addWater(token, date, amountMl);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to add water record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Water record added";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // POST /water/edit
    // body: { "index":0, "date":"YYYY-MM-DD", "amountMl":2000 }
    svr.Post("/water/edit", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("index") || !j.contains("date") || !j.contains("amountMl")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing index or date or amountMl";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::size_t index    = j["index"].get<std::size_t>();
            std::string date     = j["date"].get<std::string>();
            double      amountMl = j["amountMl"].get<double>();

            bool ok = backend.updateWater(token, index, date, amountMl);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to edit water record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Water record updated";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // POST /water/delete
    // body: { "index":0 }
    svr.Post("/water/delete", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("index")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing index";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }
            std::size_t index = j["index"].get<std::size_t>();

            bool ok = backend.deleteWater(token, index);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to delete water record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Water record deleted";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // GET /water/all
    svr.Get("/water/all", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        auto records = backend.getAllWater(token);

        json out;
        out["status"]  = "ok";
        out["records"] = json::array();
        for (const auto& r : records) {
            json jr;
            jr["date"]     = r.date;
            jr["amountMl"] = r.amountMl;
            out["records"].push_back(jr);
        }

        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    // GET /water/weekly_average
    svr.Get("/water/weekly_average", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        double avg = backend.getWeeklyAverageWater(token);

        json out;
        out["status"]          = "ok";
        out["weeklyAverageMl"] = avg;
        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    // GET /water/is_enough?goal=1500
    svr.Get("/water/is_enough", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        double goal = 1500.0;
        if (req.has_param("goal")) {
            goal = std::stod(req.get_param_value("goal"));
        }

        bool enough = backend.isWaterEnough(token, goal);

        json out;
        out["status"] = "ok";
        out["goal"]   = goal;
        out["enough"] = enough;
        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    // =======================
    //       Sleep APIs
    // =======================

    // POST /sleep/add
    // body: { "date":"YYYY-MM-DD", "hours":7.5 }
    svr.Post("/sleep/add", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("date") || !j.contains("hours")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing date or hours";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::string date = j["date"].get<std::string>();
            double      h    = j["hours"].get<double>();

            bool ok = backend.addSleep(token, date, h);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to add sleep record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Sleep record added";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // POST /sleep/edit
    // body: { "index":0, "date":"YYYY-MM-DD", "hours":6.0 }
    svr.Post("/sleep/edit", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("index") || !j.contains("date") || !j.contains("hours")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing index or date or hours";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::size_t index = j["index"].get<std::size_t>();
            std::string date  = j["date"].get<std::string>();
            double      h     = j["hours"].get<double>();

            bool ok = backend.updateSleep(token, index, date, h);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to edit sleep record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Sleep record updated";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // POST /sleep/delete
    // body: { "index":0 }
    svr.Post("/sleep/delete", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("index")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing index";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }
            std::size_t index = j["index"].get<std::size_t>();

            bool ok = backend.deleteSleep(token, index);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to delete sleep record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Sleep record deleted";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // GET /sleep/all
    svr.Get("/sleep/all", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        auto records = backend.getAllSleep(token);

        json out;
        out["status"]  = "ok";
        out["records"] = json::array();
        for (const auto& r : records) {
            json jr;
            jr["date"]  = r.date;
            jr["hours"] = r.hours;
            out["records"].push_back(jr);
        }

        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    // GET /sleep/last_hours
    svr.Get("/sleep/last_hours", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        double h = backend.getLastSleepHours(token);

        json out;
        out["status"] = "ok";
        out["hours"]  = h;
        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    // GET /sleep/is_enough?min=7
    svr.Get("/sleep/is_enough", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        double minHours = 7.0;
        if (req.has_param("min")) {
            minHours = std::stod(req.get_param_value("min"));
        }

        bool enough = backend.isSleepEnough(token, minHours);

        json out;
        out["status"]   = "ok";
        out["minHours"] = minHours;
        out["enough"]   = enough;
        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    // =======================
    //      Activity APIs
    // =======================

    // POST /activity/add
    // body: { "date":"YYYY-MM-DD", "minutes":30, "intensity":"medium" }
    svr.Post("/activity/add", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("date") || !j.contains("minutes") || !j.contains("intensity")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing date or minutes or intensity";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::string date      = j["date"].get<std::string>();
            int         minutes   = j["minutes"].get<int>();
            std::string intensity = j["intensity"].get<std::string>();

            bool ok = backend.addActivity(token, date, minutes, intensity);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to add activity record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Activity record added";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // POST /activity/edit
    // body: { "index":0, "date":"YYYY-MM-DD", "minutes":20, "intensity":"low" }
    svr.Post("/activity/edit", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("index") || !j.contains("date") ||
                !j.contains("minutes") || !j.contains("intensity")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing fields";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::size_t index     = j["index"].get<std::size_t>();
            std::string date      = j["date"].get<std::string>();
            int         minutes   = j["minutes"].get<int>();
            std::string intensity = j["intensity"].get<std::string>();

            bool ok = backend.updateActivity(token, index, date, minutes, intensity);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to edit activity record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Activity record updated";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // POST /activity/delete
    // body: { "index":0 }
    svr.Post("/activity/delete", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("index")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing index";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }
            std::size_t index = j["index"].get<std::size_t>();

            bool ok = backend.deleteActivity(token, index);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to delete activity record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Activity record deleted";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // GET /activity/all
    svr.Get("/activity/all", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        auto records = backend.getAllActivity(token);

        json out;
        out["status"]  = "ok";
        out["records"] = json::array();
        for (const auto& a : records) {
            json ja;
            ja["date"]      = a.date;
            ja["minutes"]   = a.minutes;
            ja["intensity"] = a.intensity;
            out["records"].push_back(ja);
        }

        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    // GET /activity/sort_by_duration
    // 會呼叫 backend.sortActivityByDuration() 然後回傳排序後列表
    svr.Get("/activity/sort_by_duration", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        backend.sortActivityByDuration(token);
        auto records = backend.getAllActivity(token);

        json out;
        out["status"]  = "ok";
        out["records"] = json::array();
        for (const auto& a : records) {
            json ja;
            ja["date"]      = a.date;
            ja["minutes"]   = a.minutes;
            ja["intensity"] = a.intensity;
            out["records"].push_back(ja);
        }

        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    // =======================
    //    Other Category APIs
    // =======================

    // POST /other/create
    // body: { "categoryName":"xxx" }
    // 這裡「不實際寫入資料」，只是回覆 OK，真正建立類別是在第一次 add_record 時自然出現
    svr.Post("/other/create", [](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("categoryName")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing categoryName";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Category will be created when first record is added";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // POST /other/add_record
    // body: { "categoryName":"xxx", "date":"YYYY-MM-DD", "value":123.4, "note":"..." }
    svr.Post("/other/add_record", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("categoryName") || !j.contains("date") ||
                !j.contains("value") || !j.contains("note")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing fields";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::string category = j["categoryName"].get<std::string>();
            std::string date     = j["date"].get<std::string>();
            double      value    = j["value"].get<double>();
            std::string note     = j["note"].get<std::string>();

            bool ok = backend.addOtherRecord(token, category, date, value, note);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to add other record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Other record added";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // POST /other/edit_record
    // body: { "categoryName":"xxx", "index":0, "date":"...", "value":..., "note":"..." }
    svr.Post("/other/edit_record", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("categoryName") || !j.contains("index") ||
                !j.contains("date") || !j.contains("value") || !j.contains("note")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing fields";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::string category = j["categoryName"].get<std::string>();
            std::size_t index    = j["index"].get<std::size_t>();
            std::string date     = j["date"].get<std::string>();
            double      value    = j["value"].get<double>();
            std::string note     = j["note"].get<std::string>();

            bool ok = backend.updateOtherRecord(token, category, index, date, value, note);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to edit other record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Other record updated";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // POST /other/delete_record
    // body: { "categoryName":"xxx", "index":0 }
    svr.Post("/other/delete_record", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        try {
            json j = json::parse(req.body);
            if (!j.contains("categoryName") || !j.contains("index")) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Missing categoryName or index";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::string category = j["categoryName"].get<std::string>();
            std::size_t index    = j["index"].get<std::size_t>();

            bool ok = backend.deleteOtherRecord(token, category, index);
            if (!ok) {
                json err;
                err["status"] = "error";
                err["errorMessage"] = "Failed to delete other record";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json out;
            out["status"]  = "ok";
            out["message"] = "Other record deleted";
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = std::string("Invalid JSON: ") + e.what();
            res.status = 400;
            res.set_content(err.dump(), "application/json");
        }
    });

    // GET /other/categories
    svr.Get("/other/categories", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        auto cats = backend.getOtherCategories(token);

        json out;
        out["status"]     = "ok";
        out["categories"] = cats;
        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    // GET /other/get_records?category=xxx
    svr.Get("/other/get_records", [&backend](const httplib::Request& req, httplib::Response& res) {
        std::string token = getTokenFromHeader(req);
        if (token.empty()) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing X-Auth-Token header";
            res.status = 401;
            res.set_content(err.dump(), "application/json");
            return;
        }

        if (!req.has_param("category")) {
            json err;
            err["status"] = "error";
            err["errorMessage"] = "Missing category param";
            res.status = 400;
            res.set_content(err.dump(), "application/json");
            return;
        }

        std::string category = req.get_param_value("category");
        auto records = backend.getOtherRecords(token, category);

        json out;
        out["status"]  = "ok";
        out["records"] = json::array();
        for (const auto& r : records) {
            json jr;
            jr["date"]  = r.date;
            jr["value"] = r.value;
            jr["note"]  = r.note;
            out["records"].push_back(jr);
        }

        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    std::cout << "Server started at http://0.0.0.0:8080\n";
    svr.listen("0.0.0.0", 8080);

    return 0;
}