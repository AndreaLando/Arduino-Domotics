#include <Arduino.h>
#include "Domo.h"
#include "Initialization.h"

// Callback: qualcosa nel buffer è cambiato
void somethingChanged(ModbusBuffer &buf) {
  Serial.println("ModbusBuffer: variazione rilevata");
}

// Callback: routing di un valore da un'area all'altra
void route(BufferSourceInfo info, int area, ModbusBuffer &buf) {
  Serial.print("Routing area ");
  Serial.print(area);
  Serial.print(" → valore: ");
  Serial.println(info.value);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Modbus demo avviato");

  // Avvio completo del sistema
  Manager.Begin(somethingChanged, route, nullptr);
}

void loop() {
  // Il DomoManager gestisce polling, routing e pannello
}
