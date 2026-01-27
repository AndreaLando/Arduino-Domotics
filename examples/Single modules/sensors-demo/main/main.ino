#include <Arduino.h>
#include "Intrusion.h"

// Sensore multi‑canale:
// - RT  → allarme immediato con ritardo breve
// - H24 → memoria 24h
// - MASK → mascheramento
// - LEN → logica estesa (esempio)
Sensor sensore({
    SensorChannel(-1, RT_DELAY,      SensorChannelType::RT),
    SensorChannel(-1, INITIAL_DELAY, SensorChannelType::H24),
    SensorChannel(-1, INITIAL_DELAY, SensorChannelType::MASK),
    SensorChannel(-1, RT_DELAY,      SensorChannelType::LEN)
});

// Variabili simulate per gli ingressi
bool rtInput   = false;
bool h24Input  = false;
bool maskInput = false;
bool lenInput  = false;

void stampaStati() {
  Serial.print("RT mem=");
  Serial.print(sensore.Get(SensorChannelType::RT)->mem);

  Serial.print(" | H24 mem=");
  Serial.print(sensore.Get(SensorChannelType::H24)->mem);

  Serial.print(" | MASK mem=");
  Serial.print(sensore.Get(SensorChannelType::MASK)->mem);

  Serial.print(" | LEN mem=");
  Serial.print(sensore.Get(SensorChannelType::LEN)->mem);

  Serial.print(" | alarmOut=");
  Serial.println(sensore.alarmOut);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Sensors demo avviato");

  // Attiva modalità ENGAGE → abilita latch memoria
  sensore.Engage(true);

  // Abilita sensore (attiva startup inhibit)
  sensore.Enable(true);
}

void loop() {
  // Simulazione ingressi digitali
  // (puoi sostituire con digitalRead(pin))
  rtInput   = (millis() / 2000) % 2;   // alterna ogni 2s
  h24Input  = (millis() / 5000) % 2;   // alterna ogni 5s
  maskInput = false;                   // sempre OFF
  lenInput  = (millis() / 3000) % 2;   // alterna ogni 3s

  // Esecuzione sensore
  sensore.Run({ rtInput, h24Input, maskInput, lenInput });

  // Stampa stato
  stampaStati();

  // Esempio: reset memoria ogni 15 secondi
  if ((millis() / 15000) % 2 == 1) {
    sensore.Reset();
    Serial.println(">>> RESET MEMORIA");
  }

  delay(500);
}
