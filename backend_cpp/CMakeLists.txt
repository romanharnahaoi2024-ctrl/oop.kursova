cmake_minimum_required(VERSION 3.15)
project(TransportBackend VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Include directories
include_directories(include)
include_directories(third_party)

# ------------------------------------------------------------
# SQLite3 — add the library FIRST and EXPLICITLY set language
# ------------------------------------------------------------
add_library(sqlite3 STATIC sqlite3.c)
set_target_properties(sqlite3 PROPERTIES LINKER_LANGUAGE C)
target_include_directories(sqlite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ------------------------------------------------------------
# FetchContent dependencies (AFTER sqlite3)
# ------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.3
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# ------------------------------------------------------------
# Source files
# ------------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/database.cpp
    src/api.cpp
    src/simulation.cpp
)

set(HEADERS
    include/database.h
    include/api.h
    include/simulation.h
)

# ------------------------------------------------------------
# Create executable
# ------------------------------------------------------------
add_executable(transport_backend ${SOURCES} ${HEADERS})

# ------------------------------------------------------------
# Link libraries (order matters)
# ------------------------------------------------------------
target_link_libraries(transport_backend 
    httplib::httplib
    nlohmann_json::nlohmann_json
    sqlite3
)

# pthread for UNIX
if (UNIX)
    target_link_libraries(transport_backend pthread)
endif()

# ------------------------------------------------------------
# Compiler flags
# ------------------------------------------------------------
target_compile_options(transport_backend PRIVATE -Wall -Wextra)
