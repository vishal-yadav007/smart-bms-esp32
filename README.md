# Smart Battery Management System Using CAN and IoT

An IoT-based battery monitoring and protection system built around two ESP32 boards connected over a CAN bus. The receiver collects battery data, monitors temperature, controls a cooling fan, and uploads everything to Firebase for display on a web dashboard.

---

## 📌 Project Overview

The *Smart Battery Management System (BMS)* monitors important battery parameters in real time.

It uses *ESP32, **MCP2515 CAN communication, voltage and current sensors, a **DHT22 temperature sensor, relay-based cooling control, and **Firebase Realtime Database*. The monitored data can be viewed through a web-based dashboard.

---
## 🎯 Objectives

- Monitor battery voltage in real time
- Measure battery charging/discharging current
- Calculate battery power
- Estimate battery State of Charge (SOC)
- Monitor temperature and humidity
- Automatically control a cooling fan based on temperature
- Transfer battery data using CAN communication
- Store monitoring data in Firebase
- Display battery parameters on an IoT dashboard
- Generate warnings for critical battery conditions

---

## 🏗️ System Architecture

text
                 3S Li-ion Battery
                        │
              ┌─────────┴─────────┐
              │                   │
        Voltage Sensor       Current Sensor
              │                   │
              └─────────┬─────────┘
                        │
                 ESP32 Transmitter
                        │
                     MCP2515
                        │
                    CAN Bus
                  CAN-H / CAN-L
                        │
                     MCP2515
                        │
                 ESP32 Receiver
                  ┌─────┼─────┐
                  │     │     │
                DHT22 Relay  Wi-Fi
                  │     │     │
            Temperature Fan   │
                  │           │
                  └─────┬─────┘
                        │
                    Firebase
                        │
                        ▼
                 Web Dashboard


---

Overall, the project demonstrates how CAN communication, IoT, cloud monitoring, and intelligent battery protection can be combined to create a practical and scalable smart BMS prototype.
