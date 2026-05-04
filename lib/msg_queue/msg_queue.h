#pragma once
#include <string>
#include <queue>

// queue — Data is pushed here after parsingjson
// protocol FSM pops from here

struct BLEmsg
{
    uint8_t msg_id_s;
    uint8_t type;
    uint8_t sender_id;
    uint8_t receiver_id;
    std::string text;
};

struct notification{
    uint8_t msg_id;
    std::string type;
    std::string text;
    std::string status;
    uint8_t sender_id;
    uint8_t receiver_id;
};

struct MsgQueue {
    std::queue<BLEmsg> q;

    void push(const BLEmsg& msg) {
        if (q.size() < 5) {   // cap at 5 to avoid memory issues on ESP32
            q.push(msg);
        }
    }

    bool available() {
        return !q.empty();
    }

    BLEmsg pop() {
        BLEmsg msg = q.front();
        q.pop();
        return msg;
    }
};

struct NotificationQueue{
    std::queue<notification> q;

    void push(const notification& noti){
        if(q.size() <5){
            q.push(noti);
        }
    }

    bool available(){
        return !q.empty();
    }

    notification pop(){
        notification my_noti = q.front();
        q.pop();
        return my_noti;
    }
};

extern MsgQueue msgQueue;   // one global instance shared across all modules
extern NotificationQueue notificationQueue;