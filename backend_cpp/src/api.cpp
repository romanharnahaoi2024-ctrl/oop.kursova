#include "api.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

APIServer::APIServer(std::shared_ptr<Database> db, std::shared_ptr<Simulation> sim)
    : db_(db), sim_(sim), server_(new httplib::Server()) {}

APIServer::~APIServer() {
    if (server_) {
        delete server_;
    }
}

void APIServer::setupRoutes() {
    httplib::Server* svr = server_;
    
    // Enable CORS та Content-Encoding
    svr->set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"},
        {"Cache-Control", "no-cache, max-age=0"}
    });
    
    svr->set_post_routing_handler([](const httplib::Request&, httplib::Response&) {
        return httplib::Server::HandlerResponse::Handled;
    });
    
    // Handle OPTIONS requests
    svr->Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
        return;
    });
    
    // GET /api/stops
    svr->Get("/api/stops", [this](const httplib::Request&, httplib::Response& res) {
        try {
            auto stops = db_->getAllStops();
            json j = json::array();
            for (const auto& stop : stops) {
                j.push_back({
                    {"id", stop.id},
                    {"name", stop.name},
                    {"x", stop.x},
                    {"y", stop.y}
                });
            }
            res.set_content(j.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(jsonError(e.what(), 500), "application/json");
        }
    });
    
    // GET /api/routes
    svr->Get("/api/routes", [this](const httplib::Request&, httplib::Response& res) {
        try {
            auto routes = db_->getAllRoutes();
            json j = json::array();
            for (const auto& route : routes) {
                json route_json = {
                    {"id", route.id},
                    {"name", route.name},
                    {"type", route.type},
                    {"stop_ids", route.stop_ids}
                };
                j.push_back(route_json);
            }
            res.set_content(j.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(jsonError(e.what(), 500), "application/json");
        }
    });
    
    // GET /api/transport
    svr->Get("/api/transport", [this](const httplib::Request&, httplib::Response& res) {
        try {
            auto vehicles = db_->getAllVehicles();
            json j = json::array();
            for (const auto& vehicle : vehicles) {
                j.push_back({
                    {"id", vehicle.id},
                    {"route_id", vehicle.route_id},
                    {"type", vehicle.type},
                    {"avg_speed", vehicle.avg_speed},
                    {"route_name", vehicle.route_name}
                });
            }
            res.set_content(j.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(jsonError(e.what(), 500), "application/json");
        }
    });
    
    // GET /api/transport/live
    svr->Get("/api/transport/live", [this](const httplib::Request&, httplib::Response& res) {
        try {
            auto positions = sim_->getLivePositions();
            json j = json::array();
            for (const auto& pos : positions) {
                j.push_back({
                    {"vehicle_id", pos.vehicle_id},
                    {"x", pos.x},
                    {"y", pos.y},
                    {"current_stop_index", pos.current_stop_index},
                    {"next_stop_index", pos.next_stop_index},
                    {"route_name", pos.route_name},
                    {"type", pos.type},
                    {"progress", pos.progress}
                });
            }
            res.set_content(j.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(jsonError(e.what(), 500), "application/json");
        }
    });
    
    // POST /api/admin/stop
    svr->Post("/api/admin/stop", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            Stop stop;
            if (body.contains("id")) {
                stop.id = body["id"];
            }
            stop.name = body["name"];
            stop.x = body["x"];
            stop.y = body["y"];
            
            if (db_->createOrUpdateStop(stop)) {
                res.set_content(jsonSuccess(), "application/json");
            } else {
                res.status = 400;
                res.set_content(jsonError("Failed to create/update stop"), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(jsonError(e.what(), 400), "application/json");
        }
    });
    
    // POST /api/admin/route
    svr->Post("/api/admin/route", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            Route route;
            if (body.contains("id")) {
                route.id = body["id"];
            }
            route.name = body["name"];
            route.type = body["type"];
            route.stop_ids = body["stop_ids"].get<std::vector<int>>();
            
            if (db_->createOrUpdateRoute(route)) {
                res.set_content(jsonSuccess(), "application/json");
            } else {
                res.status = 400;
                res.set_content(jsonError("Failed to create/update route"), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(jsonError(e.what(), 400), "application/json");
        }
    });
    
    // POST /api/simulation/control
    svr->Post("/api/simulation/control", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            std::string action = body["action"];
            
            if (action == "start") {
                sim_->resume();
            } else if (action == "stop") {
                sim_->pause();
            }
            
            res.set_content(jsonSuccess(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(jsonError(e.what()), "application/json");
        }
    });
    
    // GET /api/simulation/status
    svr->Get("/api/simulation/status", [this](const httplib::Request&, httplib::Response& res) {
        try {
            json j = {
                {"running", sim_->isRunning()},
                {"paused", sim_->isPaused()}
            };
            res.set_content(j.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(jsonError(e.what(), 500), "application/json");
        }
    });
    
    // POST /api/admin/transport
    svr->Post("/api/admin/transport", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            Vehicle vehicle;
            if (body.contains("id")) {
                vehicle.id = body["id"];
            }
            vehicle.route_id = body["route_id"];
            vehicle.type = body["type"];
            vehicle.avg_speed = body["avg_speed"];
            vehicle.route_name = body["route_name"];
            
            if (db_->createOrUpdateVehicle(vehicle)) {
                res.set_content(jsonSuccess(), "application/json");
            } else {
                res.status = 400;
                res.set_content(jsonError("Failed to create/update vehicle"), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(jsonError(e.what(), 400), "application/json");
        }
    });
}

void APIServer::run(int port) {
    if (!server_) {
        std::cerr << "Server not initialized" << std::endl;
        return;
    }
    
    std::cout << "Starting server on port " << port << std::endl;
    server_->listen("0.0.0.0", port);
}

std::string APIServer::jsonError(const std::string& message, int code) {
    json j = {
        {"error", true},
        {"code", code},
        {"message", message}
    };
    return j.dump();
}

std::string APIServer::jsonSuccess(const std::string& data) {
    if (data == "{}") {
        json j = {
            {"success", true}
        };
        return j.dump();
    }
    return data;
}

