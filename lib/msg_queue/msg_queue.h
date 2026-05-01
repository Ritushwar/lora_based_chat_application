#pragma once
#include <string>
#include <queue>

// Single shared queue — BLE and Serial both push here,
// protocol FSM pops from here
struct MsgQueue {
    std::queue<std::string> q;

    void push(const std::string& msg) {
        if (q.size() < 10) {   // cap at 10 to avoid memory issues on ESP32
            q.push(msg);
        }
    }

    bool available() {
        return !q.empty();
    }

    std::string pop() {
        std::string msg = q.front();
        q.pop();
        return msg;
    }
};

extern MsgQueue msgQueue;   // one global instance shared across all modules