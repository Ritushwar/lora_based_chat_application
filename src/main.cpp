#include <Arduino.h>
#include "ble_setup.h"
#include "my_protocal.h"
#include "global_data.h"
#include "msg_queue.h"


uint8_t u_id_s = 0x00;
uint8_t n_id_s = 0x00;
uint8_t u_id_r = 0x01;
uint8_t n_id_r = 0x01;

int chip_s = 5;
int reset = 2;
int intr = 22;
int led_on_t = 26;
int led_on_r = 25;

void setup(){
    Serial.begin(9600);

    setup_ble_communication("LoRa_Chat_Device_1");   // initial ble setup

    set_my_add(u_id_s, n_id_s);                      // my address
    set_r_add(u_id_r, n_id_r);                       // receiver address

    setup_lora_communication(chip_s, reset, intr, led_on_t, led_on_r);   // setup lora communication
}

void loop(){
    ble_update();
    lora_update();
    fsm_update();
    notify_app();
}