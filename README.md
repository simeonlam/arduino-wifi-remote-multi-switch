# WIFI webserver with light controller

# clone from https://github.com/simeonlam/arduino-wifi-remote-switch

### requirement
- arduino - 1.8.10
- arduino-cli - 0.10.0
- [cp2012 driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
- esp8266 lib (from board manager) - 2.7.3

### Target
build a web server for esp8266 **WeMos D1 R1**

### Compile
```
arduino-cli compile -b esp8266:esp8266:d1
```

### Build and upload
- esp8266 CH340 (larger)
```
arduino-cli compile -b esp8266:esp8266:d1 -u -p /dev/tty.usbserial-1410
arduino-cli compile --fqbn esp8266:esp8266:d1 --upload --port /dev/tty.usbserial-1410
arduino-cli compile --fqbn esp8266:esp8266:d1 --upload --port 192.168.1.xxx
```
- esp8266 CP2012 (smaller)
```
arduino-cli compile -b esp8266:esp8266:huzzah -u -p /dev/tty.SLAB_USBtoUART
arduino-cli compile --fqbn esp8266:esp8266:huzzah --upload --port /dev/tty.SLAB_USBtoUART
arduino-cli compile --fqbn esp8266:esp8266:huzzah --upload --port 192.168.1.xxx
```
