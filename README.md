ESP-IDF Gps Logger
=====================================
Based on https://github.com/RP6conrad/ESP-GPS-Logger but intended to xxmove away from Arduino.


Prerequirement is PlatformIO or ESP-IDF. Also git is good to have in PATH.

```shell
# Clone repository
$ git clone https://github.com/aivoprykk/espidf-gps-logger.git

# Change directory
$ cd espidf-gps-logger

# Update/Init submodules
# git submodule update --recursive --init

# Build project
$ pio run

# Upload firmware
$ pio run --target upload

# Build specific environment
$ pio run -e esp32dev

# Upload firmware for the specific environment
$ pio run -e esp32dev --target upload

# Clean build files
$ pio run --target clean
```# espidf-gps-logger
