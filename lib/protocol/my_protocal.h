#ifndef PROTOCOL_STATE_H
#define PROTOCOL_STATE_H


#include <stdint.h>
#include <string>
#include "lora_setup.h"
#include "packet_structure.h"
#include "msg_queue.h"
#include "global_data.h"
#include "handle_json.h"

extern uint8_t msg_id;
extern uint8_t msg_id_r;
// extern uint8_t my_user_id;
// extern uint8_t my_node_id;

// extern uint8_t user_id_r;
// extern uint8_t node_id_r;

extern std:: string msg;
extern std:: string msg_r;

// void set_my_add(uint8_t, uint8_t);
// void set_r_add(uint8_t, uint8_t);
void ble_update();
void lora_update();
void fsm_update();
bool is_for_me(uint8_t, uint8_t);
void reset_channel();
#endif