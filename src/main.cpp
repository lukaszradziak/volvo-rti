#include <Arduino.h>

enum display_mode_name {RTI_RGB, RTI_PAL, RTI_NTSC, RTI_OFF};
const char display_modes[] = {0x40, 0x45, 0x4C, 0x46};
const char brightness_levels[] = {0x20, 0x61, 0x62, 0x23, 0x64, 0x25, 0x26, 0x67, 0x68, 0x29, 0x2A, 0x2C, 0x6B, 0x6D, 0x6E, 0x2F};

int current_display_mode = RTI_RGB;
bool send_brightness = true;
char current_brightness_level = 15;

const int rti_delay = 100;

void rtiWrite(char byte) {
  Serial1.print(byte);
  delay(rti_delay);
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(2400, SERIAL_8N1, -1, 23);
}

void loop() {
  rtiWrite(display_modes[current_display_mode]);
  
  if (send_brightness){
    rtiWrite(brightness_levels[current_brightness_level]);
  } else {
    rtiWrite(0x40);
  }
  
  rtiWrite(0x83);
}
