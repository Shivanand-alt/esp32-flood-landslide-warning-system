# esp32-flood-landslide-warning-system
ESP32-based flood and landslide early warning system that monitors soil moisture, rainfall, and water levels. It calculates average soil moisture and triggers alerts using LEDs, buzzers, GPS, GSM SMS, and a Wi-Fi dashboard to support early hazard awareness.
# ESP32-Based Smart Flood and Landslide Early Warning System

## 1. Project Overview
This project is an ESP32-based prototype designed to monitor rainfall, soil moisture, and nearby water levels. It provides warning alerts when configured sensor conditions are detected.

## 2. Objectives
- Monitor soil moisture using four sensors.
- Detect rain using a rain sensor.
- Monitor water distance using an ultrasonic sensor.
- Display sensor readings on a Wi-Fi dashboard.
- Provide local alerts using an LED and buzzers.
- Send SMS alerts using a SIM800L module when configured conditions are met.
- Include GPS coordinates when a valid GPS fix is available.

## 3. Components
- ESP32 development board
- Four soil moisture sensors
- YL-83 rain sensor
- HC-SR04 ultrasonic sensor
- GPS module
- SIM800L GSM module
- LED and suitable resistor
- Buzzers
- Jumper wires and breadboard
- Power supply and prototype materials

## 4. Working Principle
The sensors provide readings to the ESP32. The controller calculates the mean of the four calibrated soil-moisture percentages and compares it with a configurable experimental threshold. It also checks rainfall detection and the ultrasonic sensor's measured distance. Depending on the configured conditions, the system activates alerts and presents readings on the dashboard. GSM and GPS features depend on module connectivity and successful testing.

## 5. Block Diagram
Soil Moisture Sensors + Rain Sensor + Ultrasonic Sensor → ESP32 → LED + Buzzers + Wi-Fi Dashboard + GSM SMS

GPS Module → ESP32 → Location information in alerts/dashboard

## 6. Software
- Arduino IDE
- ESP32 Arduino core
- TinyGPSPlus library
- ESP32 Wi-Fi and WebServer libraries

## 7. Testing
Record actual test results for:
- Soil sensors in dry and wet soil
- Rain sensor response
- Ultrasonic distance readings
- Mean moisture calculation
- LED and buzzer alerts
- Wi-Fi dashboard
- GPS fix and coordinates
- GSM SMS delivery

Do not mark a test as successful unless it has been performed.

## 8. Limitations
- Sensor readings depend on calibration, soil conditions, and placement.
- GSM alerts depend on power, SIM/network availability, and module operation.
- Wi-Fi dashboard access depends on network connectivity.
- Thresholds require experimental calibration.
- This prototype is not a certified flood or landslide prediction system.

## 9. Project Status
Prototype and feature status: Update this section with the components physically connected, features tested, and remaining work.

## 10. Media
Add links to actual prototype photos and demonstration videos here.

## 11. Safety Note
This project is an experimental early-warning prototype. It must not replace official emergency warnings or professional monitoring systems.
## Prototype Photos

![Prototype Setup](media/prototype_setup.jpg)

![Sensor Testing](media/sensor_testing.jpg)

## Working Demonstration

[Watch the project demonstration](media/working_demo.mp4)

## Project Status

Photos and videos document the prototype's actual assembly and testing progress. Features that have not yet been physically tested are marked as pending.
