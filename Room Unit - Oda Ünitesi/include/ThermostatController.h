#pragma once
#include <Arduino.h>
#include "Protocol.h" 

class ThermostatController {
private:
    // Kasanın İçindeki Sırlar (State Variables)
    float currentTemperature;
    float targetTemperature;
    float hysteresisBand;
    bool isRelayActive;
    bool stateChanged;
    
    // YENİ: Kalibrasyon Ofseti (Dışarıdan doğrudan erişilemez)
    float temperatureOffset;
    
    // Zaman Yönetimi 
    unsigned long lastHeartbeatTime;
    const unsigned long HEARTBEAT_INTERVAL = 60000; 
    
    // İç Hesaplama Fonksiyonu
    void evaluateTemperature();

public:
    // Kurucu Fonksiyon
    ThermostatController(float initialTarget, float hysteresis);
    
    // Dış Dünyadan Veri Alma Kapıları
    void updateCurrentTemperature(float sensorTemp);
    void setTargetTemperature(float newTarget);
    
    // Histerezis (Tolerans) Ayar Kapıları
    void setHysteresis(float newHysteresis);
    float getHysteresis();
    
    // YENİ: Kalibrasyon (Ofset) Ayar Kapıları BURADA DEKLARE EDİLMELİDİR
    void setTemperatureOffset(float offset);
    float getTemperatureOffset();
    float getCurrentTemp(); // Ofset uygulanmış gerçek sıcaklığı okumak için
    
    // Dış Dünyaya Veri ve Emir Verme Kapıları
    bool getRelayState();             
    bool requiresNetworkUpdate();     
    DataPacket generatePacket();      
};