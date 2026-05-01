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

#include "packet_structure.h"

// #define HEADER_SIZE 10
// uint8_t header[HEADER_SIZE];

// header index mapping
// #define PKT_TYPE    0
// #define FLAGS       1
// #define NODE_ID_S   2
// #define USER_ID_S   3
// #define NODE_ID_R   4
// #define USER_ID_R   5
// #define MSG_ID      6
// #define PAY_LEN     7
// #define CRC_MSB     8
// #define CRC_LSB     9

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
#define FLAG_ACK   0x80     // 1000 0000

// 
struct lora_packet{
    uint8_t PKT_TYPE;
    uint8_t FLAGS;
    uint8_t USER_ID_S;
    uint8_t NODE_ID_S;
    uint8_t USER_ID_R;
    uint8_t NODE_ID_R;
    uint8_t MSG_ID;
    uint8_t PAY_LEN;
    uint8_t PAYLOAD[50];
};

lora_packet_struct get_rts(uint8_t user_id_s , uint8_t node_id_s, uint8_t user_id_r, uint8_t node_id_r, uint8_t msg_id){
    lora_packet_struct my_pkt = {0};
    my_pkt.PKT_TYPE = PKT_RTS;
    my_pkt.FLAGS = FLAG_RTS;
    my_pkt.USER_ID_S = user_id_s;
    my_pkt.NODE_ID_S = node_id_s;
    my_pkt.USER_ID_R = user_id_r;
    my_pkt.NODE_ID_R = node_id_r;
    my_pkt.MSG_ID = msg_id;
    my_pkt.PAY_LEN = 0x00;
    return my_pkt;
}

lora_packet_struct get_cts(uint8_t user_id_s, uint8_t node_id_s, uint8_t user_id_r, uint8_t node_id_r, uint8_t msg_id){
  lora_packet_struct my_pkt = {0};
  my_pkt.PKT_TYPE = PKT_CTS;
  my_pkt.FLAGS = FLAG_CTS;
  my_pkt.USER_ID_S = user_id_s;
  my_pkt.NODE_ID_S = node_id_s;
  my_pkt.USER_ID_R = user_id_r;
  my_pkt.NODE_ID_R = node_id_r;
  my_pkt.MSG_ID = msg_id;
  my_pkt.PAY_LEN = 0x00;
  return my_pkt;
}
lora_packet_struct get_data_pkt(uint8_t user_id_s, uint8_t node_id_s, uint8_t user_id_r, uint8_t node_id_r, uint8_t msg_id, const std::string msg){
  lora_packet_struct my_pkt = {0};
  my_pkt.PKT_TYPE = PKT_DATA;
  my_pkt.FLAGS = FLAG_DATA;
  my_pkt.USER_ID_S = user_id_s;
  my_pkt.NODE_ID_S = node_id_s;
  my_pkt.USER_ID_R = user_id_r;
  my_pkt.NODE_ID_R = node_id_r;
  my_pkt.MSG_ID = msg_id;
  my_pkt.PAY_LEN = msg.length();

  if(my_pkt.PAY_LEN > sizeof(my_pkt.PAYLOAD)){
    my_pkt.PAY_LEN = sizeof(my_pkt.PAYLOAD);  // trucate the packet
  }
  memcpy(my_pkt.PAYLOAD, msg.c_str(), my_pkt.PAY_LEN);
  return my_pkt;
}

lora_packet_struct get_ack(uint8_t user_id_s, uint8_t node_id_s, uint8_t user_id_r, uint8_t node_id_r, uint8_t msg_id){
  lora_packet_struct my_pkt = {0};
  my_pkt.PKT_TYPE = PKT_ACK;
  my_pkt.FLAGS = FLAG_ACK;
  my_pkt.USER_ID_S = user_id_s;
  my_pkt.NODE_ID_S = node_id_s;
  my_pkt.USER_ID_R = user_id_r;
  my_pkt.NODE_ID_R = node_id_r;
  my_pkt.MSG_ID = msg_id;
  my_pkt.PAY_LEN = 0x00;
  return my_pkt;
}