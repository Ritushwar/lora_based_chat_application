// // simply sending data from Sender
// #include <LoRa.h>
// #include <SPI.h>

// const long frequency = 433E6;
// const int cs = 5;
// const int reset = 2;
// const int inte = 4;
// int led = 13;
// void setup() {
//   // put your setup code here, to run once:
//   Serial.begin(115200);
//   while(!Serial);
//   // initialize lora with pin
//   LoRa.setPins(cs, reset, inte);
//   if(!(LoRa.begin(frequency))){
//     Serial.print("LoRa init failed....");
//     delay(1000);
//     return;
//   }
//   Serial.print("LoRa init success......");
//   pinMode(inte, INPUT);
//   pinMode(led, OUTPUT);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   LoRa.beginPacket();
//   LoRa.println("This is the message from outerspace");
//   LoRa.endPacket();
//   LoRa.flush();

//   if(!digitalRead(8)){
//     Serial.println("It is not transmitting the data");
//     digitalWrite(led, HIGH);
//     delay(10);
//     digitalWrite(led, LOW);
//     delay(10);
//   }
//   delay(1000);
// }

#include <SPI.h>
#include <LoRa.h>

const long frequency = 433E6;

// ESP32 ↔ LoRa pin mapping
const int cs    = 5;   // NSS
const int reset = 2;   // RESET
const int dio0  = 4;   // DIO0

const int led = 12;    // LED pin

volatile bool txDoneFlag = false;  // ISR-safe flag

// Runs when transmission is complete (ISR)
void onTxDone() {
  txDoneFlag = true;   // ✅ only set a flag
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);

  LoRa.setPins(cs, reset, dio0);

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed");
    while (1);
  }

  LoRa.onTxDone(onTxDone);

  Serial.println("LoRa init success");
}

void loop() {
  // Send packet
  LoRa.beginPacket();
  LoRa.print("This is the message from outerspace");
  LoRa.endPacket(true);   // async transmit

  // Blink LED when TX is done
  if (txDoneFlag) {
    txDoneFlag = false;

    Serial.println("LoRa packet transmitted");
    digitalWrite(led, HIGH);
    delay(200);
    digitalWrite(led, LOW);
  }

  delay(1000);
}
