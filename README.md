How to build PlatformIO based project
=====================================

4. Run these commands:

```shell
# Clone repository
$ git clone https://github.com/aivoprykk/espidf-gps-logger.git

```shell
# Change directory
$ cd espidf-gps-logger

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
