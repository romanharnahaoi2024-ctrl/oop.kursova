#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include "database.h"
#include "transport_models.h" 

struct VehiclePosition {
    int vehicle_id;
    double x;
    double y;
    int current_stop_index;
    int next_stop_index;
    std::string route_name;
    std::string type;
    double progress; // 0.0 to 1.0 along current segment
};

class Simulation {
public:
    Simulation(std::shared_ptr<Database> db);
    ~Simulation();

    void start();
    void stop();
    bool isRunning() const;
    void pause() { paused_ = true; }
    void resume() { paused_ = false; }
    bool isPaused() const { return paused_; }

    std::vector<VehiclePosition> getLivePositions();
    VehiclePosition getVehiclePosition(int vehicle_id);

private:
    std::shared_ptr<Database> db_;
    std::atomic<bool> running_;
    std::atomic<bool> paused_;
    std::thread simulation_thread_;
    std::mutex positions_mutex_;

    struct VehicleState {
        int vehicle_id;
        int route_id;
        std::vector<int> stop_ids;
        int current_stop_idx;
        double x;
        double y;
        double target_x;
        double target_y;
        double speed; // km/h
        double progress; // 0.0 to 1.0
        bool forward; // direction
        double dwell_time; // seconds remaining at stop

        std::shared_ptr<ITransportModel> model; // Поліморфний об'єкт (Композиція)  
    };

    std::map<int, VehicleState> vehicle_states_;
    std::map<int, VehiclePosition> live_positions_;

    void simulationLoop();
    void updateVehicle(VehicleState& state, double delta_time);
    void initializeVehicles();
    std::pair<double, double> getStopCoordinates(int stop_id);
};

#endif // SIMULATION_H

