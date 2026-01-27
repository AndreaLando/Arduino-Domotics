#include <Arduino.h>
#include "Delay.h"

// === TON: attiva solo se il segnale resta ON per X ms ===
TON tonDemo(1000); // 1 secondo

// === TOFF: resta ON per X ms dopo che il segnale torna OFF ===
TOFF toffDemo(1500); // 1.5 secondi

// === Edge detection ===
Edge edgeDemo;

// === Delay non bloccante ===
unsigned long lastBlink = 0;

// === Timer ciclico ===
CycleTimer cycleDemo(2000); // ogni 2 secondi

// === One-shot timer ===
OneShot oneShotDemo(3000); // 3 secondi

// Simulazione input
bool simulatedInput = false;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Delay / Timing Demo Avviato ===");

  // Avvio one-shot
  oneShotDemo.trigger();
}

void loop() {

  // Simulazione input che cambia ogni 3 secondi
  simulatedInput = (millis() / 3000) % 2;

  // --- TON DEMO ---
  if (tonDemo.update(simulatedInput)) {
    Serial.println("[TON] Attivato dopo 1 secondo di segnale ON");
  }

  // --- TOFF DEMO ---
  if (toffDemo.update(simulatedInput)) {
    Serial.println("[TOFF] Mantiene ON anche dopo che il segnale è tornato OFF");
  }

  // --- EDGE DETECTION ---
  if (edgeDemo.rising(simulatedInput)) {
    Serial.println("[EDGE] Rising edge rilevato");
  }
  if (edgeDemo.falling(simulatedInput)) {
    Serial.println("[EDGE] Falling edge rilevato");
  }

  // --- DELAY NON BLOCCANTE ---
  if (millis() - lastBlink >= 1000) {
    lastBlink = millis();
    Serial.println("[DELAY] 1 secondo trascorso (non bloccante)");
  }

  // --- TIMER CICLICO ---
  if (cycleDemo.tick()) {
    Serial.println("[CYCLE] Tick ogni 2 secondi");
  }

  // --- ONE SHOT ---
  if (oneShotDemo.done()) {
    Serial.println("[ONESHOT] Timer completato dopo 3 secondi");
    oneShotDemo.trigger(); // lo rilancio per mostrare il comportamento
  }

  delay(20); // solo per rendere più leggibile il log
}
