#include "database.h"
#include "api.h"
#include "simulation.h"
#include <iostream>
#include <memory>
#include <signal.h>

std::shared_ptr<Simulation> g_simulation = nullptr;

void signalHandler(int signum) {
    std::cout << "\nShutting down..." << std::endl;
    if (g_simulation) {
        g_simulation->stop();
    }
    exit(signum);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    const std::string db_path = "transport.db";
    int port = 8080;
    
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    
    // Initialize database
    auto db = std::make_shared<Database>(db_path);
    if (!db->initialize()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }
    
    std::cout << "Database initialized successfully" << std::endl;
    
    // Initialize simulation
    g_simulation = std::make_shared<Simulation>(db);
    g_simulation->start();
    std::cout << "Simulation started" << std::endl;
    
    // Initialize API server
    APIServer server(db, g_simulation);
    server.setupRoutes();
    
    // Run server (blocking)
    server.run(port);
    
    return 0;
}

