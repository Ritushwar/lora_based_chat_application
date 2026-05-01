#pragma once
#include <string>

struct GlobalData {
    bool deviceConnected = false;
    bool notifyEnabled   = false;

    int chip_s;
    int reset;
    int intr;
    int led_on_t_mode;
    int led_on_r_mode;
    
    std::string ble_name = "";
    std::string received_msg = "";   // filled by LoRa FSM, read by BLE notify
};

extern GlobalData globalData;