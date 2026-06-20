# OptiComm: Advanced ESP8266 Li-Fi Communication System

## Project Overview
Li-Fi (Light Fidelity) is an innovative technology that transmits data using light rather than radio waves. This project implements a simplex (one-way) optical communication link using two ESP8266 microcontrollers. 

The **Transmitter** hosts a sleek, custom-built web server where users can type messages. These characters are translated into binary data and flashed as precise pulses using a laser diode. Across the room, the **Receiver** uses a Light Dependent Resistor (LDR) to detect these microscopic flashes, reconstructs the binary back into text, and displays the live message on an OLED screen.

---

## Hardware Requirements & Setup

### The Transmitter Component
* **Microcontroller:** NodeMCU ESP8266
* **Light Source:** Standard 5V Laser Diode Module
* **Wiring:** Connect the Laser Diode's signal pin to **D5 (GPIO 14)** on the NodeMCU.

### The Receiver Component
* **Microcontroller:** NodeMCU ESP8266
* **Sensor:** LDR (Light Dependent Resistor) Module
* **Display:** 128x64 SSD1306 OLED Display (I2C)
* **Wiring:** * Connect the LDR signal pin to **D5 (GPIO 14)**.
  * Connect the OLED to standard I2C pins (SDA, SCL) with the I2C address set to `0x3C`.

---

## Software & Dependencies

This project is written in standard C++ for the Arduino IDE. Before compiling, ensure you have the ESP8266 board manager installed, along with the following libraries:
* `ESP8266WiFi.h` & `ESP8266WebServer.h` (Built-in for Transmitter)
* `Wire.h` (Built-in I2C for Receiver)
* `Adafruit_GFX.h` (For graphics rendering)
* `Adafruit_SSD1306.h` (For the specific OLED hardware)

---

## The Science: Encoding & Decoding Data

This project does not rely on pre-packaged serial communication libraries; the optical modulation is custom-built using strict micro-timing. 

### 1. Signal Timing (Baud Rate)
The entire system operates on a synchronized **50ms Bit Period** (equivalent to 20 bits per second). 

### 2. The Transmitter (Encoding)
The laser uses "Active-LOW" logic, meaning a `LOW` signal turns the laser **ON**, and a `HIGH` signal turns it **OFF**. When a message is sent, the code loops through every character and breaks it down into standard 8-bit ASCII. For each character, it sends:
1. **Start Bit:** Laser turns ON for 50ms.
2. **Data Bits:** Sends the 8 binary bits (Least Significant Bit first), pulsing the laser ON for `1` and OFF for `0`.
3. **Stop Bit:** Laser turns OFF to signal the end of the character.

### 3. The Receiver (Decoding)
The receiver constantly monitors the LDR pin. When it detects a `LOW` signal (light hitting the sensor), it recognizes the "Start Bit".
* **The 1.5x Timing Trick:** To ensure accurate reading, the receiver immediately waits **75ms** (1.5x the bit period). This allows it to completely skip the start bit and land exactly in the chronological "middle" of the first actual data bit.
* It then loops 8 times, reading the light state every 50ms, using bitwise operations (`receivedChar |= (1 << i)`) to reconstruct the byte.

---

## Key Features & Project Innovations

Both the software and user-experience of this project include advanced features beyond simple blink-logic:

* **Cyberpunk Web UI:** The transmitter code features an embedded HTML/CSS interface with a dark-theme UI. 
* **Asynchronous Transmission:** The web UI utilizes asynchronous JavaScript (`XMLHttpRequest`) to send data to the microcontroller in the background. This allows you to transmit messages without reloading the webpage.
* **Hardware Alignment Mode:** Because lasers require precision, the UI features an "Align Laser" toggle. This overrides the data stream and locks the laser into a steady `ON` state, allowing the user to physically aim the beam at the receiver's sensor before transmitting.
* **Smart Idle Screen:** The receiver keeps track of internal timeouts using the `millis()` function. If 15 seconds pass without a new light pulse, it automatically clears old data and resets the OLED to a clean "Waiting for msg..." interface.
