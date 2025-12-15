#include "simulation.h"
#include <cmath>
#include <iostream>

Simulation::Simulation(std::shared_ptr<Database> db) 
    : db_(db), running_(false), paused_(true) { // Починаємо з ПАУЗИ (paused_ = true)
    initializeVehicles();
}

Simulation::~Simulation() {
    stop();
}

void Simulation::start() {
    if (running_) return;
    running_ = true;
    simulation_thread_ = std::thread(&Simulation::simulationLoop, this);
}

void Simulation::stop() {
    if (!running_) return;
    running_ = false;
    if (simulation_thread_.joinable()) {
        simulation_thread_.join();
    }
}

bool Simulation::isRunning() const {
    return running_;
}

void Simulation::initializeVehicles() {
    auto vehicles = db_->getAllVehicles();
    std::lock_guard<std::mutex> lock(positions_mutex_);
    
    vehicle_states_.clear();
    
    for (const auto& vehicle : vehicles) {
        auto route = db_->getRouteById(vehicle.route_id);
        if (route.stop_ids.empty()) continue;
        
        VehicleState state;
        state.vehicle_id = vehicle.id;
        state.route_id = vehicle.route_id;
        state.stop_ids = route.stop_ids;
        state.current_stop_idx = 0;
        state.speed = vehicle.avg_speed;
        state.forward = true;
        state.dwell_time = 0.0;
        state.progress = 0.0;
   
        if (vehicle.type == "tram") {
            state.model = std::make_shared<TramModel>();
        } else if (vehicle.type == "trolleybus") {
            state.model = std::make_shared<TrolleybusModel>();
        } else {
            state.model = std::make_shared<BusModel>();
        }
    
        // Start at first stop
        auto coords = getStopCoordinates(state.stop_ids[0]);
        state.x = coords.first;
        state.y = coords.second;
        
        // Set target to next stop
        if (state.stop_ids.size() > 1) {
            auto next_coords = getStopCoordinates(state.stop_ids[1]);
            state.target_x = next_coords.first;
            state.target_y = next_coords.second;
        } else {
            state.target_x = state.x;
            state.target_y = state.y;
        }
        
        vehicle_states_[vehicle.id] = state;
    }
}

std::pair<double, double> Simulation::getStopCoordinates(int stop_id) {
    auto stop = db_->getStopById(stop_id);
    return {stop.x, stop.y};
}

void Simulation::simulationLoop() {
    const double delta_time = 0.1; // 100ms per tick
    const double dwell_time_at_stop = 5.0; // seconds
    
    while (running_) {
        
        if (paused_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        auto start_time = std::chrono::steady_clock::now();
        
        {
            std::lock_guard<std::mutex> lock(positions_mutex_);
            
            for (auto& [vehicle_id, state] : vehicle_states_) {
                updateVehicle(state, delta_time);
                
                // Update live positions
                VehiclePosition pos;
                pos.vehicle_id = vehicle_id;
                pos.x = state.x;
                pos.y = state.y;
                pos.current_stop_index = state.current_stop_idx;
                pos.next_stop_index = (state.current_stop_idx + 1) % state.stop_ids.size();
                pos.progress = state.progress;
                
                auto vehicle = db_->getVehicleById(vehicle_id);
                pos.route_name = vehicle.route_name;
                pos.type = vehicle.type;
                
                live_positions_[vehicle_id] = pos;
            }
        }
        
        // Sleep to maintain ~10Hz update rate
        auto end_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count();
        int sleep_ms = std::max(0, 100 - static_cast<int>(elapsed));
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
}

void Simulation::updateVehicle(VehicleState& state, double delta_time) {
    if (state.dwell_time > 0.0) {
        state.dwell_time -= delta_time;
        return;
    }
    
    // Calculate distance to target
    double dx = state.target_x - state.x;
    double dy = state.target_y - state.y;
    double distance = std::sqrt(dx * dx + dy * dy);
    
    // Convert speed from km/h to units per second (assuming 1 unit = 100m)
    double speed_units_per_sec = (state.speed / 3.6) / 0.1; // km/h -> m/s -> units/s
    
    if (distance < 0.5) { // Reached stop (within 0.5 units)
        // Arrived at stop
        state.x = state.target_x;
        state.y = state.target_y;
        state.progress = 0.0;
        state.dwell_time = state.model->getDwellTime(); // Use model's dwell time
        
        // Move to next stop
        if (state.forward) {
            state.current_stop_idx++;
            if (state.current_stop_idx >= static_cast<int>(state.stop_ids.size())) {
                // Loop back to start
                state.current_stop_idx = 0;
            }
        } else {
            state.current_stop_idx--;
            if (state.current_stop_idx < 0) {
                state.current_stop_idx = state.stop_ids.size() - 1;
            }
        }
        
        // Set next target
        int next_idx = state.forward 
            ? ((state.current_stop_idx + 1) % state.stop_ids.size())
            : ((state.current_stop_idx - 1 + state.stop_ids.size()) % state.stop_ids.size());
        
        auto next_coords = getStopCoordinates(state.stop_ids[next_idx]);
        state.target_x = next_coords.first;
        state.target_y = next_coords.second;
    } else {
        // Move towards target
        double move_distance = speed_units_per_sec * delta_time;
        double ratio = std::min(1.0, move_distance / distance);
        
        state.x += dx * ratio;
        state.y += dy * ratio;
        
        // Update progress (0.0 to 1.0)
        double total_distance = std::sqrt(
            std::pow(state.target_x - getStopCoordinates(state.stop_ids[state.current_stop_idx]).first, 2) +
            std::pow(state.target_y - getStopCoordinates(state.stop_ids[state.current_stop_idx]).second, 2)
        );
        double traveled = total_distance - distance;
        state.progress = std::min(1.0, std::max(0.0, traveled / total_distance));
    }
}

std::vector<VehiclePosition> Simulation::getLivePositions() {
    std::lock_guard<std::mutex> lock(positions_mutex_);
    std::vector<VehiclePosition> positions;
    for (const auto& [vehicle_id, pos] : live_positions_) {
        positions.push_back(pos);
    }
    return positions;
}

VehiclePosition Simulation::getVehiclePosition(int vehicle_id) {
    std::lock_guard<std::mutex> lock(positions_mutex_);
    auto it = live_positions_.find(vehicle_id);
    if (it != live_positions_.end()) {
        return it->second;
    }
    return VehiclePosition{0, 0.0, 0.0, 0, 0, "", "", 0.0};
}

