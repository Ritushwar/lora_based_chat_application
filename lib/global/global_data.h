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
    
    uint8_t u_id_s;
    uint8_t n_id_s;

    uint8_t u_id_r;
    uint8_t n_id_r;
    std::string ble_name = "";
    bool notification_flag = false;
    std::string received_msg = "";   // filled by handle json after sending fom ble
};

void set_node_id(uint8_t,uint8_t);
extern GlobalData globalData;