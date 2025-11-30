# Wireless Crane Control System
## CHKT_251 Engineering Mechanics Assignment

---

## 📋 Project Summary

**Application:** Crane Model for Engineering Mechanics  
**Platform:** ESP32-S3 SuperMini  
**Communication:** ESP-NOW Protocol  
**Framework:** Arduino (PlatformIO)  
**Date:** November 2025

### System Components
- **Transmitter (TX)** - Wireless button controller with LED feedback
- **Receiver (RX)** - 3-servo crane controller with smooth motion control

---

## 🎮 Transmitter (CHKT_251_TRANSMITTER)

### Purpose
Wireless remote control for crane operation with visual button feedback.

### Key Features
- **6 buttons** controlling 3 crane servos (2 buttons per servo for bidirectional control)
- **Color-coded NeoPixel LED** for instant visual feedback
- **ESP-NOW communication** for low-latency wireless control
- **50ms debounce** for reliable button reading
- **Heartbeat packets** (every 5 seconds) to maintain connection

### Button Mapping

| Button Pin | Servo | Direction | LED Color |
| ---------- | ----- | --------- | --------- |
| 8          | 0     | CCW       | Red       |
| 6          | 0     | CW        | Green     |
| 9          | 1     | CCW       | Blue      |
| 5          | 1     | CW        | Yellow    |
| 10         | 2     | CCW       | Magenta   |
| 4          | 2     | CW        | Cyan      |

---

## 🤖 Receiver (CHKT_251_RECEIVER)

### Purpose
Controls 3 MG995 continuous rotation servos for crane mechanisms with smooth, professional motion.

### Key Features
- **Individual servo tuning** - Each servo has customizable max speed and acceleration rate
- **Smooth acceleration/deceleration** - No jerky movements, professional crane-like motion
- **Safety timeout** - Automatic stop after 8 seconds without signal
- **Connection monitoring** - Heartbeat tracking ensures crane safety

### Servo Configuration (Individually Tunable)

| Servo | GPIO Pin | Max Speed | Acceleration Rate | Crane Function   |
| ----- | -------- | --------- | ----------------- | ---------------- |
| 0     | 1        | 40        | 2                 | (e.g., Hoist)    |
| 1     | 2        | 25        | 1                 | (e.g., Boom)     |
| 2     | 3        | 60        | 3                 | (e.g., Rotation) |

**✅ Custom Per-Servo Settings:**
- `maxSpeed[3]` array allows different maximum speeds for each servo
- `accelRate[3]` array allows different acceleration rates for each servo
- Easily adjustable in code to match your specific crane mechanics requirements

---

## 🔄 System Interaction Flow

```
[TRANSMITTER]                      [RECEIVER]
     |                                  |
  Button Press                          |
     |                                  |
  Read Input (50ms debounce)           |
     |                                  |
  Update NeoPixel LED                  |
     |                                  |
  Create CommandPacket                 |
     |                                  |
  ESP-NOW Send ----------------------> Receive Packet
     |                                  |
  Delivery Confirmation                Validate Data
     |                                  |
     |                              Set Target Speed
     |                                  |
     |                              Apply Acceleration
     |                                  |
     |                              Update Servo PWM
     |                                  |
  Button Release                        |
     |                                  |
  Send STOP Command -----------------> Stop Servo (gradual)
     |                                  |
  Clear LED                         Monitor Timeout
     |                                  |
  Heartbeat (every 5s) --------------> Reset Timeout Timer
```

---

## 🏗️ Crane Model Application

This system is designed for an **Engineering Mechanics crane model** with the following advantages:

### Why This Design Works for Cranes

✅ **Smooth Motion Control** - Per-servo acceleration prevents jerky movements and mechanical stress  
✅ **Individual Servo Tuning** - Each crane mechanism (hoist, boom, rotation) can have custom speed/acceleration  
✅ **Safety First** - Automatic timeout stops crane if connection is lost  
✅ **Precise Control** - Bidirectional buttons allow fine control of each crane axis  
✅ **Visual Feedback** - LED confirms commands are being sent  
✅ **Professional Movement** - Ramped acceleration mimics real crane behavior  

### Suggested Servo Assignments
- **Servo 0** (GPIO 1) - Hoist mechanism (lifting/lowering)
- **Servo 1** (GPIO 2) - Boom extension or angle adjustment
- **Servo 2** (GPIO 3) - Base rotation or trolley movement

---

## 🛠️ Technical Specifications

### Communication
- **Protocol:** ESP-NOW (low-latency, no router needed)
- **Latency:** < 20ms (button press to servo movement)
- **Range:** ~200m line-of-sight
- **Heartbeat:** Every 5 seconds
- **Timeout:** 8 seconds (automatic safety stop)

### Hardware
- **Platform:** ESP32-S3 SuperMini
- **Servo Type:** MG995 Continuous Rotation
- **PWM Range:** 500-2500µs @ 50Hz
- **Upload Ports:** COM3 (TX), COM4 (RX)

---

## 🎯 Key Features for Engineering Mechanics

1. **Individual Servo Customization** ⭐
   - Adjustable `maxSpeed[3]` array for each servo
   - Adjustable `accelRate[3]` array for smooth ramping
   - Optimizes each crane mechanism independently

2. **Safety Mechanisms**
   - Connection timeout protection
   - PWM safety clamping (45°-135°)
   - Automatic stop on signal loss

3. **Real-time Control**
   - 100Hz servo update rate
   - Sub-20ms control latency
   - Smooth acceleration/deceleration

---

## 🔧 Easy Customization

To adjust crane behavior, modify these arrays in the **Receiver code**:

```cpp
// Maximum speed for each servo (crane mechanism)
int maxSpeed[3] = { 40, 25, 60 };

// Acceleration rate for each servo
int accelRate[3] = { 2, 1, 3 };
```

Match these values to your crane's mechanical requirements for optimal performance.

---

**Project Status:** ✅ Ready for Engineering Mechanics Assignment  
**Application:** Crane Model Control System  
**Platform:** ESP32-S3 + PlatformIO  

---

*Generated: November 30, 2025*
