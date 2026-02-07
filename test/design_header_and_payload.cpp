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

#include<stdint.h>
#include<string.h>
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <SPI.h>
#include <LoRa.h>

#define NUS_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_RX_UUID      "6e400002-b5a3-f393-e0a9-e50e24dcca9e" // Phone → ESP
#define NUS_TX_UUID      "6e400003-b5a3-f393-e0a9-e50e24dcca9e" // ESP → Phone


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
#define FLAG_RETRY 0x20     // 0020 0000

// node id (sender)
#define NODE_ID  0x01
#define USER_ID  0x01
u_int8_t node_id_r = 0x02;
u_int8_t user_id_r = 0x02;

// user id (sender)
// this is send my app

// node id (receiver)
// this is fetch by sender

// user id (receiver)
// this is fetch by sender

// message id
uint8_t msg_id = 0x00;

// payload length
uint8_t len_pay = 0x00;

//crc
uint16_t crc = 0x0000 ;    // dummy crc
uint8_t crc_msb = (crc >> 8) & 0xFF; 
uint8_t crc_lsb =  crc & 0xFF;

// payload
// uint8_t payload [] = "Hello";
// uint8_t payloadLen = strlen((char*)payload);

// LoRa Modes

#define RX_MODE 0
#define TX_MODE 1
int mode = RX_MODE;


// LoRa Frequency
const long frequency = 433E6;

// ESP32 ↔ LoRa pin mapping
const int cs    = 5;   // NSS
const int reset = 2;   // RESET
const int dio0  = 4;   // DIO0 (RX interrupt)

const int led_when_receive = 13;  // Blinks on packet receive
const int led_on_r_mode    = 12;  // ON = receiver mode
const int led_on_t_mode    = 14;  // ON = transmitting mode   

BLECharacteristic *txChar;
bool deviceConnected = false;
bool notifyEnabled = false;

enum ProtoState{
  IDLE,
  WAIT_CTS
};

ProtoState protoState = IDLE;
unsigned long protoTimer = 0;
const unsigned long CTS_TIMEOUT = 2000;

void set_default_header(){
  // default header is set to send RTS Signal
  header[PKT_TYPE] = PKT_RTS;
  header[FLAGS] = FLAG_RTS;
  header[NODE_ID_S] = NODE_ID;
  header[USER_ID_S] = USER_ID;
  header[NODE_ID_R] = node_id_r;
  header[USER_ID_R] = user_id_r;
  header[MSG_ID]    = msg_id;
  header[PAY_LEN]   = len_pay;
  header[CRC_MSB]   = crc_msb;
  header[CRC_LSB]   = crc_lsb;
};

// to check that packet is for me
bool isForMe(uint8_t *hdr){
  // return true if this packet is for me
  return (hdr[NODE_ID_R] == NODE_ID &&
          hdr[USER_ID_R] == USER_ID);
}

void setup_ble_communication(){
  BLEDevice::init("ESP32_BLE_CHAT_OLD_PHONE");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *service = pServer->createService(NUS_SERVICE_UUID);

  // RX characteristics (WRITE from phone)
  BLECharacteristic *rxChar = service->createCharacteristic(
    NUS_RX_UUID,
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  rxChar->setCallbacks(new RXCallbacks());

  // TX characteristics (Notify Phone)
  txChar = service->createCharacteristic(
    NUS_TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_READ
  );

  BLE2902 *cccd = new BLE2902();
  cccd->setCallbacks(new CCCDCallbacks());
  txChar->addDescriptor(cccd);

  service->start();
  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(NUS_SERVICE_UUID);
  adv->setScanResponse(true);
  adv->setMinPreferred(0x06);  
  adv->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  Serial.println("BLE UART ready");

};

void setup_lora_communication(){
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
}

void sendRTS(){
  set_default_header();
  header[PKT_TYPE] = PKT_RTS;
  header[FLAGS]    = FLAG_RTS;
  header[PAY_LEN]  = 0;

  Serial.println("Sending RTS.....");
  LoRa.beginPacket();
  LoRa.write(header, HEADER_SIZE);
  LoRa.endPacket();

  protoState = WAIT_CTS;
  protoTimer = millis();
}

void handleRTS(uint8_t *hdr){
  if(!isForMe(hdr)) return;

  Serial.println("RTS received, sending CTS");

  uint8_t cts[HEADER_SIZE];
  memcpy(cts, hdr, HEADER_SIZE);

  cts[PKT_TYPE] = PKT_CTS;
  cts[FLAGS]    = FLAG_CTS;
  cts[NODE_ID_S]= NODE_ID;
  cts[USER_ID_S]= USER_ID;

  LoRa.beginPacket();
  LoRa.write(cts, HEADER_SIZE);
  LoRa.endPacket();
}

void handleCTS(uint8_t *hdr){
  if(!isForMe(hdr)) return;

  if(protoState == WAIT_CTS){
    Serial.println("CTS received");
    protoState = IDLE;
  }
}
class CCCDCallbacks : public BLEDescriptorCallbacks {
  void onWrite(BLEDescriptor *pDesc) {
    uint8_t* value = pDesc->getValue();
    notifyEnabled = (value[0] & 0x01);
    Serial.print("Notify enabled: ");
    Serial.println(notifyEnabled);
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) {
    deviceConnected = true;
    Serial.println("BLE client connected");
  }

  void onDisconnect(BLEServer*) {
    deviceConnected = false;
    Serial.println("BLE client disconnected");
    BLEDevice::startAdvertising();
  }
};

class RXCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    Serial.println(">>> RX CALLBACK TRIGGERED <<<");

    std::string rxValue = pChar->getValue();
    Serial.print("Length: ");
    Serial.println(rxValue.length());

    if (rxValue.length() > 0) {
      Serial.print("From phone: ");
      Serial.println(rxValue.c_str());
      Serial.println();

      // transmit it to destination through lora
      mode = TX_MODE;
      setMode(mode);
      delay(10);

      sendRTS();

      mode = RX_MODE;
      setMode(mode);

      // wait for CTS

      // CTS received then sent data packet

      // check the validity of CTS
      if(header[NODE_ID_R] == header[NODE_ID_S] && header[USER_ID_S] == header[USER_ID_S]){
        // encode optional
        // append data/string with header and transmit it through lora
        // notify app though ble
      }

      // if CTS is not received then try after some time notify the app

      // if data packet is sent then wait for ack


      // if positive ack is received then notify the app

      // if negative ack or no ack is received after timeout then re attemp to send the data packet

      }
  }
};

void send_rts(){
  int a = 10;  // remaining to define
}
void setup(){
  Serial.begin(9600);
  // setup header here
  set_default_header();

  // setup ble here
  setup_ble_communication();

  // setup LoRa
  setup_lora_communication();

  
}

void loop() {
  if (deviceConnected && notifyEnabled) {
    if(mode == RX_MODE){
      int packetSize = LoRa.parsePacket();

      if(packetSize >= HEADER_SIZE){
        uint8_t rxHeader[HEADER_SIZE];
        LoRa.readBytes(rxHeader, HEADER_SIZE);

        digitalWrite(led_when_receive, HIGH);
        delay(50);
        digitalWrite(led_when_receive, LOW);

        uint8_t pktType = rxHeader[PKT_TYPE];

        if(pktType == PKT_RTS){
          handleRTS(rxHeader);
        }
        else if(pktType == PKT_CTS){
          handleCTS(rxHeader);
        }
        // check for the type of packet

        // if packet type is RTS then check the receiver id
        // if id match then send CTS else do nothing

        // if packet type is payload then check the receiver id
        // if id match then encode(optional), notify the app then send ACK to sender
        // txChar->setValue(incoming.c_str());
        // txChar->notify();
        // delay(100);
      }
    }
  }

  if(protoState == WAIT_CTS){
    if(millis() - protoTimer > CTS_TIMEOUT){
      Serial.println("CTS timeout, retrying RTS");
      header[FLAGS] |= FLAG_RETRY;
      sendRTS();
    }
  }
}
