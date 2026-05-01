#ifndef BLE_SETUP_H
#define BLE_SETUP_H

#include <stdint.h>
#include <string.h>
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <SPI.h>
#include <LoRa.h>
#include <global_data.h>
#include "msg_queue.h"


#define NUS_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_RX_UUID      "6e400002-b5a3-f393-e0a9-e50e24dcca9e" // Phone → ESP
#define NUS_TX_UUID      "6e400003-b5a3-f393-e0a9-e50e24dcca9e" // ESP → Phone


void setup_ble_communication(String);
void notify_app();
#endif
