# Test code using ESP32-CAM module (ESP-IDF ver 5.4.0)

Low cost camera:

- [Amazon link](https://www.amazon.ca/dp/B0BPXK9897)
- [HW info](https://www.aideepen.com/products/esp32-cam-wifi-bluetooth-module-with-ov2640-camera-module-development-board-esp32-support-ov2640-and-ov7670-cameras-5v)


# Env settings
```
source linux-source-me
export WIFI_SSID=<your_wifi_ssid>
export WIFI_PASSWORD=<your_wifi_password>
```
# Init submodule
```
git submodule update --init --recursive --progress
```

# To build
```
idf.py build
```

# Original link:
https://esp32tutorials.com/esp32-cam-esp-idf-live-streaming-web-server/

# Pictures
![Picture](Docs/esp32-cam-picture.webp)

# Pinout
![Pinout 1](Docs/pinout.avif)
![Pinout 2](Docs/pinout-2.avif)
