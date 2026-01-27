#include <Arduino.h>
#include "Domo.h"
#include "Initialization.h"

// Esempio minimale: inizializza il sistema senza logiche aggiuntive

void setup() {
  Serial.begin(115200);
  Serial.println("Minimal example avviato");

  // Avvia il DomoManager senza callback personalizzate
  Manager.Begin(nullptr, nullptr, nullptr);
}

void loop() {
  // Il DomoManager gestisce tutto internamente
}
