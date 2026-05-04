#include "handle_json.h"
BLEmsg msg;
void parseBLEData(std::string jsonString){
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return;
    }

    // extract value


    msg.msg_id_s = doc["messageId"];
    Serial.print("MessageID: ");
    Serial.println(msg.msg_id_s);

    if (doc["type"].is<const char*>()) {
        msg.type = atoi(doc["type"]);
    } else {
        msg.type = doc["type"];
    }
    Serial.println(msg.type);

    msg.text = doc["text"].as<const char*>();
    Serial.print("Text: ");
    Serial.println(msg.text.c_str());

    msg.sender_id = doc["senderId"];
    Serial.print("SenderId: ");
    Serial.println(msg.sender_id);

    msg.receiver_id = doc["receiverId"];
    Serial.print("ReceiverId: ");
    Serial.println(msg.receiver_id);

    // filter out for initialization or msg
    if(msg.type == 00){
        // this for initialization
        Serial.println("Initializing the address of device....");
        globalData.u_id_s = msg.sender_id;
        globalData.u_id_r = msg.receiver_id;
        Serial.println("Initialization complete.");
    }

    if(msg.type == 01){
        // we need to append this in the queue
        msgQueue.push(msg);
        Serial.println("Message append to the queue");
    }
}

std::string jsonGenerate(notification &my_notification){
    JsonDocument doc;
    
    doc["messageId"] = my_notification.msg_id;
    doc["type"]      = my_notification.type;
    doc["receiver_id"] = my_notification.receiver_id;
    doc["type"] = my_notification.type;
    doc["text"] = my_notification.text.c_str();
    doc["status"] = my_notification.status;

    std::string output;
    serializeJson(doc, output);

    return output;
}