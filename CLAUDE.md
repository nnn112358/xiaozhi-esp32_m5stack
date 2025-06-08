# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

XiaoZhi ESP32 is an AI chatbot device based on ESP32 microcontrollers. It provides voice interaction capabilities with support for multiple languages, display output, and IoT device control.

## Build Commands

### Prerequisites
- ESP-IDF 5.3+ installed and configured
- Python 3.8+ with required packages

### Common Build Commands

```bash
# Configure the project (select board and options)
idf.py menuconfig

# Build the project
idf.py build

# Flash to device
idf.py -p /dev/ttyUSB0 flash

# Monitor serial output
idf.py -p /dev/ttyUSB0 monitor

# Clean build
idf.py fullclean

# Build and flash in one command
idf.py -p /dev/ttyUSB0 flash monitor
```

### Release Build Script
```bash
# Build release firmware for all boards
python scripts/release.py

# Build for specific board
python scripts/release.py --board esp-box-3
```

### Development Tools
```bash
# Generate language files from assets
python scripts/gen_lang.py

# Convert audio to P3 format
python scripts/p3_tools/convert_audio_to_p3.py input.wav output.p3

# Flash pre-built firmware
python scripts/flash.sh firmware.bin
```

## Architecture Overview

### Core Components

1. **Application Layer** (`main/application.cc`)
   - Singleton pattern for global application state
   - Manages audio pipeline, display, protocols, and IoT devices
   - Event-driven architecture with callbacks

2. **Hardware Abstraction** (`main/boards/common/board.h`)
   - Base `Board` class with `WifiBoard` and `ML307Board` implementations
   - Factory pattern for board registration via `DECLARE_BOARD` macro
   - Each board has `config.h` (pins), `config.json` (build), and implementation file

3. **Communication Protocols** (`main/protocols/`)
   - `Protocol` interface for WebSocket and MQTT
   - Binary audio (Opus) and JSON control messages
   - Asynchronous message handling with queues

4. **Audio Processing Pipeline**
   - Audio codecs (ES8311, ES8374, ES8388, Box codec)
   - AFE processor with echo cancellation
   - Wake word detection using ESP-SR
   - Opus encoding/decoding for network transmission

5. **IoT Framework** (`main/iot/`)
   - `Thing` base class for IoT devices
   - `ThingManager` singleton for device registry
   - Voice-controllable properties and methods

### Key Design Patterns

- **Singleton**: Application, Board, ThingManager instances
- **Factory**: Board and Thing creation
- **Observer**: Event callbacks for audio/control messages
- **Template Method**: Board initialization sequence

### Board Support

The project supports 50+ boards across ESP32, ESP32-S3, ESP32-C3, and ESP32-P4. Each board directory contains:
- Board implementation (.cc file)
- Pin configuration (config.h)
- Build configuration (config.json)
- Optional custom components (audio codec, power management)

### Message Flow

1. Wake word detection triggers recording
2. Audio compressed with Opus codec
3. Sent via WebSocket/MQTT to server
4. Server responds with audio/control messages
5. Audio played back, control messages processed
6. IoT devices updated based on commands

## Important Configuration

### Menuconfig Options
- `XiaoZhi Configuration > Choose Board`: Select target hardware
- `XiaoZhi Configuration > Choose Language`: Set device language
- `Audio Processing > Enable AEC`: Acoustic echo cancellation
- `Audio Processing > Enable Wake Word Detection`: Voice activation
- `IoT Configuration > Protocol Type`: WebSocket or MQTT

### Critical Build Settings
- Board type MUST match hardware to avoid OTA conflicts
- ESP-IDF 5.3+ required for all features
- Linux recommended for development (Windows has path issues)

## Code Style

The project follows Google C++ code style. Key points:
- 2-space indentation
- Opening braces on same line
- CamelCase for classes, snake_case for functions/variables
- Header guards use full path format