/*
 * Blink for LOLIN32 ESP32 board
 *
 * This sketch can be used to generate an example binary that can be uploaded to ESP32 via OTA.
 * It needs to be used together with OTA.ino
 *
 * Steps to test OTA on ESP32:
 *   1) Upload this sketch or any other sketch (this one will blink LOLIN32 blue LED).
 *   2) In the IDE select: Sketch -> Export compiled Binary
 *   3) Upload the exported binary to a server
 *   4) Open the related OTA.ino sketch and eventually update the OTA_FILE_LOCATION
 *   5) Upload the sketch OTA.ino to perform OTA
 */

void setup() {
  // initialize digital pin 5 as an output.
  pinMode(5, OUTPUT);
}

void loop() {
  digitalWrite(5, HIGH);
  delay(1000);
  digitalWrite(5, LOW);
  delay(1000);
}