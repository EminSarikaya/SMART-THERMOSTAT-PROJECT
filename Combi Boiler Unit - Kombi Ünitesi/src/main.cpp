#include <Arduino.h>
#include <esp_now.h>
#include "Protocol.h"
#include "BoilerController.h"
#include "NetworkManager.h"

// ==========================================
// 1. DONANIM PİNLERİ
// ==========================================
#define RELAY_PIN 26 // BC546 Transistörünün Base bacağına giden pin (Kendi tasarımınıza göre güncelleyin)

// ==========================================
// 2. NESNELER
// ==========================================
BoilerController boiler(RELAY_PIN);
NetworkManager network;

// ==========================================
// 3. GLOBAL HABERLEŞME DEĞİŞKENLERİ (YENİ)
// ==========================================
volatile bool newPacketReceived = false;
DataPacket lastReceivedPacket;

// ==========================================
// 4. ESP-NOW ALICI KESMESİ (Sadece Hamallık Yapar)
// ==========================================
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    if (len == sizeof(DataPacket)) {
        // SADECE veriyi güvenli bölgeye kopyala ve bayrağı kaldır. 
        // ASLA donanım tetikleme veya Serial.print yapma!
        memcpy((void*)&lastReceivedPacket, incomingData, sizeof(DataPacket));
        newPacketReceived = true; 
    }
}

// ==========================================
// 4. SETUP VE LOOP
// ==========================================
void setup() {
    Serial.begin(115200);
    
    // 1. Kombi donanımını ve güvenliğini başlat
    boiler.begin(); 
    
    // 2. ESP-NOW ağını başlat
    if (network.initESPNow()) {
        // Ağ başarılıysa, Alıcı Kesmesini (OnDataRecv) ESP-NOW çekirdeğine kaydet
        esp_now_register_recv_cb(OnDataRecv);
    }
}

void loop() {
    // Kombi ünitesi sürekli bekçi köpeğini kontrol eder
    boiler.checkWatchdog();
    
    // Eğer Wi-Fi kesmesi yeni bir paket bıraktıysa, ağır işleri burada güvenle yap
    if (newPacketReceived) {
        newPacketReceived = false; // İşleme başladık, bayrağı indir
        
        boiler.feedWatchdog();
        boiler.setRelay(lastReceivedPacket.isRelayActive);
        
        Serial.printf("Paket Alindi! Hedef: %.1f, Mevcut: %.1f, Role Emri: %d\n", 
                      lastReceivedPacket.targetTemperature, 
                      lastReceivedPacket.currentTemperature, 
                      lastReceivedPacket.isRelayActive);
    }
}