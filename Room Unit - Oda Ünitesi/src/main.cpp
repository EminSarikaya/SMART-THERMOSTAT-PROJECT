#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#include "NetworkManager.h"
#include "ThermostatController.h"
#include "ButtonManager.h"
#include "UIManager.h" // YENİ: Arayüz Yöneticimiz

// ==========================================
// 1. DONANIM PİNLERİ VE AYARLAR
// ==========================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define ENCODER_CLK 25
#define ENCODER_DT 26
#define ENCODER_SW 27

const unsigned long SENSOR_READ_INTERVAL = 2000; 

// ==========================================
// 2. NESNELER (OBJECTS)
// ==========================================
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ThermostatController thermostat(23.0, 0.5); 
Adafruit_BMP280 bme;
uint8_t kombiMacAddress[] = {0x28, 0x05, 0xA5, 0x2D, 0x03, 0x9C};

NetworkManager network(kombiMacAddress);
ButtonManager btn(ENCODER_SW);
UIManager ui(&display, 23.0, 0.5); // YENİ: Ekran yöneticisi 23 derece ile başlar

// ==========================================
// 3. GLOBAL VE VOLATILE DEĞİŞKENLER
// ==========================================
volatile int encoderTicks = 0;   
volatile unsigned long lastInterruptTime = 0; 
unsigned long lastSensorReadTime = 0; 

// ==========================================
// KESME FONKSİYONU (ISR) - Sadece yön okur
// ==========================================
void IRAM_ATTR readEncoderISR() {
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 50) {
        if (digitalRead(ENCODER_DT) == LOW) {
            encoderTicks--; 
        } else {
            encoderTicks++; 
        }
        lastInterruptTime = interruptTime;
    }
}

// ==========================================
// KURULUM (BOOT SEQUENCE)
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(2000); // Voltaj stabilizasyonu

    if(!display.begin(0x3C, true)) {
        Serial.println("HATA: OLED bulunamadi!");
    } else {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(1);
        display.setCursor(0,0);
        display.print("Sistem Basliyor...");
        display.display();
    }

    if (!bme.begin(0x76)) {
        Serial.println("HATA: BME280 bulunamadi!");
    }

    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    btn.begin(); // SW pini pull-up yapıldı
    
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoderISR, FALLING);

    delay(500); 
    if (!network.initESPNow()) {
        Serial.println("HATA: Ag kurulamadi!");
    }
}

// ==========================================
// ANA DÖNGÜ (ORKESTRA ŞEFİ)
// ==========================================
void loop() {
    // 1. DONANIM GİRDİLERİNİ TOPLA
    ButtonEvent event = btn.read();
    int currentTicks = encoderTicks;
    encoderTicks = 0; // Tıkları hemen sıfırla ki ISR birikmesin

    // 2. GİRDİLERİ ARAYÜZE (DURUM MAKİNESİNE) YÖNLENDİR
    ui.processInput(event, currentTicks);

    // 3. KULLANICI ARAYÜZDE BİR DEĞERİ DEĞİŞTİRDİYSE YAKALA VE TERMOSTATA BİLDİR
    if (ui.isTargetChanged()) {
        thermostat.setTargetTemperature(ui.getNewTargetTemp());
        ui.clearTargetChangeFlag();
    }
    
    if (ui.isHysteresisChanged()) {
        thermostat.setHysteresis(ui.getNewHysteresis());
        ui.clearHysteresisChangeFlag();
        Serial.println("SISTEM BILGISI: Tolerans Guncellendi!");
    }
    
    if (ui.isOffsetChanged()) {
        thermostat.setTemperatureOffset(ui.getNewOffset());
        ui.clearOffsetChangeFlag();
        Serial.println("SISTEM BILGISI: Kalibrasyon Ofseti Guncellendi!");
    }
    
    // 4. SENSÖR OKUMA VE TERMOSTAT GÜNCELLEME
    unsigned long currentMillis = millis();
    if (currentMillis - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
        lastSensorReadTime = currentMillis; 
        float rawTemp = bme.readTemperature(); // Ham değer
        thermostat.updateCurrentTemperature(rawTemp); // Ofset termostat içinde uygulanır
        
        // Ekrana termostatın hesapladığı OFSETLİ sıcaklığı gönderin
        ui.updateSensorData(thermostat.getCurrentTemp(), thermostat.getRelayState());
    }

    // 5. YAZ MODU GÜVENLİK KİLİDİ (KILL SWITCH)
    if (ui.getCurrentState() == STATE_SUMMER_MODE) {
        thermostat.setTargetTemperature(5.0); 
    }

    // 6. EKRANI GÜNCELLE
    ui.updateScreen();

    // 7. AĞ İLETİŞİMİ VE KARGO (ESP-NOW)
    if (thermostat.requiresNetworkUpdate()) {
        DataPacket packet = thermostat.generatePacket();
        network.sendData(packet);
        Serial.println("Bilgi: Durum Kombi Unitesine firlatildi.");
    }
}