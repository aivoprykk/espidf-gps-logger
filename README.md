# ESP-IDF GPS Logger

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-5.4.2-blue)](https://github.com/espressif/esp-idf)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-6.12.0-orange)](https://platformio.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

A comprehensive GPS logging solution for ESP32-based devices, featuring ultra-low power operation, multiple display technologies, web interface, and extensive storage options. Built with ESP-IDF for production-ready performance.

## 🌟 Features

### Core Functionality
- **GPS Logging**: Multi-format GPS data logging (TXT, GPX, UBX, SBP, GPY)
- **u-blox Integration**: Native support for u-blox GPS receivers with UBX protocol
- **Ultra-Low Power**: ULP coprocessor for battery monitoring during deep sleep
- **Multiple Displays**: Support for e-paper, LCD, and AMOLED displays
- **Web Interface**: Built-in HTTP server with REST API and web dashboard
- **WiFi Connectivity**: Station and Access Point modes with configuration portal

### Storage & Data Management
- **Multi-Storage Support**: SD card, SPI flash (FAT/SPIFFS/LittleFS)
- **Flexible File Systems**: FAT32, SPIFFS, LittleFS with unified VFS abstraction
- **Data Export**: Multiple GPS track formats with metadata
- **Space Management**: Real-time storage monitoring and automatic file rotation

### Hardware Support
- **ESP32/ESP32-S3**: Full support for both ESP32 variants
- **LilyGO T5 v2.13**: Default target with SSD1680 e-paper display
- **LilyGO T-Display S3**: Alternative target with ST7789 LCD
- **Custom Hardware**: Extensible pin configuration for custom boards

### Advanced Features
- **OTA Updates**: Over-the-air firmware updates with rollback protection
- **Button Interface**: Physical button controls with debouncing
- **Event System**: Comprehensive event-driven architecture
- **Configuration Management**: Runtime configuration via web interface or Kconfig
- **Debug Tools**: Built-in debugging and performance monitoring

## 🏗️ Architecture

### Core Components

| Component | Description | Status |
|-----------|-------------|--------|
| **logger_adc** | ULP-based ADC monitoring with battery state management | ✅ |
| **logger_common** | Shared utilities, events, and logging macros | ✅ |
| **logger_config** | Configuration management and persistence | ✅ |
| **logger_context** | Application state and context management | ✅ |
| **logger_http** | HTTP server with REST API and web interface | ✅ |
| **logger_ubx** | u-blox GPS receiver driver and UBX protocol | ✅ |
| **logger_vfs** | Virtual File System abstraction layer | ✅ |
| **logger_wifi** | WiFi connectivity and network management | ✅ |
| **display** | Multi-display support with LVGL UI framework | ✅ |
| **gps_log** | GPS data logging and file management | ✅ |

### System Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   GPS Receiver  │───▶│  GPS Processing │───▶│  Data Storage   │
│   (u-blox)      │    │  (UBX Protocol) │    │  (Multi-FS)     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Interface │    │   Display UI    │    │   ADC Monitor   │
│   (HTTP Server) │    │   (LVGL)        │    │   (ULP)         │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────┐
                    │   Main Control  │
                    │   (Event Loop)  │
                    └─────────────────┘
```

## 🛠️ Hardware Requirements

### Supported Boards

#### LilyGO T5 v2.13 (Default)
- **MCU**: ESP32
- **Display**: 2.13" SSD1680 e-paper (250x122)
- **GPS**: u-blox NEO-M8N (optional)
- **Storage**: MicroSD card slot
- **Power**: LiPo battery connector with charging circuit

#### LilyGO T-Display S3
- **MCU**: ESP32-S3
- **Display**: 1.9" ST7789 IPS LCD (170x320)
- **GPS**: External u-blox receiver
- **Storage**: MicroSD card slot
- **Power**: LiPo battery connector

#### Custom ESP32 Boards
- Any ESP32 or ESP32-S3 board with sufficient GPIO pins
- Configurable pin mappings via Kconfig
- Support for various display controllers

### Pin Configuration

#### ESP32 Default Pinout (LilyGO T5 v2.13)
```c
// GPS UART
#define GPS_TX_PIN      GPIO_NUM_17
#define GPS_RX_PIN      GPIO_NUM_16

// Display SPI
#define EPD_MOSI        GPIO_NUM_23
#define EPD_SCLK        GPIO_NUM_18
#define EPD_CS          GPIO_NUM_5
#define EPD_DC          GPIO_NUM_19
#define EPD_RST         GPIO_NUM_4
#define EPD_BUSY        GPIO_NUM_34

// SD Card SPI
#define SD_CS           GPIO_NUM_13
#define SD_MOSI         GPIO_NUM_15
#define SD_MISO         GPIO_NUM_2
#define SD_SCLK         GPIO_NUM_14

// ADC/Battery
#define ADC_PIN         GPIO_NUM_35

// Buttons
#define BUTTON_1        GPIO_NUM_39
#define BUTTON_2        GPIO_NUM_38
#define BUTTON_3        GPIO_NUM_37
```

## 📦 Installation

### Prerequisites

- **ESP-IDF**: v5.4.2 or later ([Installation Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html))
- **PlatformIO**: v6.12.0 or later ([Installation Guide](https://docs.platformio.org/en/latest/core/installation.html))
- **Git**: For cloning and submodule management
- **Python**: 3.8+ (included with ESP-IDF)

### Download

```bash
# Clone repository
git clone https://github.com/aivoprykk/espidf-gps-logger.git
cd espidf-gps-logger

# Initialize submodules (if any)
git submodule update --recursive --init
```

### PlatformIO Build

```bash
# Build for default environment (esp32dev)
pio run

# Build for specific environment
pio run -e esp32dev

# Upload firmware
pio run --target upload

# Monitor serial output
pio run --target monitor

# Clean build files
pio run --target clean
```

### ESP-IDF Build

```bash
# Source ESP-IDF environment
source /path/to/esp-idf/export.sh

# Build project
idf.py build

# Flash to device
idf.py flash

# Monitor serial output
idf.py monitor

# Clean build files
idf.py clean
```

## ⚙️ Configuration

### Build Configuration

The project uses Kconfig for build-time configuration. Key options include:

#### Core Features
```kconfig
CONFIG_GPS_LOG_ENABLED=y              # Enable GPS logging
CONFIG_LOGGER_ADC_ENABLED=y           # Enable ADC monitoring
CONFIG_LOGGER_HTTP_ENABLED=y          # Enable HTTP server
CONFIG_LOGGER_WIFI_ENABLED=y          # Enable WiFi
CONFIG_DISPLAY_ENABLED=y              # Enable display
CONFIG_UBLOX_ENABLED=y                # Enable u-blox GPS
CONFIG_LOGGER_VFS_ENABLED=y           # Enable VFS abstraction
```

#### GPS Configuration
```kconfig
CONFIG_GPS_LOG_FORMAT_TXT=y           # TXT format logging
CONFIG_GPS_LOG_FORMAT_GPX=y           # GPX format logging
CONFIG_GPS_LOG_AUTO_START=y           # Auto-start GPS logging
CONFIG_GPS_UPDATE_RATE=1000           # GPS update rate (ms)
```

#### Display Configuration
```kconfig
CONFIG_DISPLAY_TYPE_EPAPER=y          # E-paper display
CONFIG_DISPLAY_TYPE_LCD=n             # LCD display
CONFIG_DISPLAY_REFRESH_RATE=30        # Display refresh rate (Hz)
```

#### Storage Configuration
```kconfig
CONFIG_USE_SD_CARD=y                  # Enable SD card
CONFIG_USE_FATFS=y                    # Enable FAT on SPI flash
CONFIG_FATFS_MOUNT_POINT="/fatfs"     # FAT mount point
CONFIG_SPIFFS_MOUNT_POINT="/spiffs"   # SPIFFS mount point
```

### Runtime Configuration

Access the web interface at `http://<device-ip>` for runtime configuration:

- **GPS Settings**: Update rate, logging format, satellite configuration
- **Display Settings**: Brightness, refresh mode, UI customization
- **Storage Settings**: File rotation, space management
- **Network Settings**: WiFi credentials, AP mode configuration
- **System Settings**: Sleep mode, battery thresholds, debug options

## 🚀 Usage

### Basic Operation

1. **Power On**: Device starts and initializes all enabled components
2. **GPS Acquisition**: Wait for GPS satellite lock (may take 1-2 minutes)
3. **Logging Start**: GPS data logging begins automatically or via button/web interface
4. **Data Access**: View logs via web interface or extract from storage

### Web Interface

The built-in web server provides:

- **Dashboard**: Real-time GPS data, battery status, storage usage
- **Configuration**: Runtime settings modification
- **File Browser**: Download/upload GPS logs and configuration files
- **System Info**: Device status, firmware version, debug information

### Button Controls

- **Button 1**: Start/Stop GPS logging
- **Button 2**: Toggle display mode
- **Button 3**: Enter configuration mode

### Data Formats

#### TXT Format
```
2024-01-15 10:30:15, 46.123456, 14.567890, 245.6, 12, 3.2
timestamp, latitude, longitude, altitude, satellites, hdop
```

#### GPX Format
```xml
<gpx version="1.1" creator="ESP-IDF GPS Logger">
  <trk>
    <name>GPS Track</name>
    <trkseg>
      <trkpt lat="46.123456" lon="14.567890">
        <ele>245.6</ele>
        <time>2024-01-15T10:30:15Z</time>
        <sat>12</sat>
        <hdop>3.2</hdop>
      </trkpt>
    </trkseg>
  </trk>
</gpx>
```

## 🔧 Development

### Project Structure

```
espidf-gps-logger/
├── main/                          # Main application
│   ├── main.c                     # Application entry point
│   ├── private.h                  # Private definitions
│   └── pin_config.h               # Pin configurations
├── components/                    # ESP-IDF components
│   ├── logger_adc/                # ADC monitoring
│   ├── logger_common/             # Common utilities
│   ├── logger_http/               # HTTP server
│   ├── logger_ubx/                # u-blox GPS driver
│   ├── logger_vfs/                # VFS abstraction
│   ├── display/                   # Display drivers
│   └── gps_log/                   # GPS logging
├── components/                    # Additional components
├── scripts/                       # Build scripts
├── test/                          # Unit tests
├── doc/                           # Documentation
├── platformio.ini                 # PlatformIO config
├── CMakeLists.txt                 # ESP-IDF build config
├── sdkconfig.defaults            # Default configuration
├── partitions.csv                 # Partition table
└── version.txt                    # Version information
```

### Building Components

Each component can be built independently:

```bash
# Build specific component
idf.py build --component logger_adc

# Clean specific component
idf.py clean --component logger_adc
```

### ULP Assembly

ULP (Ultra Low Power) code is automatically compiled:

```bash
# Manual ULP build (usually automatic)
python scripts/ulp_build.py
```

### Testing

```bash
# Run unit tests
idf.py test

# Run specific test component
idf.py test --component logger_common
```

## 📚 API Documentation

### Core APIs

#### GPS Logging
```c
#include "gps_log.h"

// Initialize GPS logging
gps_log_init();

// Start logging
gps_log_start();

// Stop logging
gps_log_stop();

// Get current GPS data
const gps_data_t *gps_data = gps_log_get_data();
```

#### HTTP Server
```c
#include "http_rest_server.h"

// Start HTTP server
http_rest_server_start();

// Register custom endpoint
esp_err_t http_register_endpoint(const char *uri, http_method method,
                                esp_err_t (*handler)(httpd_req_t *req));
```

#### VFS Operations
```c
#include "vfs.h"

// Initialize VFS
vfs_init();

// File operations
s_write("data.txt", "/sdcard", data, len);
char *content = s_read_from_file("config.txt", "/fatfs");
```

### Event System

```c
#include "logger_events.h"

// Register event handler
esp_event_handler_register(LOGGER_EVENT, LOGGER_EVENT_GPS_FIX_ACQUIRED,
                          gps_event_handler, NULL);

// GPS event handler
static void gps_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data) {
    switch(event_id) {
        case LOGGER_EVENT_GPS_FIX_ACQUIRED:
            // GPS fix acquired
            break;
        case LOGGER_EVENT_GPS_DATA_READY:
            // New GPS data available
            break;
    }
}
```

## 🔍 Troubleshooting

### Common Issues

#### GPS Not Acquiring Signal
- Ensure GPS antenna has clear sky view
- Check GPS module power and UART connections
- Verify GPS baud rate configuration
- Wait 1-2 minutes for cold start satellite acquisition

#### Display Not Working
- Check display pin connections
- Verify display type in Kconfig
- Check power supply voltage
- Enable debug logging for display initialization

#### SD Card Not Detected
- Format SD card as FAT32
- Check SD card pin connections
- Verify SPI bus configuration
- Check card insertion and lock switch

#### WiFi Connection Issues
- Verify SSID and password
- Check WiFi signal strength
- Try different WiFi channels
- Enable AP mode for configuration

#### Low Battery Performance
- Check battery voltage with ADC monitoring
- Verify ULP coprocessor configuration
- Adjust sleep intervals
- Check for battery drain during sleep

### Debug Tools

#### Logging Levels
```c
// Enable debug logging
CONFIG_LOGGER_COMMON_LOG_LEVEL_DEBUG=y

// View logs
idf.py monitor
```

#### Performance Monitoring
- Built-in CPU usage monitoring
- Memory usage statistics
- GPS fix quality metrics
- Storage I/O performance

#### Web Debug Interface
Access `http://<device-ip>/debug` for:
- System information
- Component status
- Performance metrics
- Configuration dump

## 🤝 Contributing

### Development Setup

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/new-feature`
3. Make your changes following the coding standards
4. Add tests for new functionality
5. Update documentation
6. Submit a pull request

### Coding Standards

- Follow ESP-IDF coding conventions
- Use the provided logging macros (`ILOG`, `DLOG`, `ELOG`, etc.)
- Document all public APIs
- Include error handling for all operations
- Test on multiple hardware configurations

### Component Development

When adding new components:

1. Create component directory: `components/your_component/`
2. Add required files: `CMakeLists.txt`, `Kconfig`, `README.md`
3. Implement component following the established patterns
4. Add component to main application
5. Update documentation

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Based on the original [ESP-GPS-Logger](https://github.com/RP6conrad/ESP-GPS-Logger) by RP6conrad
- ESP-IDF framework by [Espressif Systems](https://github.com/espressif/esp-idf)
- LVGL graphics library for display support
- u-blox for GPS technology

## 📞 Support

- **Homepage**: https://esplogger.majasa.ee/
- **User Manual**: [ESP-Logger-Manual-v1-en.pdf](https://github.com/aivoprykk/espidf-gps-logger/ESP-Logger-Manual-v1-en.pdf)
- **Issues**: [GitHub Issues](https://github.com/aivoprykk/espidf-gps-logger/issues)
- **Discussions**: [GitHub Discussions](https://github.com/aivoprykk/espidf-gps-logger/discussions)

---

**Version**: 1.0.4.25.dev
**ESP-IDF**: 5.4.2
**PlatformIO**: 6.12.0


