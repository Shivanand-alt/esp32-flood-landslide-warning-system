# System Block Diagram

```text
  SOIL MOISTURE SENSORS (4)
              |
              v
       RAIN SENSOR
              |
              v
    ULTRASONIC WATER SENSOR
              |
              v
       +-------------+
       |    ESP32    |
       | Controller  |
       +-------------+
          |   |   |
          |   |   +------> Wi-Fi Dashboard
          |   |
          |   +----------> GPS Module
          |                    |
          |                    v
          |               Location Data
          |
          +--------------> GSM Module
          |                    |
          |                    v
          |                SMS Alerts
          |
          +--------------> LED and Buzzers
```

## Description
The sensors provide rainfall, soil moisture, and water-distance readings to the ESP32. The controller evaluates configured thresholds, calculates mean soil moisture, and activates warning outputs when applicable. Wi-Fi provides dashboard access, while GPS and GSM support location and SMS features when operational.
