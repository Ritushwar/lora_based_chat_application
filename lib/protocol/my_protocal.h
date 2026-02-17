#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <global_data.h>
#include <Arduino.h>
#include <string>

void protocol_init();
void protocol_task();
void protocol_sendMessage(const std::string& msg);
void protocol_handlePacket(uint8_t* data, uint8_t len);
void set_pins();


#endif
