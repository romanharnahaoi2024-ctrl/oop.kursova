#!/bin/bash
# Quick start script for C++ backend

cd "$(dirname "$0")"

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure and build
cmake ..
make

# Run the backend
echo "Starting backend server on port 8080..."
./transport_backend 8080

