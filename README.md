# Gluvok by Lathey Weigh Trix — ESP32 Cloud Weighing System

An enterprise-grade IoT firmware for ESP32 microcontrollers designed to bridge industrial weighing scale indicators with **Cloud Supabase** database logging. Features a modern glassmorphic web configuration portal, non-volatile NVS flash settings, automatic scale weight stabilization detection, Supabase JWT authentication, and automatic over-the-air (OTA) firmware updates.

---

## 🏗️ Project Architecture & Folder Structure

The project uses a clean C++ domain-driven folder hierarchy inside `src/`. **Every code file is tightly scoped and under 100 lines of code**:

```
hermes/
├── main.ino                   # Sketch entry point (setup, loop, stream dispatch - 80 lines)
├── README.md
└── src/
    ├── config/
    │   ├── config_manager.h   # Flash NVS settings header
    │   └── config_manager.cpp # Flash NVS Preferences read/write logic (46 lines)
    ├── scale/
    │   ├── scale_parser.h     # Weight stabilization state machine header
    │   └── scale_parser.cpp   # Scale stream parsing & stability algorithm (52 lines)
    ├── network/
    │   ├── supabase_auth.h    # Supabase Auth JWT header
    │   ├── supabase_auth.cpp  # Login & Profile ID resolution (85 lines)
    │   ├── supabase_post.h    # REST weighment post header
    │   └── supabase_post.cpp  # JSON payload POST execution (49 lines)
    ├── portal/
    │   ├── wifi_portal.h      # AP Access Point & WebServer header
    │   └── wifi_portal.cpp    # WebServer routes & WiFi connection (94 lines)
    ├── ota/
    │   ├── ota_updater.h      # HTTP OTA updater header
    │   └── ota_updater.cpp    # Remote GitHub version check & flash logic (46 lines)
    └── ui/
        ├── config_page.h      # Glassmorphic Portal HTML page template
        └── save_page.h        # Confirmation & Reboot HTML page template
```

---

## 🌟 Key Features

- **Styled Glassmorphic Configuration Portal**: Built-in Access Point (AP) mode serving an embedded responsive web UI for setting up Wi-Fi network credentials, operator login details, center IDs, and minimum weight thresholds.
- **Permanent NVS Storage**: Stores configuration parameters securely in ESP32 Non-Volatile Flash (`Preferences` library).
- **Weight Scale Serial Parsing**: Reads raw continuous stream data from UART2 (pins 16/17 at 1200 Baud 8N1) supporting integer and floating-point readings.
- **Weight Stabilization Detection**: Built-in state machine that tracks weight stability over time (`STABILITY_TOLERANCE = 2.0 kg`, `STABILITY_DURATION = 10s`) before triggering cloud upload.
- **Supabase Cloud Integration**: Authenticates via Supabase Auth REST API using operator credentials to obtain dynamic JWT tokens and post weighment logs to the `weighments` database table.
- **Automatic OTA Firmware Update**: Periodically checks remote GitHub JSON release manifest (`version.json`) every hour and flashes new firmware binaries over-the-air using `HTTPUpdate`.
- **Non-Blocking Architecture**: Fully non-blocking main execution loop with auto-reconnection logic for Wi-Fi.

---

## 🛠️ Hardware Requirements & Pinout

### Recommended Microcontroller
- **ESP32 Development Board** (ESP32-WROOM-32 or similar)

### UART2 Serial Indicator Pinout
The ESP32 communicates with the industrial weight scale indicator via HardwareSerial `UART2`:

| ESP32 Pin | Function | Scale Indicator Connection |
| :--- | :--- | :--- |
| **GPIO 16** | `RX2` | Indicator TX / Serial Output |
| **GPIO 17** | `TX2` | Indicator RX (Optional) |
| **GND** | Ground | Common Ground |

---

## 🚀 Compilation & Flashing Instructions

### Prerequisites
- **Arduino IDE** (v2.0+) or **PlatformIO**
- **ESP32 Board Package**: Installed via Arduino Board Manager (`esp32` by Expressif Systems)

---

## 📄 License & Credits

Developed for **Gluvok by Lathey Weigh Trix**. All rights reserved.
