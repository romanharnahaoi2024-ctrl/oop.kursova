#include "database.h"
#include <iostream>
#include <sstream>
#include <algorithm>

Database::Database(const std::string& db_path) : db_path_(db_path), db_(nullptr) {}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool Database::initialize() {
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    return createTables() && insertSampleData();
}

bool Database::createTables() {
    std::vector<std::string> queries = {
        "CREATE TABLE IF NOT EXISTS stops ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "x REAL NOT NULL,"
        "y REAL NOT NULL"
        ");",
        
        "CREATE TABLE IF NOT EXISTS routes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "type TEXT NOT NULL"
        ");",
        
        "CREATE TABLE IF NOT EXISTS route_stops ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "route_id INTEGER NOT NULL,"
        "stop_id INTEGER NOT NULL,"
        "order_index INTEGER NOT NULL,"
        "FOREIGN KEY(route_id) REFERENCES routes(id),"
        "FOREIGN KEY(stop_id) REFERENCES stops(id)"
        ");",
        
        "CREATE TABLE IF NOT EXISTS vehicles ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "route_id INTEGER NOT NULL,"
        "type TEXT NOT NULL,"
        "avg_speed REAL NOT NULL,"
        "route_name TEXT NOT NULL,"
        "FOREIGN KEY(route_id) REFERENCES routes(id)"
        ");"
    };

    for (const auto& query : queries) {
        if (!executeQuery(query)) {
            return false;
        }
    }
    return true;
}

bool Database::insertSampleData() {
    // Check if data already exists
    std::string check = "SELECT COUNT(*) FROM stops;";
    int count = 0;
    auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
        int* count = static_cast<int*>(data);
        if (argc > 0) {
            *count = std::stoi(argv[0]);
        }
        return 0;
    };
    
    char* errMsg = nullptr;
    sqlite3_exec(db_, check.c_str(), callback, &count, &errMsg);
    if (errMsg) {
        sqlite3_free(errMsg);
    }
    
    if (count > 0) {
        return true; // Data already exists
    }

    // Insert stops
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Central Square", {50.0, 50.0}},
        {"Library", {60.0, 45.0}},
        {"City Park", {70.0, 55.0}},
        {"Old Town", {45.0, 60.0}},
        {"Shopping Mall", {55.0, 65.0}},
        {"University", {65.0, 40.0}},
        {"Hospital", {40.0, 50.0}},
        {"Train Station", {50.0, 35.0}}
    };

    for (const auto& stop : stops) {
        std::ostringstream oss;
        oss << "INSERT INTO stops (name, x, y) VALUES ('"
            << stop.first << "', " << stop.second.first << ", " << stop.second.second << ");";
        if (!executeQuery(oss.str())) {
            return false;
        }
    }

    // Insert routes
    std::vector<std::pair<std::string, std::string>> routes = {
        {"Route 1", "bus"},
        {"Route 2", "tram"},
        {"Route 3", "trolleybus"},
        {"Route 4", "bus"},
        {"Route 5", "tram"}
    };

    for (const auto& route : routes) {
        std::ostringstream oss;
        oss << "INSERT INTO routes (name, type) VALUES ('"
            << route.first << "', '" << route.second << "');";
        if (!executeQuery(oss.str())) {
            return false;
        }
    }

    // Insert route_stops
    // Route 1: Central Square -> Library -> City Park -> Shopping Mall -> Central Square
    std::vector<std::pair<int, std::vector<int>>> routeStops = {
        {1, {1, 2, 3, 5, 1}}, // Route 1
        {2, {1, 4, 7, 1}},    // Route 2
        {3, {2, 6, 3, 8, 2}}, // Route 3
        {4, {5, 3, 6, 1, 5}}, // Route 4
        {5, {8, 1, 4, 5, 8}}  // Route 5
    };

    for (const auto& rs : routeStops) {
        int route_id = rs.first;
        for (size_t i = 0; i < rs.second.size(); ++i) {
            std::ostringstream oss;
            oss << "INSERT INTO route_stops (route_id, stop_id, order_index) VALUES ("
                << route_id << ", " << rs.second[i] << ", " << i << ");";
            if (!executeQuery(oss.str())) {
                return false;
            }
        }
    }

    // Insert vehicles
    std::vector<std::tuple<int, std::string, double, std::string>> vehicles = {
        {1, "bus", 40.0, "Route 1"},
        {2, "bus", 35.0, "Route 1"},
        {3, "tram", 30.0, "Route 2"},
        {4, "trolleybus", 32.0, "Route 3"},
        {5, "bus", 38.0, "Route 4"},
        {6, "tram", 28.0, "Route 5"},
        {7, "bus", 42.0, "Route 4"},
        {8, "trolleybus", 30.0, "Route 3"}
    };

    for (const auto& v : vehicles) {
        std::ostringstream oss;
        oss << "INSERT INTO vehicles (route_id, type, avg_speed, route_name) VALUES ("
            << std::get<0>(v) << ", '" << std::get<1>(v) << "', "
            << std::get<2>(v) << ", '" << std::get<3>(v) << "');";
        if (!executeQuery(oss.str())) {
            return false;
        }
    }

    return true;
}

bool Database::executeQuery(const std::string& query) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

std::vector<Stop> Database::getAllStops() {
    std::vector<Stop> stops;
    std::string query = "SELECT id, name, x, y FROM stops ORDER BY id;";
    
    auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
        std::vector<Stop>* stops = static_cast<std::vector<Stop>*>(data);
        Stop stop;
        stop.id = std::stoi(argv[0]);
        stop.name = argv[1];
        stop.x = std::stod(argv[2]);
        stop.y = std::stod(argv[3]);
        stops->push_back(stop);
        return 0;
    };
    
    char* errMsg = nullptr;
    sqlite3_exec(db_, query.c_str(), callback, &stops, &errMsg);
    if (errMsg) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    return stops;
}

Stop Database::getStopById(int id) {
    Stop stop = {0, "", 0.0, 0.0};
    std::ostringstream oss;
    oss << "SELECT id, name, x, y FROM stops WHERE id = " << id << ";";
    
    auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
        Stop* stop = static_cast<Stop*>(data);
        stop->id = std::stoi(argv[0]);
        stop->name = argv[1];
        stop->x = std::stod(argv[2]);
        stop->y = std::stod(argv[3]);
        return 0;
    };
    
    char* errMsg = nullptr;
    sqlite3_exec(db_, oss.str().c_str(), callback, &stop, &errMsg);
    if (errMsg) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    return stop;
}

bool Database::createOrUpdateStop(const Stop& stop) {
    std::ostringstream oss;
    if (stop.id > 0) {
        oss << "UPDATE stops SET name = '" << stop.name 
            << "', x = " << stop.x << ", y = " << stop.y 
            << " WHERE id = " << stop.id << ";";
    } else {
        oss << "INSERT INTO stops (name, x, y) VALUES ('"
            << stop.name << "', " << stop.x << ", " << stop.y << ");";
    }
    return executeQuery(oss.str());
}

std::vector<Route> Database::getAllRoutes() {
    std::vector<Route> routes;
    std::string query = "SELECT id, name, type FROM routes ORDER BY id;";
    
    auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
        std::vector<Route>* routes = static_cast<std::vector<Route>*>(data);
        Route route;
        route.id = std::stoi(argv[0]);
        route.name = argv[1];
        route.type = argv[2];
        routes->push_back(route);
        return 0;
    };
    
    char* errMsg = nullptr;
    sqlite3_exec(db_, query.c_str(), callback, &routes, &errMsg);
    if (errMsg) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    // Load stop_ids for each route
    for (auto& route : routes) {
        auto routeStops = getRouteStops(route.id);
        for (const auto& rs : routeStops) {
            route.stop_ids.push_back(rs.stop_id);
        }
    }
    
    return routes;
}

Route Database::getRouteById(int id) {
    Route route = {0, "", "", {}};
    std::ostringstream oss;
    oss << "SELECT id, name, type FROM routes WHERE id = " << id << ";";
    
    auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
        Route* route = static_cast<Route*>(data);
        route->id = std::stoi(argv[0]);
        route->name = argv[1];
        route->type = argv[2];
        return 0;
    };
    
    char* errMsg = nullptr;
    sqlite3_exec(db_, oss.str().c_str(), callback, &route, &errMsg);
    if (errMsg) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    auto routeStops = getRouteStops(id);
    for (const auto& rs : routeStops) {
        route.stop_ids.push_back(rs.stop_id);
    }
    
    return route;
}

std::vector<RouteStop> Database::getRouteStops(int route_id) {
    std::vector<RouteStop> routeStops;
    std::ostringstream oss;
    oss << "SELECT route_id, stop_id, order_index FROM route_stops WHERE route_id = "
        << route_id << " ORDER BY order_index;";
    
    auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
        std::vector<RouteStop>* routeStops = static_cast<std::vector<RouteStop>*>(data);
        RouteStop rs;
        rs.route_id = std::stoi(argv[0]);
        rs.stop_id = std::stoi(argv[1]);
        rs.order_index = std::stoi(argv[2]);
        routeStops->push_back(rs);
        return 0;
    };
    
    char* errMsg = nullptr;
    sqlite3_exec(db_, oss.str().c_str(), callback, &routeStops, &errMsg);
    if (errMsg) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    return routeStops;
}

bool Database::createOrUpdateRoute(const Route& route) {
    // This is simplified - in production, you'd want transaction handling
    int route_id = route.id;
    
    if (route.id > 0) {
        std::ostringstream oss;
        oss << "UPDATE routes SET name = '" << route.name 
            << "', type = '" << route.type << "' WHERE id = " << route.id << ";";
        if (!executeQuery(oss.str())) return false;
        
        // Delete old route_stops
        oss.str("");
        oss << "DELETE FROM route_stops WHERE route_id = " << route.id << ";";
        if (!executeQuery(oss.str())) return false;
    } else {
        std::ostringstream oss;
        oss << "INSERT INTO routes (name, type) VALUES ('"
            << route.name << "', '" << route.type << "');";
        if (!executeQuery(oss.str())) return false;
        route_id = sqlite3_last_insert_rowid(db_);
    }
    
    // Insert route_stops
    for (size_t i = 0; i < route.stop_ids.size(); ++i) {
        std::ostringstream oss;
        oss << "INSERT INTO route_stops (route_id, stop_id, order_index) VALUES ("
            << route_id << ", " << route.stop_ids[i] << ", " << i << ");";
        if (!executeQuery(oss.str())) return false;
    }
    
    return true;
}

std::vector<Vehicle> Database::getAllVehicles() {
    std::vector<Vehicle> vehicles;
    std::string query = "SELECT id, route_id, type, avg_speed, route_name FROM vehicles ORDER BY id;";
    
    auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
        std::vector<Vehicle>* vehicles = static_cast<std::vector<Vehicle>*>(data);
        Vehicle vehicle;
        vehicle.id = std::stoi(argv[0]);
        vehicle.route_id = std::stoi(argv[1]);
        vehicle.type = argv[2];
        vehicle.avg_speed = std::stod(argv[3]);
        vehicle.route_name = argv[4];
        vehicles->push_back(vehicle);
        return 0;
    };
    
    char* errMsg = nullptr;
    sqlite3_exec(db_, query.c_str(), callback, &vehicles, &errMsg);
    if (errMsg) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    return vehicles;
}

Vehicle Database::getVehicleById(int id) {
    Vehicle vehicle = {0, 0, "", 0.0, ""};
    std::ostringstream oss;
    oss << "SELECT id, route_id, type, avg_speed, route_name FROM vehicles WHERE id = " << id << ";";
    
    auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
        Vehicle* vehicle = static_cast<Vehicle*>(data);
        vehicle->id = std::stoi(argv[0]);
        vehicle->route_id = std::stoi(argv[1]);
        vehicle->type = argv[2];
        vehicle->avg_speed = std::stod(argv[3]);
        vehicle->route_name = argv[4];
        return 0;
    };
    
    char* errMsg = nullptr;
    sqlite3_exec(db_, oss.str().c_str(), callback, &vehicle, &errMsg);
    if (errMsg) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    return vehicle;
}

bool Database::createOrUpdateVehicle(const Vehicle& vehicle) {
    std::ostringstream oss;
    if (vehicle.id > 0) {
        oss << "UPDATE vehicles SET route_id = " << vehicle.route_id
            << ", type = '" << vehicle.type 
            << "', avg_speed = " << vehicle.avg_speed
            << ", route_name = '" << vehicle.route_name
            << "' WHERE id = " << vehicle.id << ";";
    } else {
        oss << "INSERT INTO vehicles (route_id, type, avg_speed, route_name) VALUES ("
            << vehicle.route_id << ", '" << vehicle.type << "', "
            << vehicle.avg_speed << ", '" << vehicle.route_name << "');";
    }
    return executeQuery(oss.str());
}

