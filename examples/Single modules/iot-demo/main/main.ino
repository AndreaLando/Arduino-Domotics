#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "IOT.h"

// Configurazione rete (modifica secondo necessità)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress localIP(192,168,1,50);
IPAddress remoteIP(192,168,1,100); // App o server remoto

EthernetUDP Udp;

// Inizializzazione modulo IOT
// Porta locale: 6000
// Porta remota: 6001
IOT iot(Udp, remoteIP, 6000, 6001);

void setup() {
  Serial.begin(115200);
  Serial.println("IOT demo avviato");

  // Avvio Ethernet
  Ethernet.begin(mac, localIP);
  delay(1000);

  Serial.print("IP locale: ");
  Serial.println(Ethernet.localIP());

  // Avvio modulo IOT
  iot.begin("Sistema avviato");
}

void loop() {
  // Aggiorna IOT (ricezione comandi)
  bool received = iot.Update();

  if (received) {
    Serial.println("Comando UDP ricevuto!");
  }

  // Leggo lo stato dei comandi remoti
  auto &cmd = iot.GetStatus();

  if (cmd.onArrivoACasaChange.hasChanged()) {
    int stato = cmd.onArrivoACasaChange.get();
    Serial.print("[IOT] Stato casa cambiato → ");
    Serial.println(stato);
  }

  if (cmd.onLuciEsterneChange.hasChanged()) {
    bool value = cmd.onLuciEsterneChange.get();
    Serial.print("[IOT] Luci esterne → ");
    Serial.println(value ? "ON" : "OFF");
  }

  if (cmd.onProximity.hasChanged()) {
    bool value = cmd.onProximity.get();
    Serial.print("[IOT] Proximity → ");
    Serial.println(value ? "Arrivando" : "Partendo");
  }

  // Esempio: invio periodico temperatura media
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000) {
    float temp = 20.0 + sin(millis() / 3000.0);
    iot.setTemperaturaMediaInterna("Temperatura media aggiornata:", temp);
    lastSend = millis();
  }

  delay(50);
}
