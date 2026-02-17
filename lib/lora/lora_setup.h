#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include<global_data.h>

// LoRa Frequency
const long frequency = 433E6;

// ESP32 ↔ LoRa pin mapping
const int cs    = 5;   // NSS
const int reset = 2;   // RESET
const int dio0  = 22;   // DIO0 (RX interrupt)

const int led_when_receive = 13;  // Blinks on packet receive
const int led_on_r_mode    = 12;  // ON = receiver mode
const int led_on_t_mode    = 14;  // ON = transmitting mode

void setup_lora_communication();
void setMode(int stataus);
void sendRTS(uint8_t *, int);
void receivePKT(int, int);
void sendDATA(uint8_t *,  uint8_t *);
void sendPKT(uint8_t *);
void sendACK(uint8_t *);
