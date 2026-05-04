#ifndef HANDLE_JSON
#define HANDLE_JSON
#include <Arduino.h>
#include <ArduinoJson.h>
#include "msg_queue.h"
#include "global_data.h"
void parseBLEData(std::string);
std::string jsonGenerate(notification &);
#endif
