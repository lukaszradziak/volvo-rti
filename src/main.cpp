#include <Arduino.h>
#include <EEPROM.h>

#include <ESP32CAN.h>
#include <CAN_config.h>

CAN_device_t CAN_cfg; 
const int rx_queue_size = 10; 

enum display_mode_name {RTI_RGB, RTI_PAL, RTI_NTSC, RTI_OFF};
const char display_modes[] = {0x40, 0x45, 0x4C, 0x46};
const char brightness_levels[] = {0x19, 0x20, 0x61, 0x62, 0x23, 0x64, 0x25, 0x26, 0x67, 0x68, 0x29, 0x2A, 0x2C, 0x6B, 0x6D, 0x6E, 0x2F};

int current_display_mode = RTI_OFF;
char current_brightness_level = 12;

void rtiWrite(char byte) {
  Serial1.print(byte);
  vTaskDelay(100UL);
}

void task0(void* param){
  while(true){
    rtiWrite(display_modes[current_display_mode]);
    rtiWrite(brightness_levels[current_brightness_level]);
    rtiWrite(0x83);
  }
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(2400, SERIAL_8N1, -1, 23);

  EEPROM.begin(128);

  Serial.println("EEPROM");
  Serial.println(EEPROM.readChar(0));
  Serial.println(EEPROM.readChar(1));
  Serial.println();

  current_display_mode = EEPROM.readChar(0);
  current_brightness_level = EEPROM.readChar(1);

  CAN_cfg.speed = CAN_SPEED_125KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_5;
  CAN_cfg.rx_pin_id = GPIO_NUM_4;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  ESP32Can.CANInit();

  xTaskCreatePinnedToCore(task0, "Task0", 4096, NULL, 1, NULL, 0);
}

void loop() {
  CAN_frame_t rx_frame;
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {

    if(rx_frame.MsgID == 0x00400066){
      // printf(" from 0x%08X, DLC %d, Data ", rx_frame.MsgID,  rx_frame.FIR.B.DLC);
      // for (int i = 0; i < rx_frame.FIR.B.DLC; i++) {
      //   printf("0x%02X ", rx_frame.data.u8[i]);
      // }
      // printf("\n");

      if(rx_frame.data.u8[5] == 0x49){
        printf("reset\n\n");
        current_brightness_level = 12;
        EEPROM.writeChar(1, 12);
        EEPROM.commit();
      } else if(rx_frame.data.u8[5] == 0x51){
        printf("zero\n\n");
        current_brightness_level = 0;
        EEPROM.writeChar(1, 0);
        EEPROM.commit();
      } else if(rx_frame.data.u8[5] == 0x43){
        printf("up\n\n");
        current_display_mode = RTI_PAL;
        EEPROM.writeChar(0, RTI_PAL);
        EEPROM.commit();
      } else if(rx_frame.data.u8[5] == 0x45){
        printf("bottom\n\n");
        current_display_mode = RTI_OFF;
        EEPROM.writeChar(0, RTI_OFF);
        EEPROM.commit();
      }
    }
  }
}
