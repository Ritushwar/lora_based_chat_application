// this code test the two communication between esp32 and bluetooth serial terminal using ble

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define NUS_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_RX_UUID      "6e400002-b5a3-f393-e0a9-e50e24dcca9e" // Phone → ESP
#define NUS_TX_UUID      "6e400003-b5a3-f393-e0a9-e50e24dcca9e" // ESP → Phone

BLECharacteristic *txChar;
bool deviceConnected = false;
bool notifyEnabled = false;


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
      }
  }
};


void setup() {
  Serial.begin(9600);

  BLEDevice::init("ESP32_BLE_CHAT");

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
}

void loop() {
  if (deviceConnected && notifyEnabled) {
    txChar->setValue("ESP32 alive");
    txChar->notify();
    delay(2000);
  }
}
