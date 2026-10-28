# Components List and GPIO Connections

## Main Controller
- ESP32-WROOM-32D development board

## Sensors
| Component | ESP32 GPIO |
|---|---|
| Soil moisture sensor 1 (analog) | GPIO 32 |
| Soil moisture sensor 2 (analog) | GPIO 33 |
| Soil moisture sensor 3 (analog) | GPIO 35 |
| Soil moisture sensor 4 (analog) | GPIO 36 |
| Rain sensor digital output (DO) | GPIO 19 |
| Rain sensor analog output (AO) | GPIO 34 |
| HC-SR04 trigger (TRIG) | GPIO 5 |
| HC-SR04 echo (ECHO) | GPIO 18 |

## Alert Devices
| Component | ESP32 GPIO |
|---|---|
| LED | GPIO 2 |
| Flood warning buzzer | GPIO 25 |
| Soil moisture warning buzzer | GPIO 14 |

## Communication Modules
| Module | ESP32 Connection |
|---|---|
| GPS TX | GPIO 16 (ESP32 RX) |
| GPS RX | GPIO 17 (ESP32 TX) |
| SIM800L TXD | GPIO 26 (ESP32 RX) |
| SIM800L RXD | GPIO 27 (ESP32 TX) |
| Wi-Fi dashboard | ESP32 built-in Wi-Fi |

## Wiring Notes
- Connect all module grounds to a common ground where required.
- Power each module according to its specifications.
- Keep ESP32 GPIO input voltages within the board's safe limits.
- The HC-SR04 ECHO output may be 5 V; use a suitable voltage divider or level shifter before GPIO 18.
- Power the SIM800L using a suitable supply capable of handling its current peaks. Do not power it directly from an ESP32 GPIO.
- Check the voltage requirements of all analog sensors before connecting their outputs to the ESP32 ADC pins.

## Important
Verify the physical wiring against this list. Update this file if the actual connections differ.
