#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <stdint.h>
#include <string.h>
#include <Arduino.h>

struct  GlobalData
{
    bool deviceConnected = false;
    bool notifyEnabled = false;
    u_int8_t payload_len = 0x00;
    std::string ble_name;
    std::string my_msg;
    std::string received_msg;
    uint8_t NODE_ID   = 0x01;
    uint8_t USER_ID   = 0x01;
    uint8_t node_id_r = 0x02;
    uint8_t user_id_r = 0x02;
    
};

extern GlobalData globalData;


#endif