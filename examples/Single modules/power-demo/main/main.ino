#include <Arduino.h>
#include "Power.h"

// Creazione del PowerManager
// Limite rete: 3000W
// Impianto FV nominale: 4000W
PowerManager pm(3000, 4000, 200, 200);

// Variabili simulate
float gridPower = 0;   // Prelievo da rete
float solarPower = 0;  // Produzione FV
float indoorTemp = 20; // Temperatura interna

void setup() {
  Serial.begin(115200);
  Serial.println("Power Manager demo avviato");

  // Callback: cambio stato carico
  pm.setOnLoadChange([](const String& name, bool state){
    Serial.print("[LOAD] ");
    Serial.print(name);
    Serial.print(" → ");
    Serial.println(state ? "ON" : "OFF");
  });

  // Callback: warning limite
  pm.setOnLimitWarning([](float net, float limit){
    Serial.print("[WARNING] Prelievo vicino al limite: ");
    Serial.print(net);
    Serial.print("W / ");
    Serial.print(limit);
    Serial.println("W");
  });

  // Callback: limite superato
  pm.setOnLimitExceeded([](float net, float limit){
    Serial.print("[LIMIT] Superato limite rete: ");
    Serial.print(net);
    Serial.print("W > ");
    Serial.println(limit);
  });

  // Callback: suggerimenti
  pm.setOnSuggestion([](const String& suggestion, int severity, const String& reason){
    Serial.print("[SUGGEST] ");
    Serial.print(suggestion);
    Serial.print(" (sev ");
    Serial.print(severity);
    Serial.print(") → ");
    Serial.println(reason);
  });

  // Aggiunta carichi
  pm.addLoad("Lavatrice", PowerManager::Priority::MEDIA, 1200, 30, 60);
  pm.addLoad("Asciugatrice", PowerManager::Priority::ALTA, 1500, 60, 120);
  pm.addLoad("Lavastoviglie", PowerManager::Priority::BASSA, 1000, 20, 40);

  // Carico termico (es. pompa di calore)
  pm.addThermalLoad("PompaCalore", true, 21.0, 19.0, 23.0, 60, 60);

  // Modalità ottimizzazione
  pm.setOptimizationMode(PowerManager::OptimizationMode::MASSIMO_AUTOCONSUMO);

  // Abilita auto-tuning
  pm.enableAutoTune(true);
}

void loop() {
  // Simulazione produzione FV (curva sinusoidale)
  solarPower = 2000 + 1500 * sin(millis() / 5000.0);

  // Simulazione prelievo rete (varia casualmente)
  gridPower = 1000 + random(-300, 300);

  // Simulazione temperatura interna
  indoorTemp = 20 + sin(millis() / 8000.0);

  // Aggiorna PowerManager
  pm.setGridPower(gridPower);
  pm.setSolarPower(solarPower);

  // Lux e temperatura esterna simulati
  float lux = 50000 + 20000 * sin(millis() / 7000.0);
  float tempExt = 15 + 5 * sin(millis() / 9000.0);
  pm.setEnvironmentalData(lux, tempExt);

  // Aggiorna logica carichi
  pm.updateLoads(1, 12, 30); // mese=1, ora=12:30

  // Aggiorna logica termica
  pm.updateThermalControl(indoorTemp, 1, 12, 30);

  // Auto-tuning periodico
  pm.autoTuneStep();

  // Stampa stato
  Serial.print("Grid: ");
  Serial.print(gridPower);
  Serial.print("W  Solar: ");
  Serial.print(solarPower);
  Serial.print("W  Net: ");
  Serial.print(pm.getNetGridPower());
  Serial.println("W");

  delay(1000);
}
