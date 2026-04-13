#include "UIManager.h"

UIManager::UIManager(Adafruit_SH1106G* disp, float initialTarget, float initialHysteresis) {
    this->display = disp;
    this->currentState = STATE_HOME;
    this->menuIndex = 0;
    this->uiTargetTemp = initialTarget;
    this->currentEnvTemp = 0.0;
    this->isRelayActive = false;
    this->needsRedraw = true; 
    this->targetTempChanged = false;
    this->uiHysteresis = initialHysteresis;
    this->hysteresisChanged = false;
    this->uiOffset = 0.0; 
    this->offsetChanged = false;
}

// ==========================================
// KULLANICI GİRDİSİ (BUTON VE ENKODER) YÖNETİMİ
// ==========================================
void UIManager::processInput(ButtonEvent event, int encoderTicks) {
    // Eğer hiçbir şey yapılmadıysa çık
    if (event == BTN_NONE && encoderTicks == 0) return;

    // Bir şey yapıldıysa ekranı kesinlikle yenilememiz gerekecek
    needsRedraw = true; 

    // HANGİ EKRANDAYIZ? Ona göre tepki ver
    switch (currentState) {
        
        case STATE_HOME:
            if (event == BTN_SHORT_PRESS) {
                currentState = STATE_MAIN_MENU; // Menüye geç
                menuIndex = 0;
            } 
            else if (event == BTN_LONG_PRESS) {
                currentState = STATE_SUMMER_MODE; // Yaz moduna geç
            }
            else if (encoderTicks != 0) {
                // Ana ekranda çevrilirse sıcaklığı ayarla
                uiTargetTemp += (encoderTicks * 0.5);
                uiTargetTemp = constrain(uiTargetTemp, 5.0, 35.0);
                targetTempChanged = true; // Termostata haber vermek için bayrağı kaldır
            }
            break;

        case STATE_MAIN_MENU:
            if (encoderTicks != 0) {
                menuIndex += encoderTicks;
                if (menuIndex < 0) menuIndex = 0;
                if (menuIndex >= MAX_MENU_ITEMS) menuIndex = MAX_MENU_ITEMS - 1;
            }
            else if (event == BTN_SHORT_PRESS) {
                if (menuIndex == 0) { 
                    currentState = STATE_SET_HYSTERESIS; 
                } else if (menuIndex == 1) { 
                    currentState = STATE_SET_OFFSET; // YENİ: Kalibrasyon sayfasına geç
                } else if (menuIndex == 2) {
                    currentState = STATE_HOME;       // 3. Seçenek Çıkış oldu
                }
            }
            break;

        case STATE_SET_HYSTERESIS: 
            if (encoderTicks != 0) {
                uiHysteresis += (encoderTicks * 0.1);
                uiHysteresis = constrain(uiHysteresis, 0.1, 2.0);
            } 
            else if (event == BTN_SHORT_PRESS) {
                hysteresisChanged = true; 
                currentState = STATE_HOME;
            }
            break;

        case STATE_SET_OFFSET: // MİMARİDEN EKSİK OLAN BLOK EKLENDİ
            if (encoderTicks != 0) {
                uiOffset += (encoderTicks * 0.1); 
                uiOffset = constrain(uiOffset, -5.0, 5.0); 
            } 
            else if (event == BTN_SHORT_PRESS) {
                offsetChanged = true; 
                currentState = STATE_HOME;
            }
            break;
        case STATE_SUMMER_MODE:
            // Yaz modundayken çevirmenin etkisi yok
            if (event == BTN_LONG_PRESS) {
                currentState = STATE_HOME; // Kış moduna geri dön
            }
            break;
    }
}

// ==========================================
// SENSÖR VERİSİ GÜNCELLEMESİ
// ==========================================
void UIManager::updateSensorData(float envTemp, bool relayState) {
    // Sadece sıcaklık gerçekten değiştiyse veya röle durumu değiştiyse ekranı yenile
    if (abs(this->currentEnvTemp - envTemp) >= 0.1 || this->isRelayActive != relayState) {
        this->currentEnvTemp = envTemp;
        this->isRelayActive = relayState;
        this->needsRedraw = true; 
    }
}

// ==========================================
// ÇİZİM MOTORU (Sadece ihtiyaç varsa çizer)
// ==========================================
void UIManager::updateScreen() {
    if (!needsRedraw) return; 

    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(1);

    if (currentState == STATE_HOME) drawHome();
    else if (currentState == STATE_MAIN_MENU) drawMainMenu();
    else if (currentState == STATE_SUMMER_MODE) drawSummerMode();
    else if (currentState == STATE_SET_HYSTERESIS) drawSetHysteresis();
    else if (currentState == STATE_SET_OFFSET) drawSetOffset(); // EKSİK OLAN SATIR EKLENDİ

    display->display();
    needsRedraw = false; 
}

// ==========================================
// ÖZEL EKRAN TASARIMLARI
// ==========================================
void UIManager::drawHome() {
    display->setCursor(0, 5);
    display->setTextSize(3);
    display->print(currentEnvTemp, 1); 

    display->setTextSize(1);
    display->setCursor(75, 5);
    display->print((char)247); // DERECE SEMBOLÜ
    display->print("C");

    display->setCursor(0, 45);
    display->setTextSize(1);
    display->print("Hedef: ");
    display->print(uiTargetTemp, 1);
    display->print((char)247); // DERECE SEMBOLÜ
    display->print("C");

    if (isRelayActive) {
        display->drawBitmap(105, 5, icon_flame_16x16, 16, 16, 1);
        display->setCursor(105, 25);
        display->print("ON");
    } else {
        display->drawBitmap(105, 5, icon_snow_16x16, 16, 16, 1);
        display->setCursor(102, 25);
        display->print("OFF"); 
    }
}

void UIManager::drawMainMenu() {
    display->setCursor(0, 0);
    display->println("--- ANA MENU ---");
    
    if (menuIndex == 0) display->print("> "); else display->print("  ");
    display->println("Tolerans Ayari");
    
    if (menuIndex == 1) display->print("> "); else display->print("  ");
    display->println("Kalibrasyon"); // EKSİK OLAN MENÜ EKLENDİ
    
    if (menuIndex == 2) display->print("> "); else display->print("  ");
    display->println("Cikis");
}

void UIManager::drawSummerMode() {
    display->setCursor(0, 0); 
    display->println("--- YAZ MODU ---");
    display->setCursor(0, 20); 
    display->setTextSize(2);
    display->print(currentEnvTemp, 1); 
    display->print((char)247); // DERECE SEMBOLÜ
    display->println("C");
    
    display->setTextSize(1); 
    display->setCursor(0, 45); 
    display->println("Kombi Kilitli.");
}

// ==========================================
// BİLGİ AKTARIM KAPILARI
// ==========================================
bool UIManager::isTargetChanged() { return targetTempChanged; }
float UIManager::getNewTargetTemp() { return uiTargetTemp; }
void UIManager::clearTargetChangeFlag() { targetTempChanged = false; }
AppState UIManager::getCurrentState() { return currentState; }

void UIManager::drawSetHysteresis() {
    display->setCursor(0, 0); 
    display->println("--- TOLERANS ---");
    display->setCursor(0, 20); 
    display->setTextSize(2);
    display->print("+- "); 
    display->print(uiHysteresis, 1); 
    display->print((char)247); // DERECE SEMBOLÜ
    display->println("C");
    
    display->setTextSize(1); 
    display->setCursor(0, 45); 
    display->println("Onaylamak icin bas");
}

bool UIManager::isHysteresisChanged() { return hysteresisChanged; }
float UIManager::getNewHysteresis() { return uiHysteresis; }
void UIManager::clearHysteresisChangeFlag() { hysteresisChanged = false; }

void UIManager::drawSetOffset() {
    display->setCursor(0, 0); 
    display->println("--- KALIBRASYON ---");
    display->setCursor(0, 20); 
    display->setTextSize(2);
    if (uiOffset > 0) display->print("+"); 
    display->print(uiOffset, 1); 
    display->print((char)247); // DERECE SEMBOLÜ
    display->println("C");
    
    display->setTextSize(1); 
    display->setCursor(0, 45); 
    display->println("Onaylamak icin bas");
}

// Dış dünyanın ofset değişiminden haberdar olma kapıları
bool UIManager::isOffsetChanged() { return offsetChanged; }
float UIManager::getNewOffset() { return uiOffset; }
void UIManager::clearOffsetChangeFlag() { offsetChanged = false; }