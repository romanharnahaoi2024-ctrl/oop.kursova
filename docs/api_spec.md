# API Specification

## Base URL

Default: `http://localhost:8080`

## Endpoints

### GET /api/stops

Get all transport stops.

**Response:**
```json
[
  {
    "id": 1,
    "name": "Central Square",
    "x": 50.0,
    "y": 50.0
  },
  ...
]
```

**Status Codes:**
- `200 OK` - Success

---

### GET /api/routes

Get all transport routes.

**Response:**
```json
[
  {
    "id": 1,
    "name": "Route 1",
    "type": "bus",
    "stop_ids": [1, 2, 3, 5, 1]
  },
  ...
]
```

**Status Codes:**
- `200 OK` - Success

---

### GET /api/transport

Get all transport units (vehicles).

**Response:**
```json
[
  {
    "id": 1,
    "route_id": 1,
    "type": "bus",
    "avg_speed": 40.0,
    "route_name": "Route 1"
  },
  ...
]
```

**Status Codes:**
- `200 OK` - Success

---

### GET /api/transport/live

Get current live positions of all vehicles.

**Response:**
```json
[
  {
    "vehicle_id": 1,
    "x": 50.5,
    "y": 50.3,
    "current_stop_index": 0,
    "next_stop_index": 1,
    "route_name": "Route 1",
    "type": "bus",
    "progress": 0.25
  },
  ...
]
```

**Fields:**
- `vehicle_id` - Vehicle identifier
- `x`, `y` - Current coordinates
- `current_stop_index` - Index of current stop in route
- `next_stop_index` - Index of next stop
- `route_name` - Display name of route
- `type` - Vehicle type ("bus", "tram", "trolleybus")
- `progress` - Progress along current segment (0.0 to 1.0)

**Status Codes:**
- `200 OK` - Success

---

### POST /api/admin/stop

Create or update a stop.

**Request Body:**
```json
{
  "id": 0,  // 0 or omit for new stop, existing ID for update
  "name": "New Stop",
  "x": 55.0,
  "y": 60.0
}
```

**Response:**
```json
{
  "success": true
}
```

**Status Codes:**
- `200 OK` - Success
- `400 Bad Request` - Invalid data

---

### POST /api/admin/route

Create or update a route.

**Request Body:**
```json
{
  "id": 0,  // 0 or omit for new route
  "name": "Route 6",
  "type": "bus",
  "stop_ids": [1, 3, 5, 7]
}
```

**Response:**
```json
{
  "success": true
}
```

**Status Codes:**
- `200 OK` - Success
- `400 Bad Request` - Invalid data

---

### POST /api/admin/transport

Create or update a transport unit.

**Request Body:**
```json
{
  "id": 0,  // 0 or omit for new vehicle
  "route_id": 1,
  "type": "bus",
  "avg_speed": 45.0,
  "route_name": "Route 1"
}
```

**Response:**
```json
{
  "success": true
}
```

**Status Codes:**
- `200 OK` - Success
- `400 Bad Request` - Invalid data

---

## Error Responses

All endpoints may return error responses in the following format:

```json
{
  "error": true,
  "code": 400,
  "message": "Error description"
}
```

**Common Status Codes:**
- `400 Bad Request` - Invalid request data
- `500 Internal Server Error` - Server error

---

## CORS

All endpoints support CORS with the following headers:
- `Access-Control-Allow-Origin: *`
- `Access-Control-Allow-Methods: GET, POST, OPTIONS`
- `Access-Control-Allow-Headers: Content-Type`

---

## Rate Limiting

No rate limiting is currently implemented. For production use, consider adding rate limiting.

---

## Data Types

- **id**: Integer (positive)
- **name**: String (non-empty)
- **x, y**: Real number (coordinates)
- **type**: String ("bus", "tram", or "trolleybus")
- **avg_speed**: Real number (km/h, positive)
- **stop_ids**: Array of integers
- **progress**: Real number (0.0 to 1.0)

