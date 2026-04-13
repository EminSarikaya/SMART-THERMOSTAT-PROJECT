#pragma once
#include <Arduino.h>

// ESP-NOW üzerinden taşınacak veri paketi (Payload)
typedef struct {
    uint8_t messageType;        // 0: Heartbeat (Kalp Atışı), 1: Control Command (Kontrol Komutu)
    float currentTemperature;   // BME280'den okunan güncel sıcaklık
    float targetTemperature;    // Kullanıcının belirlediği hedef sıcaklık
    bool isRelayActive;         // Rölenin olması gereken durumu (true = ON, false = OFF)
} DataPacket;