#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include<Arduino.h>
#include<string.h>

// === BLE SETTINGS ===
#define DEVICE_NAME "WHEELCHAIR_01"
#define serviceUUID "12345678-1234-1234-1234-1234567890ab"
#define charUUID "abcd1234-1234-1234-1234-abcdef123456"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;


// Server connect/disconnect callback
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE Device Connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE Device Disconnected");
    // Restart advertising
    pServer->getAdvertising()->start();
  }
};

// Characteristic write callback
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    std::string rxValue = pChar->getValue();  
    if (rxValue.length() > 0) {
      Serial.print("Command Received: ");
      Serial.println(rxValue.c_str());
    }
  }

};

void setup() {
  Serial.begin(9600);

  // Initialize BLE
  BLEDevice::init(DEVICE_NAME);

  // Create BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(serviceUUID);

  // Create BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      charUUID,
      BLECharacteristic::PROPERTY_WRITE |
      BLECharacteristic::PROPERTY_WRITE_NR
  );

  // pCharacteristic->addDescriptor(new BLE2902());
  // pCharacteristic->setCallbacks(new MyCallbacks());

  BLE2902 *cccd = new BLE2902();
  pCharacteristic->addDescriptor(cccd);
  pCharacteristic->setCallbacks(new MyCallbacks());
  // Start the service
  pService->start();

  // Start advertising
  // pServer->getAdvertising()->start();
  // Serial.println("BLE Test Device Started, waiting for connection...");

  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(serviceUUID);
  adv->setScanResponse(true);
  adv->setMinPreferred(0x06);  
  adv->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  Serial.println("Ready to receive command");
}

void loop() {
  // BLE works via callbacks, nothing required here
}
