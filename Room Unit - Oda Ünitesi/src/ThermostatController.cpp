#include "ThermostatController.h"

// 1. Kurucu Fonksiyon (Constructor)
// Sistem ilk açıldığında hedefleri ve başlangıç durumlarını belirleriz.
ThermostatController::ThermostatController(float initialTarget, float hysteresis) {
    targetTemperature = initialTarget;
    hysteresisBand = hysteresis;
    currentTemperature = 0.0;
    isRelayActive = false; // Kombi kapalı başlar
    lastHeartbeatTime = 0; // 0 olması, sistem açılır açılmaz ağa ilk paketin fırlatılmasını sağlar
    stateChanged = true;
    this->temperatureOffset = 0.0; // Varsayılan ofset 0
}

// 2. Kapsüllenmiş (Gizli) Histerezis Algoritması
void ThermostatController::evaluateTemperature() {
    // Kombiyi açma şartı (Alt Limit)
    if (currentTemperature <= (targetTemperature - hysteresisBand)) {
        isRelayActive = true; 
    } 
    // Kombiyi kapatma şartı (Üst Limit)
    else if (currentTemperature >= (targetTemperature + hysteresisBand)) {
        isRelayActive = false;
    }
}

// 3. Sensörden Gelen Veriyi Alma Kapısı
void ThermostatController::updateCurrentTemperature(float sensorTemp) {
    bool oldRelayState = isRelayActive; // Eski durumu kaydet
    
    currentTemperature = sensorTemp + temperatureOffset;
    evaluateTemperature(); 
    
    // Eğer sıcaklık değişimi röleyi açıp/kapatmışsa, anında haber ver!
    if (oldRelayState != isRelayActive) {
        stateChanged = true;
    }
}

void ThermostatController::setTemperatureOffset(float offset) {
    this->temperatureOffset = offset;
    // Ofset değiştiği için mevcut sıcaklık ve röle durumu anında yeniden hesaplanmalı
    evaluateTemperature(); 
    stateChanged = true;
}
float ThermostatController::getTemperatureOffset() { return temperatureOffset; }
float ThermostatController::getCurrentTemp() { return currentTemperature; }

// 4. Enkoderden Gelen Hedef Verisini Alma Kapısı
void ThermostatController::setTargetTemperature(float newTemp) {
    float constrainedTemp = constrain(newTemp, 5.0, 35.0);
    
    if (this->targetTemperature != constrainedTemp) {
        this->targetTemperature = constrainedTemp;
        evaluateTemperature(); // Yeni hedefe göre röle durumunu anında hesapla!
        stateChanged = true;   // Acil gönderim bayrağını kaldır!
    }
}

// 5. Durum Bildirme (Ekran için)
bool ThermostatController::getRelayState() {
    return isRelayActive;
}

// 6. Zaman ve Olay Yönetimi (Orkestra Şefine "Fırlat" emri veren fonksiyon)
bool ThermostatController::requiresNetworkUpdate() {
    unsigned long currentTime = millis(); 
    
    // ŞART: 60 saniye dolduysa VEYA acil durum bayrağı kalktıysa
    if (currentTime - lastHeartbeatTime >= HEARTBEAT_INTERVAL || stateChanged) {
        lastHeartbeatTime = currentTime; 
        stateChanged = false; // Gönderim izni verildi, bayrağı indir
        return true; 
    }
    
    return false; 
}

// 7. Kargo Paketini Hazırlama
DataPacket ThermostatController::generatePacket() {
    DataPacket packet;
    packet.messageType = 1; // Standart kontrol verisi
    packet.currentTemperature = currentTemperature;
    packet.targetTemperature = targetTemperature;
    packet.isRelayActive = isRelayActive;
    return packet; // Hazırlanan paketi teslim et
}

void ThermostatController::setHysteresis(float newHysteresis) {
    // Sınırlandırma (Örn: 0.1 ile 2.0 derece arası)
    this->hysteresisBand = constrain(newHysteresis, 0.1, 2.0);
    evaluateTemperature(); // Tolerans değiştiği için röle durumunu anında tekrar hesapla
    stateChanged = true;   // Ağ üzerinden Kombi'ye hemen haber ver
}

float ThermostatController::getHysteresis() {
    return this->hysteresisBand;
}
