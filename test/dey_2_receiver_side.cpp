#include <SPI.h>
#include <LoRa.h>

const long frequency = 433E6;

// ESP32 ↔ LoRa pin mapping
const int cs    = 5;   // NSS
const int reset = 2;   // RESET
const int dio0  = 4;   // DIO0 (RX interrupt)

const int led_when_receive = 13;  // Blinks on packet receive
const int led_on_r_mode    = 21;  // ON = receiver mode

volatile bool rxFlag = false;
String receivedMessage = "";

// ISR: called when a packet is received
void onReceive(int packetSize) {
  if (packetSize == 0) return;

  receivedMessage = "";
  while (LoRa.available()) {
    receivedMessage += (char)LoRa.read();
  }
  rxFlag = true;   // signal main loop
}

void setup() {
  Serial.begin(115200);

  pinMode(led_when_receive, OUTPUT);
  pinMode(led_on_r_mode, OUTPUT);

  digitalWrite(led_on_r_mode, HIGH);  // receiver mode ON

  LoRa.setPins(cs, reset, dio0);

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed");
    while (1);
  }

  // Attach RX interrupt
  LoRa.onReceive(onReceive);
  LoRa.receive();   // put LoRa into RX mode

  Serial.println("LoRa Receiver ready");
}

void loop() {
  // If a packet was received
  if (rxFlag) {
    rxFlag = false;

    Serial.print("Received message: ");
    Serial.println(receivedMessage);

    // Blink LED on receive
    digitalWrite(led_when_receive, HIGH);
    delay(100);
    digitalWrite(led_when_receive, LOW);
  }
}
