#include <ble_setup.h>
#include <my_protocal.h>

BLECharacteristic *txChar;

void notify_app(){
  txChar->setValue(globalData.received_msg.c_str());
  txChar->notify();
  delay(10);
}

class CCCDCallbacks : public BLEDescriptorCallbacks {
  void onWrite(BLEDescriptor *pDesc) {
    uint8_t* value = pDesc->getValue();
    globalData.notifyEnabled = (value[0] & 0x01);
    Serial.print("Notify enabled: ");
    Serial.println(globalData.notifyEnabled);
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) {
    globalData.deviceConnected = true;
    Serial.println("BLE client connected");
  }

  void onDisconnect(BLEServer*) {
    globalData.deviceConnected = false;
    Serial.println("BLE client disconnected");
    BLEDevice::startAdvertising();
  }
};

class RXCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    Serial.println(">>> RX CALLBACK TRIGGERED <<<");
    std::string app_msg = pChar->getValue();
    Serial.print("Length: ");
    Serial.println(app_msg.length());

    if (app_msg.length() > 0) {
      Serial.print("From phone: ");
      Serial.println(app_msg.c_str());
      Serial.println();
      protocol_sendMessage(app_msg);
    }
  }
};

void setup_ble_communication(String my_name){
  globalData.ble_name = std::string(my_name.c_str());
  BLEDevice::init(globalData.ble_name);

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

