# Fault-Tolerant Embedded Runtime System

## Overview

The Fault-Tolerant Embedded Runtime System is an ESP32-based embedded safety controller designed to provide reliable runtime fault detection, fault isolation, and autonomous recovery for battery monitoring applications.

The system continuously monitors four battery cell voltages, detects hardware and software faults, transitions between multiple runtime operating modes, and maintains structured timestamped fault logs while ensuring uninterrupted operation of healthy modules.

This project demonstrates an automotive-inspired fault management architecture implemented using a fully non-blocking event-driven software design.

---

## Features

- Fully Event-Driven Architecture
- 100% Non-Blocking Firmware
- millis()-Based Runtime Scheduler
- Four Battery Cell Monitoring
- Sensor Disconnection Detection
- Invalid ADC Reading Detection
- Frozen ADC Detection
- Relay Feedback Verification
- Relay Mismatch Detection
- Fault Isolation
- Runtime State Machine
- Automatic Recovery
- Timestamped Fault Logging
- LCD Runtime Dashboard
- Serial Diagnostic Dashboard
- Buzzer Alarm
- Relay Protection

---

## Runtime Modes

The firmware automatically transitions between the following operating modes:

### NORMAL

- All sensors healthy
- Relay ON
- System operational

### DEGRADED

- One or more sensor faults detected
- Healthy modules continue operating
- Partial system functionality maintained

### FAILSAFE

- Critical faults detected
- Relay disconnected
- Buzzer activated
- LCD warning displayed

### SHUTDOWN

- Severe system failure
- Complete protection mode
- Relay permanently OFF until recovery

---

## Fault Detection

The runtime continuously checks for:

- Sensor Disconnection
- Invalid ADC Readings
- Frozen ADC Conditions
- Relay Feedback Mismatch

Each detected fault is logged with:

- Timestamp
- Fault Type
- Module Number

---

## Hardware Used

- ESP32 Development Board
- 4 × Potentiometers (Battery Cell Simulation)
- I2C LCD 16×2 Display
- Relay Module
- Active Buzzer

---

## Software Used

- Arduino IDE
- ESP32 Arduino Core
- Wokwi Simulator
- GitHub
- GitHub Pages

---

## Runtime Scheduler

The firmware executes independent software tasks using millis() scheduling.

Tasks include:

- Sensor Sampling
- Fault Detection
- Runtime State Update
- Relay Verification
- LCD Refresh
- Serial Dashboard
- Buzzer Control

No delay() function is used anywhere in the firmware.

---

## Project Structure

```
Fault-Tolerant-Embedded-Runtime-System/

├── sketch.ino
├── diagram.json
├── libraries.txt
├── README.md
├── index.html
├── style.css
└── script.js
```

---

## Wokwi Simulation

https://wokwi.com/projects/468414981553584129

---

## GitHub Repository

https://github.com/bikash-cloud/Fault-Tolerant-Embedded-Runtime-System

---

## Website

https://bikash-cloud.github.io/Fault-Tolerant-Embedded-Runtime-System/

---

## Example Runtime Output

```
Runtime Mode : NORMAL

Relay : ON

Healthy Sensors : 4/4

Fault Logs :

No Faults
```

Example Fault

```
Runtime Mode : FAILSAFE

Relay : OFF

Fault :
Sensor Disconnect

Timestamp :
18 seconds
```

---

## Applications

- Battery Management Systems
- Automotive Embedded Systems
- Functional Safety Demonstration
- Runtime Monitoring
- Industrial Controllers
- Embedded Diagnostics
- Safety Critical Embedded Systems

---

## Future Improvements

- CAN Bus Diagnostics
- EEPROM Fault Storage
- SD Card Logging
- RTC Timestamp Support
- Watchdog Integration
- Dual MCU Redundancy
- OTA Diagnostics
- Cloud Monitoring

---

## Author

**BIKASH SWAIN**

B.Tech – Electronics & Communication Engineering

Embedded Systems | ESP32 | STM32 | IoT | Automotive Embedded Systems

LinkedIn

https://www.linkedin.com/in/bikash-swain-b75961322

GitHub

https://github.com/bikash-cloud

Email

swainbikash524@gmail.com

---

© 2026 BIKASH SWAIN

All Rights Reserved.
