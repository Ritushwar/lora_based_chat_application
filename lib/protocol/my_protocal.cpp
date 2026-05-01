#include "my_protocal.h"

uint8_t my_user_id = 0x00;
uint8_t my_node_id = 0x00;
uint8_t user_id_r = 0x00;
uint8_t node_id_r = 0x00;
uint8_t msg_id    = 0x00;
uint8_t msg_id_r  = 0x00;
std::string msg   = "";
std::string msg_r = "";

#define TIMEOUT_MS 3000UL
#define POST_SUCCESS_DELAY 100UL
static unsigned long rts_start_ms = 0;
static unsigned long data_start_ms = 0;
static unsigned long cts_start_ms = 0;
static unsigned long success_start_ms = 0;
int max_retry = 3;
int rts_retry_c = 0;
int data_retry_c = 0;

enum ProtoState {
    IDLE,
    SEND_RTS,
    WAIT_CTS,
    SEND_DATA,
    WAIT_ACK,
    SUCCESS,
    SEND_CTS,
    WAIT_DATA,  
    SEND_ACK
};

static ProtoState protoState = IDLE;

static bool cts_received  = false;
static bool ack_received  = false;
static bool rts_received  = false;
static bool data_received = false;

static bool channel_busy = false;

void reset_channel() {
    cts_received  = false;
    ack_received  = false;
    rts_received  = false;
    data_received = false;
    channel_busy  = false;
    rts_retry_c   = 0;
    data_retry_c  = 0;
}

bool is_for_me(uint8_t u_id_r, uint8_t n_id_r){
    if(u_id_r== my_user_id && n_id_r == my_node_id){
        return true;
    }
    else{
        return false;
    }
}
void set_my_add(uint8_t user_id, uint8_t node_id) {
    my_user_id = user_id;
    my_node_id = node_id;
}

void set_r_add(uint8_t user_id, uint8_t node_id) {
    user_id_r = user_id;
    node_id_r = node_id;
}

void send_rts() {
    Serial.println("Sending RTS");
    send_packet(get_rts(my_user_id, my_node_id, user_id_r, node_id_r, msg_id));
}

void send_cts() {
    Serial.println("Sending CTS");
    send_packet(get_cts(my_user_id, my_node_id, user_id_r, node_id_r, msg_id_r));
}

void send_data() {
    Serial.println("Sending DATA");
    send_packet(get_data_pkt(my_user_id, my_node_id, user_id_r, node_id_r, msg_id, msg));
}

void send_ack() {
    Serial.println("Sending ACK");
    send_packet(get_ack(my_user_id, my_node_id, user_id_r, node_id_r, msg_id_r));
}

void ble_update(){
    if(msgQueue.available() && !channel_busy && protoState == IDLE){
        msg = msgQueue.pop();
        channel_busy = true;
        protoState = SEND_RTS;
    }

    if(msg_r.length() > 0){
        Serial.println("Received: " + String(msg_r.c_str()));
        globalData.received_msg = msg_r;
        msg_r.clear();
    }
    return;
}

void lora_update(){
    if(!is_packet_available())  return;

    int pkt_size = get_packet_size();
    if(pkt_size < 8){
        Serial.print("Invalid packet size of: ");
        Serial.println(pkt_size);
        lora_packet_struct random_pkt = get_packet();    // discard the packet
        return;
    }
    else{
       Serial.print("Valid packet size of: "); 
       Serial.println(pkt_size);
    }

    lora_packet_struct pkt = get_packet();
    
    // check if the packet is addressed to me
    if(!is_for_me(pkt.USER_ID_R, pkt.NODE_ID_R)){
        return;
    }

    switch (pkt.PKT_TYPE)
    {
    case 0x01:   // RTS
        if(!channel_busy){
            msg_id_r = pkt.MSG_ID;
            rts_received = true;
        }else{
            Serial.println("RTS ignored — channel busy");
        }
        break;
    
    case 0x02:    // CTS
        cts_received = true;
        break;
    
    case 0x03:    // DATA
        msg_id_r = pkt.MSG_ID;
        msg_r.assign((char*)pkt.PAYLOAD, pkt.PAY_LEN);
        data_received = true;
        break;
    
    case 0x04:  // ACK
        if(pkt.MSG_ID == msg_id){
            ack_received = true;
        }
        break;;    
    
    default:
        break;
    }
}

void fsm_update(){
    switch (protoState)
    {
    case IDLE:
        // small cooldown after sucess before accepting new RTS and success
        if (millis() - success_start_ms < POST_SUCCESS_DELAY) break;
        if(rts_received){
            rts_received = false;
            protoState = SEND_CTS;
        }
        break;
    
    case SEND_RTS:
        send_rts();
        rts_start_ms = millis();
        protoState = WAIT_CTS;
        break;
    
    case WAIT_CTS:
        if (cts_received){
            cts_received = false;
            protoState = SEND_DATA;
        }
        else if (millis()- rts_start_ms > TIMEOUT_MS)
        {
            Serial.println("Timeout waiting for CTS, retrying....");
            rts_retry_c++;
            if(rts_retry_c <= max_retry){
                protoState = SEND_RTS;
            }else{
                reset_channel();
                protoState = IDLE;
            }

        }
        break;

    case SEND_DATA:
        send_data();
        data_start_ms = millis();
        protoState = WAIT_ACK;
        break;
    
    case WAIT_ACK:
        if(ack_received){
            ack_received = false;
            protoState = SUCCESS;
        }
        else if(millis() - data_start_ms > TIMEOUT_MS){
            Serial.println("Timeout waiting for ack, retrying ......");
            data_retry_c++;
            if(data_retry_c <= max_retry){
                protoState = SEND_DATA;
            }else{
                reset_channel();
                protoState = IDLE;
            }
        }
        break;
    
    case SUCCESS:
        Serial.println("Message sent successfully!");
        msg_id++;
        success_start_ms = millis();
        reset_channel();
        protoState = IDLE;
        break;
    
    case SEND_CTS:
        send_cts();
        cts_start_ms = millis();
        protoState = WAIT_DATA;
        break;
    
    case WAIT_DATA:
        if(data_received){
            data_received = false;
            protoState = SEND_ACK;
        }
        else if(millis() - cts_start_ms > TIMEOUT_MS){
            Serial.println("Timeout waiting for DATA, return to IDLE");
            reset_channel();
            protoState = IDLE;
        }
        break;
    
    case SEND_ACK:
        send_ack();
        reset_channel();
        protoState = IDLE;
        break;

    default:
        break;
    }
}