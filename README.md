# Health Tracker (Backend)

> This is a final project for the course DBME 2008 程式語言 at NTU.

- Health Backend is a C++ health tracking server that provides RESTful APIs for managing user health data, including water intake, sleep, activity, BMI, and custom biomedical-related records.

- The backend is designed to work with a front-end dashboard and stores data locally using JSON files.

## Build
- g++ -std=c++17 server.cpp backend/HealthBackend.cpp -o server_app
- ./server_app
- Expected output: Server started at http://0.0.0.0:8080

## Frontend
### Server
- [2025_Fall_Programming_Languages_Final_Project_Frontend](https://github.com/CoinVeil4065852/2025_Fall_Programming_Languages_Final_Project_Frontend.git) is the official backend for this project. Go to the page to see more.

## Folder Structure

```
health_backend/
├── main.cpp                    # Core backend testing (no HTTP)
├── server.cpp                  # HTTP server entry point (REST API)
├── httplib.h                   # cpp-httplib (header-only HTTP library)
│
├── external/
│   └── json.hpp                 # nlohmann JSON header-only library
│
├── backend/
│   ├── HealthBackend.hpp
│   └── HealthBackend.cpp
│
├── user/
│   ├── User.hpp
│   ├── User.cpp
│   ├── UserBackend.hpp
│   └── UserBackend.cpp
│
├── records/
│   ├── Water.hpp
│   ├── Water.cpp
│   ├── Sleep.hpp
│   ├── Sleep.cpp
│   ├── Activity.hpp
│   ├── Activity.cpp
│   ├── OtherCategory.hpp
│   └── OtherCategory.cpp
│
├── helpers/
│   ├── validation.hpp
│   ├── validation.cpp
│   └── json.hpp                 # (replaced by nlohmann/json)
│
└── data/
    ├── storage.json             # Auto-generated persistent storage
    └── storage.example.json     # Example layout (no real data)
```

---

- `main.cpp` is for backend logic testing (no HTTP).
- `server.cpp` is the REST API entry point.
- Data is persisted to `data/storage.json`.
- JSON parsing is implemented with `nlohmann/json` (header-only).
