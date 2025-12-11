import sys
import os
import json
import requests
from PySide6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                               QHBoxLayout, QPushButton, QLabel, QListWidget, 
                               QListWidgetItem, QSplitter, QGroupBox, QSpinBox,
                               QTabWidget, QTableWidget, QTableWidgetItem, QMessageBox)
from PySide6.QtWebEngineWidgets import QWebEngineView
from PySide6.QtCore import QTimer, Qt, QUrl
from PySide6.QtGui import QIcon
from datetime import datetime
from PySide6.QtWidgets import (QTextEdit, QFormLayout, QLineEdit, QComboBox)    

# Backend API URL
BACKEND_URL = os.getenv('BACKEND_URL', 'http://localhost:8080')


class TransportDesktopApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.stops = []
        self.routes = []
        self.vehicles = []
        self.vehicle_positions = []
        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.updateLiveData)
        self.update_interval = 2000  # 2 seconds
        
        self.initUI()
        self.loadInitialData()
        self.startAutoUpdate()
    
    def initUI(self):
        self.setWindowTitle('Transport Schedule Management - Desktop Client')
        self.setGeometry(100, 100, 1400, 900)
        
        # Central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # Main layout
        main_layout = QHBoxLayout(central_widget)
        
        # Splitter for resizable panels
        splitter = QSplitter(Qt.Horizontal)
        main_layout.addWidget(splitter)
        
        # Left panel - Controls and info
        left_panel = QWidget()
        left_layout = QVBoxLayout(left_panel)
        
        # Control buttons
        control_group = QGroupBox("Controls")
        control_layout = QVBoxLayout()
        
        self.refresh_btn = QPushButton("🔄 Refresh")
        self.refresh_btn.clicked.connect(self.loadInitialData)
        control_layout.addWidget(self.refresh_btn)
        
        self.start_btn = QPushButton("▶ Start Simulation")
        self.start_btn.clicked.connect(self.startSimulation)
        control_layout.addWidget(self.start_btn)
        
        self.stop_btn = QPushButton("⏸ Stop Simulation")
        self.stop_btn.clicked.connect(self.stopSimulation)
        self.stop_btn.setEnabled(False)
        control_layout.addWidget(self.stop_btn)
        
        # Update interval
        interval_layout = QHBoxLayout()
        interval_layout.addWidget(QLabel("Update Interval (ms):"))
        self.interval_spin = QSpinBox()
        self.interval_spin.setMinimum(500)
        self.interval_spin.setMaximum(10000)
        self.interval_spin.setValue(2000)
        self.interval_spin.setSingleStep(500)
        self.interval_spin.valueChanged.connect(self.changeUpdateInterval)
        interval_layout.addWidget(self.interval_spin)
        control_layout.addLayout(interval_layout)
        
        control_group.setLayout(control_layout)
        left_layout.addWidget(control_group)
        
        # Routes list
        routes_group = QGroupBox("Routes")
        routes_layout = QVBoxLayout()
        self.routes_list = QListWidget()
        self.routes_list.itemClicked.connect(self.onRouteSelected)
        routes_layout.addWidget(self.routes_list)
        routes_group.setLayout(routes_layout)
        left_layout.addWidget(routes_group)
        
        # Status
        status_group = QGroupBox("Status")
        status_layout = QVBoxLayout()
        self.status_label = QLabel("Ready")
        status_layout.addWidget(self.status_label)
        status_group.setLayout(status_layout)
        left_layout.addWidget(status_group)
        
        left_layout.addStretch()
        splitter.addWidget(left_panel)
        
        # Right panel - Map and details
        right_panel = QWidget()
        right_layout = QVBoxLayout(right_panel)
        
        # Tabs for map and admin
        tabs = QTabWidget()
        
        # Map tab
        map_tab = QWidget()
        map_layout = QVBoxLayout(map_tab)
        self.map_view = QWebEngineView()
        self.initMap()
        map_layout.addWidget(self.map_view)
        tabs.addTab(map_tab, "🗺️ Map")

        # 2. Log Tab 
        log_tab = QWidget()
        log_layout = QVBoxLayout(log_tab)
        self.log_area = QTextEdit()
        self.log_area.setReadOnly(True)
            
        self.log_area.setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: Consolas; font-size: 12px;")
        log_layout.addWidget(self.log_area)
        tabs.addTab(log_tab, "📝 Session Log")

        # 3. Add Route Tab 
        create_route_tab = QWidget()
        route_form_layout = QFormLayout(create_route_tab)
        
        self.route_name_input = QLineEdit()
        self.route_type_input = QComboBox()
        self.route_type_input.addItems(["bus", "tram", "trolleybus"])
        self.route_stops_input = QLineEdit()
        self.route_stops_input.setPlaceholderText("IDs через кому: 1, 2, 3")
        
        btn_add_route = QPushButton("Save Route")
        btn_add_route.clicked.connect(self.submitNewRoute)
        
        route_form_layout.addRow("Route Name:", self.route_name_input)
        route_form_layout.addRow("Type:", self.route_type_input)
        route_form_layout.addRow("Stop IDs:", self.route_stops_input)
        route_form_layout.addRow(btn_add_route)
        
        tabs.addTab(create_route_tab, "➕ Add Route")
        
        # Admin tab
        admin_tab = QWidget()
        admin_layout = QVBoxLayout(admin_tab)
        
        # Admin table for vehicles
        admin_table_label = QLabel("Vehicles")
        admin_layout.addWidget(admin_table_label)
        self.admin_table = QTableWidget()
        self.admin_table.setColumnCount(5)
        self.admin_table.setHorizontalHeaderLabels(["ID", "Route ID", "Type", "Speed", "Route Name"])
        admin_layout.addWidget(self.admin_table)
        
        tabs.addTab(admin_tab, "⚙️ Admin")
        
        right_layout.addWidget(tabs)
        splitter.addWidget(right_panel)
        
        # Set splitter proportions
        splitter.setStretchFactor(0, 1)
        splitter.setStretchFactor(1, 3)
    
    def initMap(self):
        """Initialize the web map view with Lviv coordinates and route lines"""
        html_content = """
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
            <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
            <style>
                body { margin: 0; padding: 0; }
                #map { height: 100vh; width: 100%; }
                .vehicle-icon { 
                    background: transparent; border: none; text-align: center; 
                    transition: all 0.2s;
                }
            </style>
        </head>
        <body>
            <div id="map"></div>
            <script>
                // === КОНСТАНТИ ЛЬВОВА ===
                const LVIV_LAT = 49.8397;
                const LVIV_LON = 24.0297;
                const SCALE = 0.005;

                // Функція конвертації (Умовні координати -> GPS Львова)
                function convertCoord(x, y) {
                    return [
                        LVIV_LAT + (y - 50) * SCALE,
                        LVIV_LON + (x - 50) * SCALE
                    ];
                }

                // Центруємо карту на ЛЬВОВІ
                var map = L.map('map').setView([LVIV_LAT, LVIV_LON], 14);
                
                L.tileLayer('https://{s}.basemaps.cartocdn.com/rastertiles/voyager/{z}/{x}/{y}{r}.png', {
                    attribution: '© OpenStreetMap contributors'
                }).addTo(map);

                var stopMarkers = {};
                var vehicleMarkers = {};
                var routeLines = [];

                // --- МАЛЮВАННЯ ЛІНІЙ (МАРШРУТИ) ---
                window.drawRoutes = function(routes, stops) {
                    // Очистка старих ліній
                    routeLines.forEach(function(line) { map.removeLayer(line); });
                    routeLines = [];

                    routes.forEach(function(route) {
                        if (!route.stop_ids || route.stop_ids.length < 2) return;

                        var latlngs = [];
                        route.stop_ids.forEach(function(stopId) {
                            var stop = stops.find(s => s.id === stopId);
                            if (stop) {
                                // ВИКОРИСТОВУЄМО КОНВЕРТАЦІЮ
                                latlngs.push(convertCoord(stop.x, stop.y));
                            }
                        });

                        var color = '#3388ff';
                        if (route.type === 'tram') color = '#28a745';
                        else if (route.type === 'trolleybus') color = '#fd7e14';

                        if (latlngs.length > 1) {
                            var polyline = L.polyline(latlngs, {
                                color: color,
                                weight: 4,
                                opacity: 0.7,
                                lineJoin: 'round'
                            }).addTo(map);
                            routeLines.push(polyline);
                        }
                    });
                };

                // --- МАЛЮВАННЯ ЗУПИНОК ---
                function updateStops(stops) {
                    stops.forEach(function(stop) {
                        if (!stopMarkers[stop.id]) {
                            // КОНВЕРТАЦІЯ
                            var coords = convertCoord(stop.x, stop.y);
                            stopMarkers[stop.id] = L.marker(coords)
                                .addTo(map)
                                .bindPopup('<b>' + stop.name + '</b>');
                        }
                    });
                }
                
                // --- МАЛЮВАННЯ ТРАНСПОРТУ ---
                function updateVehicles(positions) {
                    positions.forEach(function(pos) {
                        var icon = pos.type === 'bus' ? '🚌' : 
                                   pos.type === 'tram' ? '🚊' : '🚎';
                        
                        // КОНВЕРТАЦІЯ
                        var coords = convertCoord(pos.x, pos.y);

                        if (!vehicleMarkers[pos.vehicle_id]) {
                            vehicleMarkers[pos.vehicle_id] = L.marker(coords, {
                                icon: L.divIcon({
                                    className: 'vehicle-icon',
                                    html: '<div style="font-size: 24px;">' + icon + '</div>',
                                    iconSize: [30, 30]
                                })
                            }).addTo(map);
                        } else {
                            vehicleMarkers[pos.vehicle_id].setLatLng(coords);
                            // Оновити іконку, якщо змінився тип (на всяк випадок)
                            // ...
                        }
                    });
                }
                
                window.updateMap = function(stops, positions) {
                    updateStops(stops);
                    updateVehicles(positions);
                };
            </script>
        </body>
        </html>
        """
        self.map_view.setHtml(html_content)
    
    def loadInitialData(self):
        """Load initial data from backend"""
        try:
            self.status_label.setText("Loading...")
            
            # Load stops
            response = requests.get(f"{BACKEND_URL}/api/stops", timeout=5)
            if response.status_code == 200:
                self.stops = response.json()
            else:
                self.showError("Failed to load stops")
                return
            
            # Load routes
            response = requests.get(f"{BACKEND_URL}/api/routes", timeout=5)
            if response.status_code == 200:
                self.routes = response.json()
                self.updateRoutesList()
            else:
                self.showError("Failed to load routes")
                return
            
            # Load vehicles
            response = requests.get(f"{BACKEND_URL}/api/transport", timeout=5)
            if response.status_code == 200:
                self.vehicles = response.json()
                self.updateAdminTable()
            else:
                self.showError("Failed to load vehicles")
                return

            self.updateLiveData()
            self.drawRoutesOnMap()
            self.status_label.setText("Connected")
            
        except requests.exceptions.RequestException as e:
            self.showError(f"Connection error: {str(e)}")
            self.status_label.setText("Disconnected")
    
    def updateRoutesList(self):
        """Update the routes list widget"""
        self.routes_list.clear()
        for route in self.routes:
            icon = "🚌" if route['type'] == 'bus' else "🚊" if route['type'] == 'tram' else "🚎"
            item = QListWidgetItem(f"{icon} {route['name']} ({route['type']})")
            item.setData(Qt.UserRole, route)
            self.routes_list.addItem(item)
    
    def updateAdminTable(self):
        """Update the admin table with vehicle data"""
        self.admin_table.setRowCount(len(self.vehicles))
        for i, vehicle in enumerate(self.vehicles):
            self.admin_table.setItem(i, 0, QTableWidgetItem(str(vehicle['id'])))
            self.admin_table.setItem(i, 1, QTableWidgetItem(str(vehicle['route_id'])))
            self.admin_table.setItem(i, 2, QTableWidgetItem(vehicle['type']))
            self.admin_table.setItem(i, 3, QTableWidgetItem(str(vehicle['avg_speed'])))
            self.admin_table.setItem(i, 4, QTableWidgetItem(vehicle['route_name']))
    
    def updateLiveData(self):
        """Update live vehicle positions"""
        try:
            response = requests.get(f"{BACKEND_URL}/api/transport/live", timeout=5)
            if response.status_code == 200:
                self.vehicle_positions = response.json()
                self.updateMap()
            else:
                self.status_label.setText("Error updating positions")
        except requests.exceptions.RequestException as e:
            self.status_label.setText(f"Connection error: {str(e)}")
    
    def updateMap(self):
        """Update the map with current data"""
        stops_js = json.dumps(self.stops)
        positions_js = json.dumps(self.vehicle_positions)
        script = f"window.updateMap({stops_js}, {positions_js});"
        self.map_view.page().runJavaScript(script)

    def drawRoutesOnMap(self):
        """Send routes data to the map to draw lines"""
        if not self.routes or not self.stops:
            return
            
        import json
        routes_js = json.dumps(self.routes)
        stops_js = json.dumps(self.stops)
        
        # Передаємо дані в JS
        script = f"if (window.drawRoutes) window.drawRoutes({routes_js}, {stops_js});"
        self.map_view.page().runJavaScript(script)
    
    def onRouteSelected(self, item):
        """Handle route selection"""
        route = item.data(Qt.UserRole)
        if route:
            msg = f"Route: {route['name']}\nType: {route['type']}\nStops: {len(route.get('stop_ids', []))}"
            QMessageBox.information(self, "Route Information", msg)
    
    def startSimulation(self):
        """Start simulation via API and logging"""
        try:
            # Відправка команди на бекенд
            resp = requests.post(f"{BACKEND_URL}/api/simulation/control", json={"action": "start"})
            
            if resp.status_code == 200:
                self.start_btn.setEnabled(False)
                self.stop_btn.setEnabled(True)
                self.status_label.setText("Simulation running")
                
                # Очищення логу (Нова сесія)
                self.log_area.clear() 
                self.simulation_start_time = datetime.now()
                
                self.log_message(f"=== SIMULATION SESSION STARTED ===")
                self.log_message(f"Time: {self.simulation_start_time.strftime('%H:%M:%S')}")
                self.log_message("System: Backend signal sent. Vehicles moving.")
            else:
                self.showError("Failed to start backend simulation")
                
        except Exception as e:
            self.showError(f"Connection error: {str(e)}")
    
    def stopSimulation(self):
        """Stop simulation via API and logging"""
        try:
            resp = requests.post(f"{BACKEND_URL}/api/simulation/control", json={"action": "stop"})
            
            if resp.status_code == 200:
                self.start_btn.setEnabled(True)
                self.stop_btn.setEnabled(False)
                self.status_label.setText("Simulation paused")
                
                # Запис підсумків сесії
                end_time = datetime.now()
                duration = end_time - self.simulation_start_time
                
                self.log_message("System: Backend paused.")
                self.log_message(f"=== SIMULATION STOPPED ===")
                self.log_message(f"Time: {end_time.strftime('%H:%M:%S')}")
                self.log_message(f"⏱ DURATION: {duration}")
            else:
                self.showError("Failed to stop backend simulation")
                
        except Exception as e:
            self.showError(f"Connection error: {str(e)}")

    def log_message(self, msg):
        self.log_area.append(msg)
    
    def startAutoUpdate(self):
        """Start automatic updates"""
        self.update_timer.start(self.update_interval)
    
    def changeUpdateInterval(self, value):
        """Change the update interval"""
        self.update_interval = value
        self.update_timer.setInterval(value)
    
    def showError(self, message):
        """Show an error message"""
        QMessageBox.critical(self, "Error", message)

    def submitNewRoute(self):
        try:
            name = self.route_name_input.text()
            r_type = self.route_type_input.currentText()
            stops_str = self.route_stops_input.text()
            
            # Парсинг ID (з перевіркою)
            try:
                stop_ids = [int(s.strip()) for s in stops_str.split(',') if s.strip()]
            except ValueError:
                self.showError("Невірний формат ID. Використовуйте числа через кому: 1, 2, 3")
                return

            if not name or not stop_ids:
                self.showError("Заповніть всі поля")
                return

            payload = {
                "name": name,
                "type": r_type,
                "stop_ids": stop_ids
            }
            
            # Відправка на сервер
            response = requests.post(f"{BACKEND_URL}/api/admin/route", json=payload)
            
            if response.status_code == 200:
                QMessageBox.information(self, "Success", f"Route '{name}' created!")
                self.route_name_input.clear()
                self.route_stops_input.clear()
                # Оновити дані, щоб побачити новий маршрут
                self.loadInitialData() 
                self.log_message(f"[Admin] New route added: {name}")
            else:
                self.showError("Failed to add route")
                
        except Exception as e:
            self.showError(f"Error: {str(e)}")


def main():
    app = QApplication(sys.argv)
    window = TransportDesktopApp()
    window.show()
    sys.exit(app.exec())


if __name__ == '__main__':
    main()

