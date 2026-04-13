#include "NetworkManager.h"

NetworkManager::NetworkManager() {
    // Yapıcı metot şimdilik boş
}

bool NetworkManager::initESPNow() {
    WiFi.mode(WIFI_STA); // Cihazı Wi-Fi istasyon moduna al
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("HATA: ESP-NOW baslatilamadi!");
        return false;
    }
    
    Serial.println("NetworkManager: ESP-NOW Alici olarak baslatildi.");
    Serial.print("Kombi Cihazi MAC Adresi: ");
    Serial.println(WiFi.macAddress());
    
    return true;
}