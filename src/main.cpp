#include <Arduino.h>
#include "ble_setup.h"
#include "my_protocal.h"
#include "global_data.h"
#include "msg_queue.h"
#include <ArduinoJson.h>

// we have to change the address for each device
uint8_t n_id_s = 0x01;
uint8_t n_id_r = 0x00;

int chip_s = 5;
int reset = 2;
int intr = 22;
int led_on_t = 26;
int led_on_r = 25;

void setup(){
    Serial.begin(9600);

    setup_ble_communication("LoRa_Chat_Device_1");   // initial ble setup
    set_node_id(n_id_s, n_id_r);                  // node id is fixed for receiver and sender

    setup_lora_communication(chip_s, reset, intr, led_on_t, led_on_r);   // setup lora communication
}

void loop(){
    ble_update();
    lora_update();
    fsm_update();
    notify_app();
}