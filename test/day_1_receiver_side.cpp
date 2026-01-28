// simply receiving data from Sender
#include <LoRa.h>
#include <SPI.h>

const long frequency = 433E6;
const int cs = 5;
const int reset = 2;
const int inte = 4;
int led_when_receive = 22;
int led_on_r_mode = 21;
void setup() {
  // put your setup code here, to run once:
  pinMode(led_when_receive, OUTPUT);
  pinMode(led_on_r_mode, OUTPUT);
  Serial.begin(115200);
  Serial.print("Serial begin");
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
  int packet_size = LoRa.parsePacket();
  if(packet_size != 0){
    String message = LoRa.readString();
    Serial.print("Message is: ");
    Serial.println(message);
    digitalWrite(led_when_receive, HIGH);
    delay(100);
    digitalWrite(led_when_receive, LOW);
    delay(100);
  }
  digitalWrite(led_on_r_mode, HIGH);
  delay(50);
  digitalWrite(led_on_r_mode, LOW);
  delay(50);
  Serial.println("*");
}
