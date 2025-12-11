# Quick Start Guide

## Prerequisites Check

Before starting, ensure you have:

1. **C++ Compiler**: `g++ --version` or `clang++ --version` (or Visual Studio on Windows)
2. **CMake**: `cmake --version` (3.15+)
3. **SQLite3**: `sqlite3 --version`
4. **Python 3**: `python3 --version` (3.8+)
5. **pip**: `pip --version`

## Step-by-Step Setup

### 1. Build C++ Backend (5 minutes)

**Linux/macOS:**
```bash
cd backend_cpp
mkdir build && cd build
cmake ..
make
```

**Windows:**
```bash
cd backend_cpp
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

### 2. Start Backend Server

**Linux/macOS:**
```bash
cd backend_cpp/build
./transport_backend
```

**Windows:**
```bash
cd backend_cpp\build\Release
transport_backend.exe
```

You should see:
```
Database initialized successfully
Simulation started
Starting server on port 8080
```

**Keep this terminal open!**

### 3. Start Web Frontend (New Terminal)

```bash
cd frontend_py
pip install -r requirements.txt
python app.py
```

Open browser: `http://localhost:5000`

### 4. Start Desktop App (Optional, New Terminal)

```bash
cd desktop_client
pip install -r requirements.txt
python main.py
```

## Verify Everything Works

1. **Backend**: Open `http://localhost:8080/api/stops` in browser - should see JSON
2. **Web Frontend**: Open `http://localhost:5000` - should see map with stops
3. **Desktop App**: Should show map with vehicles moving

## Troubleshooting

### Backend won't compile
- Check CMake version: `cmake --version` (need 3.15+)
- Install SQLite3 dev package: `sudo apt-get install libsqlite3-dev` (Linux)
- On Windows: Ensure Visual Studio C++ tools installed

### Backend won't start
- Check if port 8080 is in use: `netstat -an | grep 8080` (Linux) or `netstat -an | findstr 8080` (Windows)
- Try different port: `./transport_backend 8081`

### Frontend can't connect
- Verify backend is running: `curl http://localhost:8080/api/stops`
- Check `BACKEND_URL` environment variable
- Ensure firewall allows localhost connections

### Map shows no data
- Check browser console (F12) for errors
- Verify backend is running and accessible
- Ensure internet connection (for OpenStreetMap tiles)

## Next Steps

- Explore the map at `http://localhost:5000`
- View routes at `http://localhost:5000/routes`
- Try admin panel at `http://localhost:5000/admin`
- Watch vehicles move in real-time!

## Stopping Services

- **Backend**: Press `Ctrl+C` in backend terminal
- **Frontend**: Press `Ctrl+C` in frontend terminal
- **Desktop App**: Close the window

## Database Reset

To reset the database and start fresh:
```bash
cd backend_cpp
rm transport.db  # or delete transport.db on Windows
# Then restart backend - it will recreate the database
```

