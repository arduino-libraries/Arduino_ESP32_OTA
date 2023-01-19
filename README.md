`Arduino_ESP32_OTA`
====================

*Note: This library is currently in [beta](#running-tests).*

[![Compile Examples](https://github.com/arduino-libraries/Arduino_ESP32_OTA/workflows/Compile%20Examples/badge.svg)](https://github.com/arduino-libraries/Arduino_ESP32_OTA/actions?workflow=Compile+Examples)
[![Arduino Lint](https://github.com/arduino-libraries/Arduino_ESP32_OTA/workflows/Arduino%20Lint/badge.svg)](https://github.com/arduino-libraries/Arduino_ESP32_OTA/actions?workflow=Arduino+Lint)
[![Spell Check](https://github.com/arduino-libraries/Arduino_ESP32_OTA/workflows/Spell%20Check/badge.svg)](https://github.com/arduino-libraries/Arduino_ESP32_OTA/actions?workflow=Spell+Check)

This library allows OTA (Over-The-Air) firmware updates for ESP32 boards. OTA binaries are downloaded via WiFi and stored in the OTA flash partition. After integrity checks the reference to the new firmware is configured in the bootloader; finally board resets to boot new firmware.

The library is based on the [Update](https://github.com/espressif/arduino-esp32/tree/master/libraries/Update) library of the [arduino-esp32](https://github.com/espressif/arduino-esp32) core.

## :mag: How?

* Create a minimal [example](examples/OTA/OTA.ino)
* Create a [compressed](https://github.com/arduino-libraries/ArduinoIoTCloud/blob/master/extras/tools/lzss.py) [ota](https://github.com/arduino-libraries/ArduinoIoTCloud/blob/master/extras/tools/bin2ota.py) file

## :key: Requirements

* Flash size >= 4MB
* OTA_1 partition available within selected partition scheme

    | Partition scheme | OTA support |
    | --- | --- |
    | `app3M_fat9M_16MB` | :heavy_check_mark: |
    | `default_16MB` | :heavy_check_mark: |
    | `large_spiffs_16MB` | :heavy_check_mark: |
    | `ffat` | :heavy_check_mark: |
    | `default_8MB` | :heavy_check_mark: |
    | `max_app_8MB` | :x: |
    | `min_spiffs` | :heavy_check_mark: |
    | `default` / `default_ffat` | :heavy_check_mark: |
    | `huge_app` | :x: |
    | `no_ota` | :x: |
    | `noota_3g` / `noota_3gffat` / `noota_ffat` | :x: |
    | `rainmaker` | :heavy_check_mark: |
    | `minimal` | :x: |
    | `bare_minimum_2MB` | :x: |


## :running: Tests

* The library has been tested on the following boards:

    | Module | Board |
    | --- | --- |
    | `ESP32-WROOM` | [ESP32-DevKitC V4](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html#) |
    | `ESP32-WROVER` | [Freenove_ESP32_WROVER](https://github.com/Freenove/Freenove_ESP32_WROVER_Board) |
    |  | DOIT ESP32 DEVKIT V1 |
    |  | Wemos Lolin D32 |
    |  | NodeMCU-32S |
    | `ESP32-­S3-­WROOM`| [ESP32-S3-DevKitC-1 v1.1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html) |
    | `ESP32-­S2-­SOLO` | [ESP32-S2-DevKitC-1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/hw-reference/esp32s2/user-guide-s2-devkitc-1.html) |
    |  | NodeMCU-32-S2 |
    | `ESP32-C3`  | [LILYGO mini D1 PLUS](https://github.com/Xinyuan-LilyGO/LilyGo-T-OI-PLUS)|
