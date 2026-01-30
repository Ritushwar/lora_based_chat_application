// this code test the two communication between esp32 and bluetooth serial terminal using ble

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

// to set the mode of LoRa
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

// call_back function to enale notifications
class CCCDCallbacks : public BLEDescriptorCallbacks {
  void onWrite(BLEDescriptor *pDesc) {
    uint8_t* value = pDesc->getValue();
    notifyEnabled = (value[0] & 0x01);
    Serial.print("Notify enabled: ");
    Serial.println(notifyEnabled);
  }
};

// call_back function to check the connection status of BLE
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

// call_back function when data is received from phone
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
      delay(50);

      LoRa.beginPacket();
      LoRa.print(rxValue.c_str());
      LoRa.endPacket();

      Serial.print("Sent: ");
      Serial.println(rxValue.c_str());

      mode = RX_MODE;
      setMode(mode);
      }
  }
};


void setup() {

  // setup serial communication
  Serial.begin(9600);

  // setup BLE
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

  // setup BLE
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
}

void loop() {
  if (deviceConnected && notifyEnabled) {
    if(mode == RX_MODE){
      int packetSize = LoRa.parsePacket();

      if(packetSize){
        String incoming = "";

        while(LoRa.available()){
          incoming += (char)LoRa.read();
        }
        digitalWrite(led_when_receive, HIGH);
        delay(100);
        digitalWrite(led_when_receive, LOW);

        txChar->setValue(incoming.c_str());
        txChar->notify();
        delay(100);
      }
    }
  }
}
