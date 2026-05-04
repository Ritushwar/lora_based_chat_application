#include "global_data.h"
void set_node_id(uint8_t sender,uint8_t receiver){
    globalData.n_id_s = sender;
    globalData.n_id_r = receiver;
}
GlobalData globalData;