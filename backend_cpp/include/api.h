#ifndef API_H
#define API_H

#include <string>
#include <memory>
#include "database.h"
#include "simulation.h"

// Forward declaration
namespace httplib {
    class Server;
}

class APIServer {
public:
    APIServer(std::shared_ptr<Database> db, std::shared_ptr<Simulation> sim);
    ~APIServer();
    void setupRoutes();
    void run(int port = 8080);

private:
    std::shared_ptr<Database> db_;
    std::shared_ptr<Simulation> sim_;
    httplib::Server* server_;

    // Helper methods for JSON responses
    std::string jsonError(const std::string& message, int code = 400);
    std::string jsonSuccess(const std::string& data = "{}");
};

#endif // API_H

