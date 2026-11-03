## Hardware Components

* Arduino Uno
* Ultrasonic Sensor (HC-SR04)
* 3x Servo Motors
* Ignition Coil / High Voltage Igniter Module
* Relay Module
* Power Supply
* DIY Launcher Mechanism
* Connecting Wires

---

## Pin Configuration

| Component         | Arduino Pin |
| ----------------- | ----------- |
| Ultrasonic Echo   | 2           |
| Ultrasonic Trig   | 3           |
| Ignition Output   | 4           |
| Rotary Servo      | 9           |
| Arm Up/Down Servo | 10          |
| Radar Servo       | 11          |

---

## System Operation

1. System powers on and moves all servos to their **initial positions**.
2. The radar servo rotates the ultrasonic sensor from **0° to 180°** scanning the environment.
3. Distance readings are filtered to remove noise.
4. If an object is detected within **50 cm**, the system identifies it as a target.
5. The launcher rotates toward the detected direction.
6. The launcher arm moves upward to the firing position.
7. The ignition output activates briefly to launch the projectile.
8. The program stops after firing.

---

## Arduino Code

The complete Arduino program is included in this repository:

```
Missile_Launch_Full.ino
```

This code controls:

* Radar scanning
* Target detection
* Servo movement
* Ignition activation

---
## Wiring Diagram

![Wiring Diagram](Schematic_Diagram.jpg)
