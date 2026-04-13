#pragma once
#include <Arduino.h>
#include <esp_now.h>
#include "Protocol.h" 
#include <WiFi.h>

class NetworkManager {
private:
    // Sınıfın kendi sırrı (Dışarıdan kesinlikle erişilemez)
    uint8_t targetMac[6]; 

public:
    // Kurucu Fonksiyon: MAC adresini artık nesne yaratılırken dışarıdan alacak
    NetworkManager(uint8_t* macAddress); 
    
    bool initESPNow(); 
    bool sendData(DataPacket data); 
};