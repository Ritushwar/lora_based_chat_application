// This is to design the correct header format for two way reliable communication using lora

/*  
  Byte 0: Packet Type      (1B)
  Byte 1: Flag / Status    (1B)
  Byte 2  : Node ID (Sender)   (1B)
  Byte 3  : User ID (Sender)   (1B)
  Byte 4  : Node ID (Receiver) (1B)
  Byte 5  : User ID (Receiver) (1B)
  Byte 6  : Message ID         (1B)
  Byte 7  : Payload Length     (1B)
  Byte 8  : CRC (MSB)          (1B)
  Byte 9  : CRC (LSB)          (1B)
--------------------------------
  Byte 10+: Payload (N bytes)
*/

// let's define constraints

// #include <ble_setup.h>
#include<global_data.h>
#include<my_protocal.h>
#include<lora_setup.h>
#include<ble_setup.h>

#define HEADER_SIZE 10
uint8_t header[HEADER_SIZE];

// header index mapping
#define PKT_TYPE    0
#define FLAGS       1
#define NODE_ID_S   2
#define USER_ID_S   3
#define NODE_ID_R   4
#define USER_ID_R   5
#define MSG_ID      6
#define PAY_LEN     7
#define CRC_MSB     8
#define CRC_LSB     9

// packets types
#define PKT_RTS    0x01
#define PKT_CTS    0x02
#define PKT_DATA   0x03
#define PKT_ACK    0x04
#define PKT_HEADER 0x05

// flags
#define FLAG_RTS   0x01     // 0000 0001
#define FLAG_CTS   0x02     // 0000 0010
#define FLAG_SENT  0x04     // 0000 0100
#define FLAG_DEL   0x08     // 0000 1000
#define FLAG_ERROR 0x10     // 0001 0000
#define FLAG_RETRY 0x20     // 0010 0000
#define FLAG_DATA  0x40     // 0100 0000


// message id
uint8_t msg_id = 0x00;

// payload length
uint8_t len_pay = 0x00;

//crc
uint16_t crc = 0x0000 ;    // dummy crc
uint8_t crc_msb = (crc >> 8) & 0xFF; 
uint8_t crc_lsb =  crc & 0xFF;

   
enum ProtoState {
    STATE_IDLE,          // normal state
    STATE_SEND_RTS,      // to send rts
    STATE_WAIT_CTS,      // waiting for cts
    STATE_SEND_DATA,     // to send data
    STATE_WAIT_ACK,      // waiting for ack
    STATE_SUCCESS,       // successfully message was send
    STATE_SEND_CTS,      // to send cts
    STATE_RECEIVED_DATA, // to receive data
    STATE_SEND_ACK,      // to send ack
    STATE_ERROR          // error state
};

// Private state variable
static ProtoState protoState = STATE_IDLE;
// static std::string txBuffer;

static unsigned long stateTimer = 0;
static uint8_t retryCount = 0;

// ProtoState protoState = STATE_IDLE;
// unsigned long protoTimer = 0;
const unsigned long CTS_TIMEOUT = 10000;     // 10 sec

void protocol_sendMessage(const std::string& msg){
  if(protoState != STATE_IDLE){
    Serial.println("Protocol Busy");
    return;
  }
  Serial.println("Success in assign to global variable");
  globalData.my_msg = msg;
  protoState = STATE_SEND_RTS;
};

void set_default_header(){
  // default header is set to send RTS packet
  header[PKT_TYPE]  = PKT_RTS;
  header[FLAGS]     = FLAG_RTS;
  header[NODE_ID_S] = globalData.NODE_ID;
  header[USER_ID_S] = globalData.USER_ID;
  header[NODE_ID_R] = globalData.node_id_r;
  header[USER_ID_R] = globalData.user_id_r;
  header[MSG_ID]    = 0x00;
  header[PAY_LEN]   = 0x00;
  header[CRC_MSB]   = 0x00;
  header[CRC_LSB]   = 0x00;
};

// to check that packet is for me
bool isForMe(uint8_t *hdr){
  // return true if this packet is for me
  return (hdr[NODE_ID_R] == globalData.NODE_ID &&
          hdr[USER_ID_R] == globalData.USER_ID);
}

void protocol_handlePacket(uint8_t* packet, uint8_t len){
  Serial.println("Received header PKT type:");
  Serial.println(packet[0], HEX);
  if(packet[PKT_TYPE] == PKT_CTS){
    Serial.println("CTS is received");
    if(!isForMe(packet)) return;
    Serial.print("CTS is for me, i am sending data.");
    protoState = STATE_SEND_DATA;
  }
  else if (packet[PKT_TYPE] == PKT_DATA)
  {
    Serial.println("Data packet is received");
    if(!isForMe(packet)) return;
    Serial.println("And it for me");
    globalData.received_msg = std::string((char*)(packet+HEADER_SIZE), len-HEADER_SIZE);
    protoState = STATE_RECEIVED_DATA;
  }
  else if(packet[PKT_TYPE] == PKT_ACK){
    if(!isForMe(packet)) return;
    Serial.println("ACK packet is received");
    protoState = STATE_SUCCESS;
  }
  else if (packet[PKT_TYPE] == PKT_RTS)
  {
    Serial.println("RTS is received");
    if(!isForMe(packet)) return;
    Serial.println("This packet is for me, i am sending cts packet");
    protoState = STATE_SEND_CTS;
  }
  else{
    Serial.print("Nothing match");
  }
}
uint8_t get_msg_id(){
  msg_id++;
  return msg_id;
}



void prepare_payload(uint8_t* payload, int pay_len){
  set_default_header();
  header[PKT_TYPE]  = PKT_DATA;
  header[FLAGS]     = FLAG_DATA;
  header[MSG_ID]    = get_msg_id();
  header[PAY_LEN]   = pay_len;
  header[CRC_MSB]   = 0x00;
  header[CRC_LSB]   = 0x00;

  for(int i = 0; i<pay_len; i++){
    payload[i] = globalData.my_msg[i];
    Serial.println(payload[i]);
  }
}

void protocol_task(){

  switch(protoState){

    case STATE_IDLE:
      receivePKT(HEADER_SIZE,PAY_LEN);
      break;

    case STATE_SEND_RTS:{
      set_default_header();
      sendRTS(header ,HEADER_SIZE);
      // stateTimer = millis();
      protoState = STATE_WAIT_CTS;
      break;
    }  

    case STATE_WAIT_CTS:{
      receivePKT(HEADER_SIZE,PAY_LEN);
      break;
    }

    case STATE_SEND_DATA: {
        len_pay = globalData.my_msg.length();
        uint8_t payload[len_pay];
        prepare_payload(payload, len_pay);
        Serial.println("My payload");
        for(int i =0; i<len_pay; i++){
          Serial.print(payload[i], HEX);
        }
        sendDATA(header, payload);
        protoState = STATE_WAIT_ACK;
        break;
    }

    case STATE_WAIT_ACK:{
        // if(millis() - stateTimer > ACK_TIMEOUT){
        // handleRetry();
        receivePKT(HEADER_SIZE,PAY_LEN);
        break;
    }

    case STATE_SEND_CTS:{
        set_default_header();
        header[PKT_TYPE] = PKT_CTS;
        header[FLAGS]    = FLAG_CTS;
        delay(200);
        sendPKT(header);
        Serial.println("Set in idle state");
        protoState = STATE_IDLE;
        break;
    }

    case STATE_RECEIVED_DATA:
        notify_app();
        protoState = STATE_SEND_ACK;

    case STATE_SEND_ACK:
         set_default_header();
         header[PKT_TYPE] = PKT_ACK;
         header[FLAGS]    = FLAG_SENT;
         delay(200);
         sendACK(header);
         protoState = STATE_SUCCESS;

    case STATE_SUCCESS:
        Serial.println("Transmission successful");
        retryCount = 0;
        protoState = STATE_IDLE;
        break;
    case STATE_ERROR:
        Serial.println("Transmission failed");
        retryCount = 0;
        protoState = STATE_IDLE;
        break;
}
};
