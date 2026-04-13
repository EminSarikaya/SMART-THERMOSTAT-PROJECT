#include "ButtonManager.h"

ButtonManager::ButtonManager(uint8_t pin) {
    this->buttonPin = pin;
    this->isPressed = false;
    this->longPressHandled = false;
    this->pressStartTime = 0;
}

void ButtonManager::begin() {
    // Enkoder butonları genelde INPUT_PULLUP gerektirir (Basılınca LOW olur)
    pinMode(buttonPin, INPUT_PULLUP);
}

ButtonEvent ButtonManager::read() {
    // Buton LOW ise basılmıştır
    bool currentState = (digitalRead(buttonPin) == LOW);
    unsigned long currentTime = millis();

    // 1. DURUM: Butona ilk basıldığı an
    if (currentState && !isPressed) {
        isPressed = true;
        longPressHandled = false;
        pressStartTime = currentTime;
        return BTN_NONE; // Henüz kısa mı uzun mu bilmiyoruz, beklemeye devam
    }

    // 2. DURUM: Buton hala basılı tutuluyor
    if (currentState && isPressed) {
        // Eğer uzun basma süresi aşıldıysa ve daha önce tetiklenmediyse
        if (!longPressHandled && (currentTime - pressStartTime >= LONG_PRESS_TIME)) {
            longPressHandled = true; // Sadece 1 kez tetiklenmesini garanti et
            return BTN_LONG_PRESS;   // UZUN BASIŞI FIRLAT!
        }
    }

    // 3. DURUM: Buton bırakıldı
    if (!currentState && isPressed) {
        isPressed = false;
        
        // Eğer uzun basma işlenmediyse ve debounce süresi aşıldıysa (gerçek bir basışsa)
        if (!longPressHandled && (currentTime - pressStartTime >= DEBOUNCE_DELAY)) {
            return BTN_SHORT_PRESS; // KISA BASIŞI FIRLAT!
        }
    }

    // Hiçbir olay yoksa
    return BTN_NONE;
}