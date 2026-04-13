#ifndef BOILER_CONTROLLER_H
#define BOILER_CONTROLLER_H

#include <Arduino.h>

class BoilerController {
private:
    uint8_t relayPin;
    bool currentRelayState;
    
    // Watchdog (Güvenlik) Değişkenleri
    unsigned long lastHeartbeatTime; 
    const unsigned long WATCHDOG_TIMEOUT = 15 * 60 * 1000; // 15 Dakika (milisaniye cinsinden)

public:
    BoilerController(uint8_t pin);
    
    void begin(); // Pin modlarını (OUTPUT) ayarlamak için
    void setRelay(bool state); // Ağdan gelen emri röleye basmak için
    
    void feedWatchdog(); // Yeni paket geldiğinde köpeği beslemek (zamanı sıfırlamak) için
    void checkWatchdog(); // loop() içinde sürekli süreyi kontrol etmek için
};

#endif