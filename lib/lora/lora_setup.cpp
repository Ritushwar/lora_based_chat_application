#include<lora_setup.h>
#include<my_protocal.h>
// LoRa Modes
#define RX_MODE 0
#define TX_MODE 1
int mode = RX_MODE;

void setup_lora_communication(){
  // setup LoRa
  LoRa.setPins(cs, reset, dio0);

  pinMode(led_when_receive, OUTPUT);
  pinMode(led_on_r_mode, OUTPUT);
  pinMode(led_on_t_mode, OUTPUT);

  if (!LoRa.begin(frequency)){
    Serial.println("LoRa init failed");
    while (1);
    }
  setMode(RX_MODE);          // by default
  Serial.println("LoRa ready to operate");
};

void setMode(int stataus){
    if(stataus== RX_MODE){
      LoRa.receive();                       // set to receive mode
      digitalWrite(led_on_t_mode, LOW);
      digitalWrite(led_on_r_mode, HIGH);    // indicate my led
    }
    else{
      digitalWrite(led_on_r_mode, LOW);      // set to transmit mode indicate my led
      digitalWrite(led_on_t_mode, HIGH);      
    }
};

void sendRTS(uint8_t * header, int header_size){
  // set to transmitting mode
  mode = TX_MODE;
  setMode(mode);
  delay(20);

  Serial.println("Sending RTS.....");
  LoRa.beginPacket();
  LoRa.write(header, header_size);
  LoRa.endPacket();
  Serial.println("RTS has been sent");

  mode = RX_MODE;
  setMode(mode);
  delay(200);
}

void receivePKT(int header_size, int pay_pos){
  if(mode == RX_MODE){
    int packetSize = LoRa.parsePacket();
    if(packetSize >= header_size){
      Serial.println("Packet is received");
      uint8_t rxHeader[header_size];
      LoRa.readBytes(rxHeader, header_size);
      Serial.print("Header is received:");
      for(int i =0; i<header_size; i++){
        Serial.print(rxHeader[i], HEX);
      }
      Serial.println(" ");


      int payload_len = rxHeader[pay_pos];
      // if payload is available
      if(payload_len>0 && packetSize >= header_size + payload_len){
        uint8_t payload[payload_len];
        LoRa.readBytes(payload, payload_len);

        Serial.println("Payload is also attached");
        uint8_t merged[header_size + payload_len];
        memcpy(merged, rxHeader, header_size);
        memcpy(merged+header_size, payload, payload_len);

        protocol_handlePacket(merged, header_size + payload_len);
        return;
                
      }
      // pass to protocol to further process
      protocol_handlePacket(rxHeader, header_size);
    }
  }
}

void sendDATA(uint8_t * header, uint8_t * payload){
  mode = TX_MODE;
  setMode(mode);

  Serial.println("Sending payload with data");
  // send header and payload at once
  LoRa.beginPacket();
  LoRa.write(header, 10);      // header size is fixed
  LoRa.write(payload, globalData.my_msg.length());
  LoRa.endPacket();

  Serial.println("Succes in sending");
  mode = RX_MODE;
  setMode(mode);
  delay(50);
}

void sendPKT(uint8_t * header){
  Serial.println("Sending CTS");
  Serial.print("My packet is: ");
  for(int i =0; i<10; i++){
    Serial.print(header[i], HEX);
  }
  Serial.println(" ");
  mode = TX_MODE;
  setMode(mode);
  LoRa.beginPacket();
  LoRa.write(header, 10); 
  LoRa.endPacket();
  Serial.println("CTS is sent");
  mode = RX_MODE;
  setMode(mode);
  delay(10);
}
void sendACK(uint8_t * header){
  Serial.println("Sending ACK");
  Serial.print("My packet is: ");
  for(int i =0; i<10; i++){
    Serial.print(header[i], HEX);
  }
  Serial.println(" ");
  mode = TX_MODE;
  setMode(mode);
  LoRa.beginPacket();
  LoRa.write(header, 10); 
  LoRa.endPacket();
  Serial.println("ACK is sent");
  mode = RX_MODE;
  setMode(mode);
  delay(10);
}