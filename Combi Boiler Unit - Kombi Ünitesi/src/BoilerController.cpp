#include "BoilerController.h"

// 1. YAPICI METOT (Constructor)
BoilerController::BoilerController(uint8_t pin) {
    this->relayPin = pin;
    this->currentRelayState = false; // Başlangıçta kombi her zaman kapalı olmalı
    this->lastHeartbeatTime = 0;
}

// 2. DONANIMI BAŞLATMA (Active-LOW için Revize Edildi)
void BoilerController::begin() {
    digitalWrite(relayPin, LOW); // Transistörü kapalı başlat (Röle Durur)
    pinMode(relayPin, OUTPUT);   
    Serial.println("BoilerController: Donanim baslatildi. Role KAPALI.");
    feedWatchdog(); 
}

// 3. RÖLE KONTROLÜ (Active-LOW için Revize Edildi)
void BoilerController::setRelay(bool state) {
    if (this->currentRelayState != state) {
        this->currentRelayState = state;
        digitalWrite(relayPin, state ? HIGH : LOW); // HIGH = YANAR, LOW = DURUR
        Serial.print("BoilerController: Kombi Durumu -> ");
        Serial.println(state ? "YANIYOR [ON]" : "DURDU [OFF]");
    }
}

// 4. BEKÇİ KÖPEĞİNİ BESLEME (Zaman Damgasını Güncelleme)
void BoilerController::feedWatchdog() {
    // Oda ünitesinden her paket geldiğinde bu fonksiyon çağrılacak
    this->lastHeartbeatTime = millis(); 
}

// 5. GÜVENLİK DENETÇİSİ (Sürekli saate bakan fonksiyon)
void BoilerController::checkWatchdog() {
    unsigned long currentMillis = millis();
    
    // Eğer şu anki zaman ile son haberleşme arasındaki fark 15 dakikayı (WATCHDOG_TIMEOUT) geçtiyse...
    if (currentMillis - lastHeartbeatTime > WATCHDOG_TIMEOUT) {
        
        // Ve eğer kombi o an YANIYORSA...
        if (currentRelayState == true) {
            Serial.println("KRITIK ALARM: 15 dakikadir Oda Unitesinden haber alinamiyor!");
            Serial.println("KRITIK ALARM: Iletisim koptu. Watchdog kombiyi ZORLA KAPATIYOR!");
            
            setRelay(false); // Evi fırına çevirmemek için sistemi acil durdur!
        }
    }
}