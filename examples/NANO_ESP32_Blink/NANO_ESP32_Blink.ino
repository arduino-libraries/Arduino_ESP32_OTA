/*
 * Blink for Arduino NANO ESP32 board
 *
 * This sketch can be used to generate an example binary that can be uploaded to ESP32 via OTA.
 * It needs to be used together with OTA.ino
 *
 * Steps to test OTA:
 *   1) Upload this sketch or any other sketch (this one this one lights up the RGB LED with different colours).
 *   2) In the IDE select: Sketch -> Export compiled Binary
 *   3) Upload the exported binary to a server
 *   4) Open the related OTA.ino sketch and eventually update the OTA_FILE_LOCATION
 *   5) Upload the sketch OTA.ino to perform OTA
 */

void setLed(int blue, int gree, int red) {
  if (blue == 1) {
    digitalWrite(LED_BLUE, LOW);
  }
  else {
    digitalWrite(LED_BLUE, HIGH);
  }

  if (gree == 1) {
    digitalWrite(LED_GREEN, LOW);
  }
  else {
    digitalWrite(LED_GREEN, HIGH);
  }

  if (red == 1) {
    digitalWrite(LED_RED, LOW);
  }
  else {
    digitalWrite(LED_RED, HIGH);
  }
}


void setup()
{
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
}

void loop()
{ // Blue LED on
  setLed(1, 0, 0);
  delay(1000);
  // Green LED on
  setLed(0, 1, 0);
  delay(1000);
  // Red LED on
  setLed(0, 0, 1);
  delay(1000);
}
