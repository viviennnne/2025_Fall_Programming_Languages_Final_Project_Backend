# Health Tracker (Backend)

> This is a final project for the course DBME 2008 程式語言 at NTU.

- Health Backend is a C++ health tracking server that provides RESTful APIs for managing user health data, including water intake, sleep, activity, BMI, and custom biomedical-related records.

- The backend is designed to work with a front-end dashboard and stores data locally using JSON files.

## Build
 ```txt
g++ -std=c++17 \
    server.cpp \
    backend/HealthBackend.cpp \
    helpers/Logger.cpp \
    -o server_app
./server_app
```

Expected output:
```txt
 Server started at http://0.0.0.0:8080
 ```

## Frontend
- [2025_Fall_Programming_Languages_Final_Project_Frontend](https://github.com/CoinVeil4065852/2025_Fall_Programming_Languages_Final_Project_Frontend.git) is the official backend for this project. Go to the page to see more.

## Data Storage 
- All data is stored in a JSON file:
```txt
data/storage.json
```

## API Endpoints Overview
### Authentication and User
| Method | Endpoint | Description |
|-------|---------|-------------|
| POST | /register | Register a new user |
| POST | /login | Login and get authentication token |
| GET | /user/profile | Get user profile information |
### Water Records
| Method | Endpoint | Description |
|-------|---------|-------------|
| GET | /user/bmi | Calculate and get BMI |
| GET | /waters | Get all water intake records |
| POST | /waters | Add a water intake record |
| PATCH | /waters/{id} | Update a water intake record |
| DELETE | /waters/{id} | Delete a water intake record |
### Sleep Records
| Method | Endpoint | Description |
|-------|---------|-------------|
| GET | /sleeps | Get all sleep records |
| POST | /sleeps | Add a sleep record |
| PATCH | /sleeps/{id} | Update a sleep record |
| DELETE | /sleeps/{id} | Delete a sleep record |
### Activity Records
| Method | Endpoint | Description |
|-------|---------|-------------|
| GET | /activities | Get all activity records |
| POST | /activities | Add an activity record |
| PATCH | /activities/{id} | Update an activity record |
| DELETE | /activities/{id} | Delete an activity record |
### Custom Categories
| Method | Endpoint | Description |
|-------|---------|-------------|
| GET | /category/list | List all custom categories |
| POST | /category/create | Create a new custom category |
| DELETE | /category/{categoryId} | Delete a category |
| GET | /category/{categoryId}/list | Get items in a category |
| POST | /category/{categoryId}/add | Add item to category |
| PATCH | /category/{categoryId}/{itemId} | Update category item |
| DELETE | /category/{categoryId}/{itemId} | Delete category item |
















