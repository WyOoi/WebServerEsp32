# ESP32 Robot Control Web Server

This project provides a web server and Bluetooth interface for controlling a simple robot using an ESP32 microcontroller.

## Hardware Requirements

- ESP32 development board
- Cytron Motor Driver
- DC motors (2 or 4 depending on your robot chassis)
- HC-05 Bluetooth module
- Power supply for motors
- Robot chassis
- Jumper wires

## Wiring Instructions

### Motor Driver Connections
- GPIO12 -> PWM pin for left motor
- GPIO10 -> DIR pin for left motor
- GPIO6 -> PWM pin for right motor
- GPIO4 -> DIR pin for right motor

### HC-05 Bluetooth Module
- HC-05 TX -> ESP32 RX2 (GPIO16)
- HC-05 RX -> ESP32 TX2 (GPIO17)

## Software Setup

1. Install the Arduino IDE
2. Add ESP32 board support to Arduino IDE:
   - Open Arduino IDE
   - Go to File > Preferences
   - Add `https://dl.espressif.com/dl/package_esp32_index.json` to the "Additional Boards Manager URLs" field
   - Go to Tools > Board > Boards Manager
   - Search for "esp32" and install the ESP32 package

3. Install required libraries:
   - Go to Sketch > Include Library > Manage Libraries
   - Search for and install "WiFi", "WebServer", and "CytronMotorDriver" libraries

4. Open the `robot_control.ino` file in the Arduino IDE
5. Update the WiFi credentials in the code with your network name and password
6. Select your ESP32 board from Tools > Board
7. Select the correct port from Tools > Port
8. Upload the sketch to your ESP32

## Usage

### Web Control
1. After uploading, open the Serial Monitor to see the ESP32's IP address
2. Connect to this IP address from a web browser on a device connected to the same WiFi network
3. Use the control interface to move the robot:
   - Press and hold buttons for movement
   - Release to stop

### Bluetooth Control
1. Pair your device with the HC-05 module (default password is usually 1234 or 0000)
2. Use a Bluetooth terminal app to send commands:
   - 'W' - Move forward
   - 'S' - Move backward
   - 'A' - Turn left
   - 'D' - Turn right
   - 'X' - Stop

## Customizing

You can modify the motor speed by changing the DRIVE_SPEED constant in the code. 
