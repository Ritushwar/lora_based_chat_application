#ifndef SETUP_LORA_H
#define SETUP_LORA_H

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "packet_structure.h"
#include "global_data.h"

// LoRa Frequency
const long frequency = 433E6;
extern volatile bool receivedFlag;

// methods
void setup_lora_communication(const int, const int, const int, const int, const int);
void setMode(int);
void send_packet(const lora_packet_struct&);
int get_packet_size();
lora_packet_struct get_packet();
bool is_packet_available();
#endif