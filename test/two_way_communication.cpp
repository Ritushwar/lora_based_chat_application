#include <SPI.h>
#include <LoRa.h>

const long frequency = 433E6;

// ESP32 ↔ LoRa pin mapping
const int cs    = 5;   // NSS
const int reset = 2;   // RESET
const int dio0  = 4;   // DIO0 (RX interrupt)

const int led_when_receive = 13;  // Blinks on packet receive
const int led_on_r_mode    = 12;  // ON = receiver mode
const int led              = 14;  // to indicate the data
const int button           = 21;  // to transfer the data    
bool led_status            = false;   // status of led 

// Modes
#define RX_MODE 0
#define TX_MODE 1
int mode = RX_MODE;

void setMode(int stataus){
    if(stataus== RX_MODE){
        LoRa.receive();                       // set to receive mode
        digitalWrite(led_on_r_mode, HIGH);    // indicate my led
    }
    else{
        digitalWrite(led_on_r_mode, LOW);      // set to transmit mode indicate my led
    }
}

void setup(){
    Serial.begin(9600);
    LoRa.setPins(cs, reset, dio0);

    pinMode(led_when_receive, OUTPUT);
    pinMode(led_on_r_mode, OUTPUT);
    pinMode(button, INPUT);
    pinMode(led, OUTPUT);


    if (!LoRa.begin(frequency)){
        Serial.println("LoRa init failed");
        while (1);
                }
    // LoRa.onReceive(onReceive);

    Serial.println("LoRa ready to operate");
    setMode(RX_MODE);              // by default
}

void loop(){
    if(mode == RX_MODE){
        int packetSize = LoRa.parsePacket();

        if (packetSize) {
            String incoming = "";

            while (LoRa.available()) {
                incoming += (char)LoRa.read();
            }
        digitalWrite(led_when_receive, HIGH);
        delay(100);
        digitalWrite(led_when_receive, LOW);

        if(incoming == "led_on"){
            digitalWrite(led, HIGH);
        }
        if(incoming == "led_off"){
            digitalWrite(led, LOW);
        }
        // Serial.print("Received: ");
        // Serial.println(incoming);
        }
    }

    static unsigned long lastPress = 0;
    if(digitalRead(button) == LOW){  // pulled up
        // String outgoing = Serial.readStringUntil('\n');
        if(millis() - lastPress > 300){
            led_status = !led_status;
            String outgoing;
            if(led_status){
                outgoing = "led_on";
            }else{
                outgoing = "led_off";
            }
            mode = TX_MODE;
            setMode(mode);

            delay(50);

            LoRa.beginPacket();
            LoRa.print(outgoing);
            LoRa.endPacket();

            Serial.print("Sent: ");
            Serial.println(outgoing);

            mode = RX_MODE;     // go back to receiving mode
            setMode(mode);
        }
    }
}