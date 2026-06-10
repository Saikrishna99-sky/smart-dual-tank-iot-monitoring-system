# smart-dual-tank-iot-monitoring-system
IoT-based dual water tank monitoring and control system using ESP32, ATmega32 I/O card, UART, MQTT, and 4-20mA pressure level sensors.
# Smart Dual Tank IoT Monitoring System

An IoT-based water tank level monitoring and motor control project using ESP32, ATmega32 I/O card, UART communication, MQTT, and industrial 4-20mA pressure level transmitters.

## Features

- Dual tank level monitoring
- 4-20mA pressure sensor support
- ATmega32 base I/O card for AI and DO handling
- ESP32 gateway for WiFi and MQTT communication
- UART JSON communication between ATmega32 and ESP32
- MQTT publish to MQTTX / cloud dashboard
- MQTT command receive and forwarding to I/O card
- Motor ON/OFF control
- Low water buzzer alert
- Webpage-based WiFi and MQTT configuration

## System Architecture

```text
Pressure Sensor
     ↓
ATmega32 I/O Card
     ↓ UART JSON
ESP32 Gateway
     ↓ WiFi MQTT
MQTTX / Cloud Dashboard
