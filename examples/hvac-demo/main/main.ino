#include <Arduino.h>
#include "HVAC.h"

// Creazione zone
Zona zonaGiorno("Zona Giorno", 21.0, 1);
Zona zonaNotte("Zona Notte", 20.0, 2);

// Pompa di calore in modalità AUTO
PompaDiCalore pdc(PompaDiCalore::AUTO, 21.0);

void setup() {
  Serial.begin(115200);
  Serial.println("HVAC demo avviato");

  // Aggiungo le zone
  pdc.aggiungiZona(&zonaGiorno);
  pdc.aggiungiZona(&zonaNotte);

  // Callback compressore
  pdc.setCallbackCompressore([](bool on){
    Serial.print("Compressore: ");
    Serial.println(on ? "ON" : "OFF");
  });

  // Callback fancoil
  pdc.setCallbackFancoil([](String nome, int id, bool stato){
    Serial.print("Fancoil ");
    Serial.print(nome);
    Serial.print(" (");
    Serial.print(id);
    Serial.print("): ");
    Serial.println(stato ? "ON" : "OFF");
  });

  // Callback circolatore
  pdc.setCallbackCircolatore([](bool on){
    Serial.print("Circolatore: ");
    Serial.println(on ? "ON" : "OFF");
  });
}

void loop() {
  // Simulazione temperature
  float tempInterna = 20.0 + sin(millis() / 5000.0);
  float tempEsterna = 10.0 + sin(millis() / 8000.0);

  zonaGiorno.aggiornaTemperatura(tempInterna);
  zonaNotte.aggiornaTemperatura(tempInterna - 0.5);

  pdc.aggiornaTemperaturaInterna(tempInterna);
  pdc.aggiornaTemperaturaEsterna(tempEsterna);

  delay(500);
}
