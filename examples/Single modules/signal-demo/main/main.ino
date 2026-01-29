#include <Arduino.h>
#include "Signal.h"

#include <Arduino.h>
#include "Signal.h"

// -----------------------------
// Pin di esempio
// -----------------------------
const int BTN = 2;       // pulsante
const int LED_TOGGLE = 3;
const int LED_TON = 4;
const int LED_TOF = 5;
const int LED_TP = 6;
const int LED_DEBOUNCE = 7;
const int LED_EDGE = 8;

// -----------------------------
// Oggetti delle tue classi
// -----------------------------
ToggleSignal toggle;
long toggleValue = 0;

TON ton(1000, Milliseconds);   // 1s ON-delay
TOF tof(1000, Milliseconds);   // 1s OFF-delay
TP  tp(500, Milliseconds);     // impulso di 0.5s

Debounce deb(30);              // debounce 30ms
Edge edge;                     // edge detector

void setup() {
    pinMode(BTN, INPUT_PULLUP);
    pinMode(LED_TOGGLE, OUTPUT);
    pinMode(LED_TON, OUTPUT);
    pinMode(LED_TOF, OUTPUT);
    pinMode(LED_TP, OUTPUT);
    pinMode(LED_DEBOUNCE, OUTPUT);
    pinMode(LED_EDGE, OUTPUT);

    Serial.begin(115200);
    Serial.println("Esempi classi Signal.h avviati");
}

void loop() {
    // Lettura pulsante (invertito perché INPUT_PULLUP)
    bool rawBtn = !digitalRead(BTN);

    // -----------------------------
    // 1) ToggleSignal
    // -----------------------------
    long old = toggleValue;
    toggle.change(rawBtn, toggleValue);
    digitalWrite(LED_TOGGLE, toggleValue);

    // -----------------------------
    // 2) TON — On Delay
    // -----------------------------
    ton.Run(rawBtn);
    digitalWrite(LED_TON, ton.Q());

    // -----------------------------
    // 3) TOF — Off Delay
    // -----------------------------
    tof.Run(rawBtn);
    digitalWrite(LED_TOF, tof.Q());

    // -----------------------------
    // 4) TP — Pulse Timer
    // -----------------------------
    tp.Run(rawBtn);
    digitalWrite(LED_TP, tp.Q());

    // -----------------------------
    // 5) Debounce
    // -----------------------------
    bool stableBtn = deb.Run(rawBtn);
    digitalWrite(LED_DEBOUNCE, stableBtn);

    // -----------------------------
    // 6) Edge detection
    // -----------------------------
    if (edge.Rising(stableBtn)) {
        Serial.println("Rising edge!");
        digitalWrite(LED_EDGE, HIGH);
    }
    if (edge.Falling(stableBtn)) {
        Serial.println("Falling edge!");
        digitalWrite(LED_EDGE, LOW);
    }

    delay(1);
}
