#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "ButtonManager.h" 
#include "Icons.h" 

enum AppState {
    STATE_HOME,             
    STATE_MAIN_MENU,        
    STATE_SUMMER_MODE,      
    STATE_SET_HYSTERESIS,   // DÜZELTİLDİ: Virgül eklendi
    STATE_SET_OFFSET
};

class UIManager {
private:
    Adafruit_SH1106G* display; 
    
    AppState currentState;
    int menuIndex;
    const int MAX_MENU_ITEMS = 3; 
    
    bool needsRedraw; 

    float uiTargetTemp;
    float currentEnvTemp;
    bool isRelayActive;
    bool targetTempChanged; 

    float uiHysteresis;
    bool hysteresisChanged;

    // MİMARİ DÜZELTME: Bu değişkenler private olmak ZORUNDADIR.
    float uiOffset; 
    bool offsetChanged;

    void drawHome();
    void drawMainMenu();
    void drawSummerMode();
    void drawSetHysteresis(); 
    void drawSetOffset(); // YENİ: Çizim fonksiyonunun deklarasyonu

public:
    UIManager(Adafruit_SH1106G* disp, float initialTarget, float initialHysteresis);

    void processInput(ButtonEvent event, int encoderTicks);
    void updateSensorData(float envTemp, bool relayState);
    void updateScreen(); 

    bool isTargetChanged();
    float getNewTargetTemp();
    void clearTargetChangeFlag();
    AppState getCurrentState(); 

    bool isHysteresisChanged();
    float getNewHysteresis();
    void clearHysteresisChangeFlag();
    
    // YENİ: Kalibrasyon iletişim kapıları (Getter/Setter mantığı)
    bool isOffsetChanged();
    float getNewOffset();
    void clearOffsetChangeFlag();
};