#include <Arduino.h>
#include "WeatherStation.h"

// Funzioni di lettura simulate (sostituisci con i tuoi sensori reali)
int readTemp()  { return analogRead(A0); }
int readWind()  { return analogRead(A1); }
int readRain()  { return analogRead(A2); }
int readLight() { return analogRead(A3); }

WeatherStation ws(readTemp, readWind, readRain, readLight);

void setup() {
  Serial.begin(115200);
  Serial.println("Weather demo avviato");

  ws.setEventCallback([](WeatherEvent e){
    Serial.print("Evento meteo: ");
    Serial.println((int)e);
  });

  ws.setAlarmCallback([](const WeatherAlarm *alarms, int count){
    Serial.print("Allarmi attivi: ");
    for (int i = 0; i < count; i++) {
      Serial.print((int)alarms[i]);
      Serial.print(" ");
    }
    Serial.println();
  });
}

void loop() {
  ws.update();

  Serial.print("Temp: ");
  Serial.print(ws.getTemperature());
  Serial.print("  Wind: ");
  Serial.print(ws.getWind());
  Serial.print("  Rain: ");
  Serial.print(ws.getRain());
  Serial.print("  Light: ");
  Serial.println(ws.getLight());

  delay(500);
}
