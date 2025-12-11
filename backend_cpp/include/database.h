#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <memory>
#include <sqlite3.h>

struct Stop {
    int id;
    std::string name;
    double x;
    double y;
};

struct Route {
    int id;
    std::string name;
    std::string type; // "bus", "tram", "trolleybus"
    std::vector<int> stop_ids; // ordered list of stop IDs
};

struct Vehicle {
    int id;
    int route_id;
    std::string type;
    double avg_speed; // km/h
    std::string route_name;
};

struct RouteStop {
    int route_id;
    int stop_id;
    int order_index;
};

class Database {
public:
    Database(const std::string& db_path);
    ~Database();

    bool initialize();
    bool createTables();
    bool insertSampleData();

    // Stops
    std::vector<Stop> getAllStops();
    bool createOrUpdateStop(const Stop& stop);
    Stop getStopById(int id);

    // Routes
    std::vector<Route> getAllRoutes();
    bool createOrUpdateRoute(const Route& route);
    Route getRouteById(int id);
    std::vector<RouteStop> getRouteStops(int route_id);

    // Vehicles
    std::vector<Vehicle> getAllVehicles();
    bool createOrUpdateVehicle(const Vehicle& vehicle);
    Vehicle getVehicleById(int id);

private:
    std::string db_path_;
    sqlite3* db_;

    bool executeQuery(const std::string& query);
    bool executeQueryWithCallback(const std::string& query, 
                                   int (*callback)(void*, int, char**, char**), 
                                   void* data);
};

#endif // DATABASE_H

