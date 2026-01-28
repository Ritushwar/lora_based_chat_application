// simply sending data from Sender
#include <LoRa.h>
#include <SPI.h>

const long frequency = 433E6;
const int cs = GPIO_NUM_5;
const int reset = GPIO_NUM_2;
const int inte = GPIO_NUM_4;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial);
  // initialize lora with pin
  LoRa.setPins(cs, reset, inte);
  if(!(LoRa.begin(frequency))){
    Serial.print("LoRa init failed....");
    delay(1000);
    return;
  }
  Serial.print("LoRa init success......");
}

void loop() {
  // put your main code here, to run repeatedly:
  LoRa.beginPacket();
  LoRa.println("This is the message from outerspace");
  LoRa.endPacket();
  LoRa.flush();

  Serial.println("It is working");
  delay(1000);
}
