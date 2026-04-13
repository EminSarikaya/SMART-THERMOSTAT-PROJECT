#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

class NetworkManager {
public:
    NetworkManager();
    bool initESPNow(); // Sadece anteni ve protokolü başlatır
};

#endif