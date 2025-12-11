# Transport Schedule Management and Visualization System

A complete transport schedule management system with C++ backend, Python web frontend, and Python desktop application.

## Project Structure

```
project_root/
├── backend_cpp/          # C++ REST API backend
│   ├── CMakeLists.txt
│   ├── src/
│   │   ├── main.cpp
│   │   ├── database.cpp
│   │   ├── api.cpp
│   │   └── simulation.cpp
│   ├── include/
│   │   ├── database.h
│   │   ├── api.h
│   │   └── simulation.h
│   └── transport.db      # SQLite database (created on first run)
├── frontend_py/          # Python web frontend (Flask)
│   ├── app.py
│   ├── requirements.txt
│   ├── templates/
│   │   ├── base.html
│   │   ├── index.html
│   │   ├── routes.html
│   │   └── admin.html
│   └── static/
│       └── style.css
├── desktop_client/       # Python desktop app (PySide6)
│   ├── main.py
│   └── requirements.txt
└── README.md
```

## Features

- **C++ Backend**: REST API with SQLite database, real-time transport simulation
- **Web Frontend**: Interactive map with Leaflet.js, route visualization, admin panel
- **Desktop App**: Native desktop client with embedded map and live updates
- **Real-time Simulation**: Vehicles move between stops with realistic physics
- **Multiple Transport Types**: Buses, trams, and trolleybuses
- **Fictional City District**: 8 stops with meaningful names

## Prerequisites

### Backend (C++)

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.15 or higher
- SQLite3 development libraries
- pkg-config (for finding SQLite3)

**On Ubuntu/Debian:**
```bash
sudo apt-get install build-essential cmake libsqlite3-dev pkg-config
```

**On Windows:**
- Install Visual Studio 2019 or later with C++ support
- Install CMake from https://cmake.org/
- SQLite3 will be linked automatically (via CMake)

**On macOS:**
```bash
brew install cmake sqlite3 pkg-config
```

### Frontend (Python)

- Python 3.8 or higher
- pip

### Desktop Client (Python)

- Python 3.8 or higher
- pip
- PySide6 (will be installed via requirements.txt)

## Building and Running

### 1. Build the C++ Backend

```bash
cd backend_cpp
mkdir build
cd build
cmake ..
make  # On Windows: use cmake --build . or open in Visual Studio
```

**On Windows (Visual Studio):**
```bash
cd backend_cpp
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

The executable will be created as `transport_backend` (or `transport_backend.exe` on Windows).

### 2. Run the C++ Backend

```bash
# From the build directory
./transport_backend [port]

# Default port is 8080
./transport_backend 8080
```

The backend will:
- Create `transport.db` SQLite database if it doesn't exist
- Initialize tables and insert sample data
- Start the simulation
- Listen on the specified port (default: 8080)

**Sample Data:**
- 8 stops: Central Square, Library, City Park, Old Town, Shopping Mall, University, Hospital, Train Station
- 5 routes: Route 1 (bus), Route 2 (tram), Route 3 (trolleybus), Route 4 (bus), Route 5 (tram)
- 8 vehicles distributed across routes

### 3. Run the Python Web Frontend

```bash
cd frontend_py
pip install -r requirements.txt

# Set backend URL (optional, defaults to http://localhost:8080)
export BACKEND_URL=http://localhost:8080

# Run Flask server
python app.py
```

The web frontend will be available at `http://localhost:5000` (or the port specified in the PORT environment variable).

**Pages:**
- `/` - Interactive map with live vehicle positions
- `/routes` - List of all routes with stop information
- `/admin` - Admin panel for managing stops, routes, and vehicles

### 4. Run the Desktop Application

```bash
cd desktop_client
pip install -r requirements.txt

# Set backend URL (optional, defaults to http://localhost:8080)
export BACKEND_URL=http://localhost:8080

# Run desktop app
python main.py
```

**Features:**
- Embedded map with live vehicle tracking
- Routes list with details
- Admin panel for viewing vehicles
- Refresh button
- Adjustable update interval
- Start/Stop simulation controls

## API Endpoints

The C++ backend exposes the following REST API endpoints:

### Public Endpoints

- `GET /api/stops` - Get all stops
- `GET /api/routes` - Get all routes with stop sequences
- `GET /api/transport` - Get all transport units
- `GET /api/transport/live` - Get current positions of all vehicles

### Admin Endpoints

- `POST /api/admin/stop` - Create or update a stop
  ```json
  {
    "id": 0,  // 0 for new, existing ID for update
    "name": "Stop Name",
    "x": 50.0,
    "y": 50.0
  }
  ```

- `POST /api/admin/route` - Create or update a route
  ```json
  {
    "id": 0,
    "name": "Route Name",
    "type": "bus|tram|trolleybus",
    "stop_ids": [1, 2, 3, 4]
  }
  ```

- `POST /api/admin/transport` - Create or update a vehicle
  ```json
  {
    "id": 0,
    "route_id": 1,
    "type": "bus|tram|trolleybus",
    "avg_speed": 40.0,
    "route_name": "Route 1"
  }
  ```

All endpoints return JSON and support CORS.

## Database Schema

### Tables

**stops**
- `id` (INTEGER PRIMARY KEY)
- `name` (TEXT)
- `x` (REAL) - X coordinate
- `y` (REAL) - Y coordinate

**routes**
- `id` (INTEGER PRIMARY KEY)
- `name` (TEXT)
- `type` (TEXT) - "bus", "tram", or "trolleybus"

**route_stops**
- `id` (INTEGER PRIMARY KEY)
- `route_id` (INTEGER) - Foreign key to routes
- `stop_id` (INTEGER) - Foreign key to stops
- `order_index` (INTEGER) - Order of stop in route

**vehicles**
- `id` (INTEGER PRIMARY KEY)
- `route_id` (INTEGER) - Foreign key to routes
- `type` (TEXT) - "bus", "tram", or "trolleybus"
- `avg_speed` (REAL) - Average speed in km/h
- `route_name` (TEXT) - Display name

## Simulation Logic

The simulation runs in a background thread and updates vehicle positions every 100ms:

1. Each vehicle follows its assigned route
2. Vehicles move between stops at their average speed
3. When a vehicle reaches a stop, it waits 5 seconds (dwell time)
4. Vehicles loop back to the first stop after completing the route
5. Positions are updated in real-time and exposed via `/api/transport/live`

## Configuration

### Environment Variables

**Backend:**
- None required (port can be passed as command-line argument)

**Frontend:**
- `BACKEND_URL` - C++ backend URL (default: `http://localhost:8080`)
- `PORT` - Flask server port (default: `5000`)

**Desktop Client:**
- `BACKEND_URL` - C++ backend URL (default: `http://localhost:8080`)

## Troubleshooting

### Backend won't compile

- Ensure you have C++17 compatible compiler
- Check that CMake can find SQLite3: `pkg-config --modversion sqlite3`
- On Windows, ensure Visual Studio C++ tools are installed

### Backend won't start

- Check if port 8080 is already in use
- Ensure SQLite3 is properly installed
- Check database file permissions

### Frontend can't connect to backend

- Verify backend is running: `curl http://localhost:8080/api/stops`
- Check `BACKEND_URL` environment variable
- Ensure firewall allows connections

### Desktop app shows blank map

- Ensure backend is running and accessible
- Check browser console in QWebEngineView (may require debugging)
- Verify internet connection (for OpenStreetMap tiles)

## Development Notes

- The coordinate system uses arbitrary units (not real GPS coordinates)
- Simulation speed can be adjusted by modifying `delta_time` in `simulation.cpp`
- Vehicle icons are emoji-based for simplicity
- The system is designed for a fictional city district

## License

This project is provided as-is for educational purposes.

## Author

Transport Schedule Management System - Course Project

