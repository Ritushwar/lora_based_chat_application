#include "my_protocal.h"
uint8_t msg_id_r  = 0x00;
std::string msg_r = "";
BLEmsg my_msg;
notification my_noti;

#define TIMEOUT_MS 2000UL
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
    if(u_id_r== globalData.u_id_s && n_id_r == globalData.n_id_s){
        return true;
    }
    else{
        return false;
    }
}

void send_rts() {
    Serial.println("Sending RTS");
    send_packet(get_rts(my_msg.sender_id, globalData.n_id_s, my_msg.receiver_id, globalData.n_id_r, my_msg.msg_id_s));
}

void send_cts() {
    Serial.println("Sending CTS");
    send_packet(get_cts(globalData.u_id_s, globalData.n_id_s, globalData.u_id_r, globalData.n_id_r, msg_id_r));
}

void send_data() {
    Serial.println("Sending DATA");
    send_packet(get_data_pkt(my_msg.sender_id, globalData.n_id_s, my_msg.receiver_id, globalData.n_id_r, my_msg.msg_id_s, my_msg.text));
}

void send_ack() {
    Serial.println("Sending ACK");
    send_packet(get_ack(globalData.u_id_s, globalData.n_id_s, globalData.u_id_r, globalData.n_id_r, msg_id_r));
}

void ble_update(){
    if(msgQueue.available() && !channel_busy && protoState == IDLE){
        my_msg = msgQueue.pop();
        channel_busy = true;
        protoState = SEND_RTS;
    }

    if(notificationQueue.available() && protoState == IDLE){
        globalData.notification_flag = true;
        notification my_notifi = notificationQueue.pop();
        globalData.received_msg = jsonGenerate(my_notifi);
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
        Serial.println("This packet id not for me:"); 
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
        msg_r.assign((char*)pkt.PAYLOAD, pkt.PAY_LEN);
        
        my_noti.msg_id = pkt.MSG_ID;
        my_noti.sender_id = pkt.USER_ID_S;
        my_noti.receiver_id = pkt.USER_ID_R;
        my_noti.type = "00";                // msg
        my_noti.status = "00";              // received
        my_noti.text = msg_r;
        msg_r.clear();

        // push to the queue
        notificationQueue.push(my_noti);
        data_received = true;
        break;
    
    case 0x04:  // ACK
        if(pkt.MSG_ID == my_msg.msg_id_s){
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
            if(rts_retry_c < max_retry){
                protoState = SEND_RTS;
            }else{
                Serial.println("Receiver not availabe to receive");
                reset_channel();
                my_noti.msg_id = my_msg.msg_id_s;
                my_noti.sender_id = my_msg.sender_id;
                my_noti.receiver_id = my_msg.receiver_id;
                my_noti.type = "01";     // ack
                my_noti.status = "02";   // delivered
                my_noti.text = "";       // null

                // push to the queue
                notificationQueue.push(my_noti);
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
        success_start_ms = millis();
        reset_channel();
        
        // queue the notification
        my_noti.msg_id = my_msg.msg_id_s;
        my_noti.sender_id = my_msg.sender_id;
        my_noti.receiver_id = my_msg.receiver_id;
        my_noti.type = "01";     // ack
        my_noti.status = "01";   // delivered
        my_noti.text = "";       // null

        // push to the queue
        notificationQueue.push(my_noti);
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