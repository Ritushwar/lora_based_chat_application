#include <Arduino.h>
#include "ble_setup.h"
#include "my_protocal.h"
#include "global_data.h"
#include "msg_queue.h"
#include <ArduinoJson.h>
#include <esp_system.h>

// we have to change the address for each device
uint8_t n_id_s = 0x00;
uint8_t n_id_r = 0x01;

int chip_s = 5;
int reset = 2;
int intr = 22;
int led_on_t = 26;
int led_on_r = 25;

 uint32_t random_delay;
void setup(){
    Serial.begin(9600);

    setup_ble_communication("LoRa_Chat_Device_1");   // initial ble setup
    set_node_id(n_id_s, n_id_r);                  // node id is fixed for receiver and sender

    setup_lora_communication(chip_s, reset, intr, led_on_t, led_on_r);   // setup lora communication
}

void loop(){
    // random_delay = (esp_random() % 101) + 100;    // random delay between 100ms to 200 ms
    lora_update();
    // delay(random_delay);

    // random_delay = (esp_random() % 101) + 100;
    ble_update();
    // delay(random_delay);

    // random_delay = (esp_random() % 101) + 100;
    fsm_update();
    // delay(random_delay);
    
    notify_app();
}