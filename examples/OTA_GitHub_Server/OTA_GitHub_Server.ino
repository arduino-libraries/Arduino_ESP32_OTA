/*
 * This example demonstrates how to use to update the firmware using Arduino_ESP_OTA library
 *
 * Steps:
 *   1) Create a sketch for your ESP board and verify
 *      that it both compiles and works.
 *   2) In the IDE select: Sketch -> Export compiled Binary.
 *   3) Create an OTA update file utilising the tools 'lzss.py' and 'bin2ota.py' stored in
 *      https://github.com/arduino-libraries/ArduinoIoTCloud/tree/master/extras/tools .
 *      A) ./lzss.py --encode SKETCH.bin SKETCH.lzss
 *      B) ./bin2ota.py ESP SKETCH.lzss SKETCH.ota
 *   4) Upload the OTA file to a network reachable location, e.g. LOLIN_32_Blink.ino.ota
 *      has been uploaded to: http://downloads.arduino.cc/ota/LOLIN_32_Blink.ino.ota 
 *   5) Verify if a custom ca_cert is needed by default DigiCert root CA are used
 *      https://www.digicert.com/kb/digicert-root-certificates.htm
 *   6) Perform an OTA update via steps outlined below.
 */

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <Arduino_ESP32_OTA.h>

#include <WiFi.h>

#include "arduino_secrets.h"

#include "root_ca.h"

/******************************************************************************
 * CONSTANT
 ******************************************************************************/

/* Please enter your sensitive data in the Secret tab/arduino_secrets.h */
static char const SSID[] = SECRET_SSID;  /* your network SSID (name) */
static char const PASS[] = SECRET_PASS;  /* your network password (use for WPA, or use as key for WEP) */


#if defined(ARDUINO_NANO_ESP32)
static char const OTA_FILE_LOCATION[] = "https://raw.githubusercontent.com/arduino-libraries/Arduino_ESP32_OTA/main/examples/NANO_ESP32_Blink/NANO_ESP32_Blink.ino.ota";
#else
static char const OTA_FILE_LOCATION[] = "https://raw.githubusercontent.com/arduino-libraries/Arduino_ESP32_OTA/main/examples/LOLIN_32_Blink/LOLIN_32_Blink.ino.ota";
#endif

/******************************************************************************
 * SETUP/LOOP
 ******************************************************************************/

void setup()
{
  Serial.begin(9600);
  while (!Serial) {}

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print  ("Attempting to connect to '");
    Serial.print  (SSID);
    Serial.println("'");
    WiFi.begin(SSID, PASS);
    delay(2000);
  }
  Serial.print  ("You're connected to '");
  Serial.print  (WiFi.SSID());
  Serial.println("'");

  Arduino_ESP32_OTA ota;
  Arduino_ESP32_OTA::Error ota_err = Arduino_ESP32_OTA::Error::None;

  /* Configure custom Root CA */
  ota.setCACert(root_ca);

  Serial.println("Initializing OTA storage");
  if ((ota_err = ota.begin()) != Arduino_ESP32_OTA::Error::None)
  {
    Serial.print  ("Arduino_ESP_OTA::begin() failed with error code ");
    Serial.println((int)ota_err);
    return;
  }


  Serial.println("Starting download to flash ...");
  int const ota_download = ota.download(OTA_FILE_LOCATION);
  if (ota_download <= 0)
  {
    Serial.print  ("Arduino_ESP_OTA::download failed with error code ");
    Serial.println(ota_download);
    return;
  }
  Serial.print  (ota_download);
  Serial.println(" bytes stored.");


  Serial.println("Verify update integrity and apply ...");
  if ((ota_err = ota.update()) != Arduino_ESP32_OTA::Error::None)
  {
    Serial.print  ("ota.update() failed with error code ");
    Serial.println((int)ota_err);
    return;
  }

  Serial.println("Performing a reset after which the bootloader will start the new firmware.");
#if defined(ARDUINO_NANO_ESP32)
  Serial.println("Hint: Arduino NANO ESP32 will blink Red Green and Blue.");
#else
  Serial.println("Hint: LOLIN32 will blink Blue.");
#endif
  delay(1000); /* Make sure the serial message gets out before the reset. */
  ota.reset();
}

void loop()
{

}
