from flask import Flask, render_template, jsonify, request
import requests
import os

app = Flask(__name__)

# Backend API URL - can be set via environment variable
BACKEND_URL = os.getenv('BACKEND_URL', 'http://localhost:8080')

def call_backend(endpoint, method='GET', data=None):
    """Helper function to call the C++ backend API"""
    url = f"{BACKEND_URL}{endpoint}"
    try:
        if method == 'GET':
            response = requests.get(url, timeout=5)
        elif method == 'POST':
            response = requests.post(url, json=data, timeout=5)
        else:
            return None
        
        if response.status_code == 200:
            return response.json()
        else:
            return None
    except requests.exceptions.RequestException as e:
        print(f"Error calling backend: {e}")
        return None

@app.route('/')
def index():
    """Home page with map"""
    return render_template('index.html')

@app.route('/routes')
def routes():
    """Routes page"""
    return render_template('routes.html')

@app.route('/admin')
def admin():
    """Admin page"""
    return render_template('admin.html')

# API endpoints that proxy to backend
@app.route('/api/stops')
def api_stops():
    """Get all stops"""
    data = call_backend('/api/stops')
    if data is not None:
        return jsonify(data)
    return jsonify([]), 500

@app.route('/api/routes')
def api_routes():
    """Get all routes"""
    data = call_backend('/api/routes')
    if data is not None:
        return jsonify(data)
    return jsonify([]), 500

@app.route('/api/transport')
def api_transport():
    """Get all transport units"""
    data = call_backend('/api/transport')
    if data is not None:
        return jsonify(data)
    return jsonify([]), 500

@app.route('/api/transport/live')
def api_transport_live():
    """Get live transport positions"""
    data = call_backend('/api/transport/live')
    if data is not None:
        return jsonify(data)
    return jsonify([]), 500

@app.route('/api/admin/stop', methods=['POST'])
def api_admin_stop():
    """Create or update a stop"""
    data = request.get_json()
    result = call_backend('/api/admin/stop', method='POST', data=data)
    if result is not None:
        return jsonify(result)
    return jsonify({'error': 'Failed to create/update stop'}), 500

@app.route('/api/admin/route', methods=['POST'])
def api_admin_route():
    """Create or update a route"""
    data = request.get_json()
    result = call_backend('/api/admin/route', method='POST', data=data)
    if result is not None:
        return jsonify(result)
    return jsonify({'error': 'Failed to create/update route'}), 500

@app.route('/api/admin/transport', methods=['POST'])
def api_admin_transport():
    """Create or update a transport unit"""
    data = request.get_json()
    result = call_backend('/api/admin/transport', method='POST', data=data)
    if result is not None:
        return jsonify(result)
    return jsonify({'error': 'Failed to create/update transport'}), 500

if __name__ == '__main__':
    port = int(os.getenv('PORT', 5000))
    app.run(host='0.0.0.0', port=port, debug=True)

