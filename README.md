# About
This repository stores the implementation of a BB8 robot

# Install environment
Tutorial: https://randomnerdtutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/
- Install VS Code
- Install Python 3.x (latest)
- Install PlatformIO extension for VS Code

# Guides and examples used
PS3 controller
- https://github.com/jvpernis/esp32-ps3

WebSerial
- https://randomnerdtutorials.com/esp32-webserial-library/

OTA
- https://github.com/ayushsharma82/AsyncElegantOTA/issues/39
- https://randomnerdtutorials.com/esp32-ota-over-the-air-arduino/

# Robot setup
In VS Code, go to the PlatformIO sidebar
- Select "Erase Flash"
- Select "Upload"
- Select "Upload Filesystem Image"

Now your device is ready for configuration
WiFi access point should be available with SSID "ESP", connect to it

Open a web browser and navigate to http://192.168.5.1
You can change WiFi authentication data and bluetooth MAC address in the "Authentication" subpage

Go to "Configuration parameters" subpage
- For "body" device set NodeId to 1
- For "neck" device set NodeId to 2

When all configurations are done, in the "body" device go to "Switch to bluetooth" subpage. This will change mode to bluetooth, and restart the device.
To change back from bluetooth to WiFi mode, hold the IO0 button for 2 seconds on the device.

Note:
Currently "neck" handles WiFi connection and "body" handles bluetooth connection in normal operation.
PID parameters will be loaded from "neck", and commands from remote controller will be accepted from "body" only.
This can be changed in the code with REGLIST_HAVE_OTA and REGLIST_HAVE_REMOTE definitions.


You can adjust the PID parameters on the "PID tune" subpage.
P, I and D are used as gains, SAT is used as saturation. PID output will be limited between -SAT and +SAT.
