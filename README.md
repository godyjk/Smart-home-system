# Smart-home-system
基于arduino开发的智能家居程序，包括wifi传输，红外遥控，声控，光控，数据显示等
# Smart Home Control System - Project Documentation

## Project Overview
This project is a **multi-node smart home control system** based on the Arduino ecosystem. It leverages two core hardware nodes—ESP8266 (WiFi communication) and Arduino Mega (device control core)—to enable manual control, intelligent automatic control, and remote control of home devices (windows, doors, fans, lights, etc.). Additionally, it supports environmental data collection (temperature, humidity, light intensity) and visual display.


## System Architecture
The system consists of two core hardware nodes, each with independent yet collaborative functions. The overall architecture is as follows:

| Hardware Node  | Core Functions                                                                 | Communication Method          |
|----------------|--------------------------------------------------------------------------------|-------------------------------|
| ESP8266        | WiFi gateway that connects to an MQTT server, enabling data transmission between Arduino Mega and remote platforms | WiFi (MQTT), UART (Serial)    |
| Arduino Mega   | Device control core that drives sensors (temperature-humidity, rain, PIR motion, light) and actuators (servos, fans, lights) | UART (Serial)                 |


## Code File Description
The project includes 2 core code files, corresponding to the two hardware nodes. The mapping between filenames and functions is as follows:

| Code File               | Corresponding Hardware | Core Functions                                                                 |
|-------------------------|------------------------|--------------------------------------------------------------------------------|
| `ESP8266_MQTT.ino`      | ESP8266                | 1. Connect to a WiFi network<br>2. Connect to an MQTT server for topic subscription/publishing<br>3. Forward data between Arduino Mega and the server |
| `Mega_DeviceControl.ino`| Arduino Mega           | 1. Drive sensors (DHT11, rain sensor, light-dependent resistor, PIR motion sensor)<br>2. Control actuators (servos for doors/windows, fans, RGB lights)<br>3. Support infrared remote control, voice control, and intelligent automatic modes |


## Hardware Preparation
### 1. General Hardware (Basic Dependencies)
- USB data cables (for program burning and power supply)
- 12V/2A power adapter (for powering Arduino Mega and actuators)
- Breadboard, jumper wires (male-to-female / male-to-male)


### 2. Hardware List by Node
#### ESP8266 Node
| Component          | Model/Specification               | Purpose                          |
|--------------------|-----------------------------------|----------------------------------|
| ESP8266 Development Board | NodeMCU 1.0 (ESP-12E core)      | Core for WiFi communication      |
| USB-TTL Adapter    | CH340 chip-based                  | Program burning and serial debugging |


#### Arduino Mega Node
| Component          | Model/Specification               | Purpose                          | Wiring Pins (Mega)              |
|--------------------|-----------------------------------|----------------------------------|----------------------------------|
| Arduino Mega       | Mega 2560                         | Control core                     | -                                |
| Temperature-Humidity Sensor | DHT11                        | Collect ambient temperature and humidity | 7 (DHT11_PIN)                    |
| Rain Sensor        | Analog output type                | Detect rain (to control windows) | A1 (Rain)                        |
| Light-Dependent Resistor (LDR) Module | Analog output type          | Detect light intensity (to control lights) | A0                               |
| PIR Motion Sensor  | HC-SR501                          | Detect human presence (for motion-activated lights) | 25 (pushButton)              |
| Servos             | SG90 (2 units)                    | Control door/window opening/closing | 6 (servoPin), 31 (servoPin2)    |
| Fan                | 12V DC fan (with relay)           | Cooling                          | 9 (Fan_Pin)                      |
| RGB LED Module     | Common cathode                    | Ambient lighting                 | 43 (LED_R), 3 (LED_G), 5 (LED_B) |
| IR Receiver Module | VS1838B                           | Receive infrared remote signals  | A2 (RECV_PIN)                    |
| LCD1602 Screen     | I2C interface version             | Display temperature/humidity data | A4 (SDA), A5 (SCL)              |
| TM1637 Display     | 4-digit 7-segment display         | Show light intensity             | 3 (PIN_DIO), 4 (PIN_CLK)         |
| Voice Control Module | Custom voice recognition module | Voice-controlled device operation | 46 (IVC_O), 49 (IVC_C), etc.    |


## Environment Setup
### 1. Development Software
- **Arduino IDE** (recommended version: 1.8.19 or later): Used for writing and burning code.
- Required Libraries Installation: Open Arduino IDE → Go to 「Sketch」→「Include Library」→「Manage Libraries」, then search and install the following libraries:

| Library Name               | Purpose                          |
|----------------------------|----------------------------------|
| `ESP8266WiFi`              | WiFi communication for ESP8266   |
| `PubSubClient`             | MQTT protocol communication      |
| `IRremote`                 | Infrared signal reception and parsing |
| `Servo`                    | Servo motor control              |
| `DHT sensor library`       | Driver for DHT11 temperature-humidity sensor |
| `LiquidCrystal_I2C`        | Control for I2C-interfaced LCD1602 |
| `TM1637Display`            | Driver for TM1637 7-segment display |


### 2. Hardware Configuration (Key Parameter Modification)
Some parameters in the code must be modified to match your actual environment; otherwise, the system will not work properly. The key parameters to modify are as follows:

#### ESP8266 Code (`ESP8266_MQTT.ino`)
```cpp
// WiFi network information (replace with your WiFi SSID and password)
const char* ssid = "12345"; 
const char* password = "12345678"; 

// MQTT server information (replace with your MQTT server IP and port)
const char* mqttServer = "192.168.156.91"; 
const int mqttPort = 1883;

// MQTT topics (modifiable as needed; must match the server-side topics)
String topicString = "hardwareTopic"; // Publish topic (Mega → Server)
String subTopic = "webTopic";         // Subscribe topic (Server → Mega)
```

#### Arduino Mega Code (`Mega_DeviceControl.ino`)
```cpp
// Temperature threshold (fan turns on automatically when exceeded; adjust as needed)
#define TEMP_THRESHOLD 29

// Rain sensor threshold (window closes automatically when exceeded; calibrate for your sensor)
// In code: analogRead(Rain) > 800; modify the value as needed
#define RAIN_THRESHOLD 800 

// IR remote control key codes (calibrate for your remote; replace values in the 'case' statements)
// Example: Key 0 corresponds to codes 0x97483BFB and 0xFF9867; use a serial monitor to read actual codes
```


## Function Description
### 1. Core Control Functions
The system supports **3 control methods** to cover different usage scenarios:

| Control Method       | Implementation                                                                 | Supported Functions                  |
|----------------------|--------------------------------------------------------------------------------|--------------------------------------|
| Manual Control       | Infrared remote control (keys map to device on/off), physical buttons (doorbell) | Door/window on/off, fan on/off, light on/off |
| Intelligent Automatic Control | Sensor triggering (temperature-humidity, rain, light intensity, PIR motion) | 1. Fan turns on when temperature exceeds threshold<br>2. Window closes when it rains<br>3. Light turns on when light intensity is low<br>4. Motion-activated light |
| Voice Control        | Voice module outputs high/low levels to trigger device control logic           | Door/window, fan, light on/off; enable/disable intelligent mode |
| Remote Control       | ESP8266 + MQTT for remote control                                              | Full device on/off, status query     |


### 2. Data Collection and Display
- **Environmental Data Collection**: Collects temperature/humidity via DHT11, light intensity via LDR, and rain status via the rain sensor.
- **Data Display**:
  - LCD1602 Screen: Real-time display of temperature and humidity (format: `Humidity: XX.X%`, `Temperature: XX.X°C`).
  - TM1637 7-Segment Display: Real-time display of light intensity (analog value).
  - Serial Debugging: All device statuses and sensor data are output via the serial port (baud rate: 9600).


## Deployment Steps
### 1. Hardware Wiring
Follow the pin instructions in the 「Hardware Preparation」section to connect each module to the development board. Note:
- High-power devices (fans, servos) must be powered by an external power supply (12V); do not use the 5V pin of the development board directly.
- For serial communication modules (e.g., ESP8266 connected to Mega), cross-connect TX/RX pins (Mega TX → ESP8266 RX, Mega RX → ESP8266 TX).


### 2. Code Burning
1. Open Arduino IDE and select the corresponding development board:
   - ESP8266: 「Tools」→「Board」→「ESP8266 Boards」→「NodeMCU 1.0」.
   - Arduino Mega: 「Tools」→「Board」→「Arduino AVR Boards」→「Arduino Mega or Mega 2560」.
2. Select the corresponding serial port (「Tools」→「Port」).
3. Open the corresponding code file and click the 「Upload」button to burn the program.


### 3. System Testing
1. **Basic Function Testing**:
   - After burning, open the serial monitor (baud rate: 9600) and check the device initialization information (e.g., whether ESP8266 successfully connects to WiFi and the MQTT server).
   - Press the corresponding keys on the infrared remote control and check if the serial output matches the device action (e.g., press the "0" key to control window opening/closing).
2. **Intelligent Mode Testing**:
   - Spray water on the rain sensor and observe if the window closes automatically.
   - Cover the LDR with your hand and observe if the light brightens automatically.
3. **Remote Control Testing**:
   - Send a message to the `webTopic` MQTT topic via the MQTT server and check if Arduino Mega receives and executes the command.


## Troubleshooting
1. **ESP8266 Fails to Connect to WiFi**:
   - Verify that the SSID and password are correct (case-sensitive).
   - Ensure the WiFi network uses the 2.4GHz band (ESP8266 does not support 5GHz).

2. **Arduino Mega Fails to Control Servos/Fans**:
   - Check wiring connections and ensure actuators are powered by an external power supply.
   - Check the serial output for corresponding command logs to rule out code logic issues.

3. **MQTT Data Transmission Fails**:
   - Verify that the MQTT server is online and the IP/port are correct.
   - Check the "Client State" output in the ESP8266 serial monitor to troubleshoot issues (e.g., state code 5 indicates authentication failure).

4. **Infrared Remote Control Not Responding**:
   - Check the wiring of the IR receiver module (ensure it is connected to the correct pin A2).
   - Use the serial monitor to read the actual key codes of your remote and update the `case` values in the code.


## Notes
1. Safety Tips:
   - High-power devices (fans, servos) must use a dedicated external power supply to avoid overloading and damaging the development board.
   - Keep sensors (e.g., DHT11, rain sensor) away from water and high-temperature environments to ensure measurement accuracy.
2. Stability Optimization:
   - For long-term use, add heat sinks to the ESP8266 and Arduino Mega to prevent overheating.
   - Use a local LAN MQTT server (e.g., Mosquitto) instead of a public network server to avoid latency issues.
3. Expansion Suggestions:
   - Add a mobile app (e.g., using MQTT.fx or a custom app) for more intuitive remote control.
   - Integrate a smoke sensor or gas sensor to expand safety monitoring functions.
