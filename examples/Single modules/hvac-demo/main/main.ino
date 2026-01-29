#include <Arduino.h>
#include "HVAC.h"

/* =====================================================
   CALLBACKS — HARDWARE INTEGRATION POINTS
   These functions are invoked automatically whenever
   the system changes the state of the compressor,
   circulator pump, or any zone's fancoil.
   Replace Serial prints with relay control in production.
   ===================================================== */

void onCompressor(bool state) {
    Serial.print("[COMPRESSOR] → ");
    Serial.println(state ? "ON" : "OFF");
}

void onCirculator(bool state) {
    Serial.print("[CIRCULATOR] → ");
    Serial.println(state ? "ON" : "OFF");
}

void onFancoil(String zoneName, int number, bool state) {
    Serial.print("[FANCOIL] Zone: ");
    Serial.print(zoneName);
    Serial.print("  #");
    Serial.print(number);
    Serial.print(" → ");
    Serial.println(state ? "ON" : "OFF");
}


/* =====================================================
   OBJECT CREATION
   Two independent thermal zones + heat pump controller.
   ===================================================== */

// Zone(name, setpoint, fancoil_number)
Zona livingRoom("Living Room", 21.0, 1);
Zona bedroom("Bedroom", 19.0, 2);

// Heat pump in AUTO mode with global setpoint 20°C
PompaDiCalore pdc(PompaDiCalore::AUTO, 20.0);


/* =====================================================
   SETUP
   ===================================================== */

void setup() {
    Serial.begin(115200);

    // Register zones inside the heat pump controller
    pdc.aggiungiZona(&livingRoom);
    pdc.aggiungiZona(&bedroom);

    // Register callbacks using the new setter methods
    pdc.setCallbackCompressore(onCompressor);
    pdc.setCallbackCircolatore(onCirculator);
    pdc.setCallbackFancoil(onFancoil);

    Serial.println("HVAC system initialized.");
}


/* =====================================================
   LOOP — MAIN APPLICATION LOGIC
   ===================================================== */

void loop() {

    /* ----------------------------------------------
       1. Simulated temperature readings
       Replace these with real sensors in production.
       ---------------------------------------------- */
    float indoorTemp  = 20.3;
    float outdoorTemp = 7.5;

    // Update internal and external temperatures
    pdc.aggiornaTemperaturaInterna(indoorTemp);
    pdc.aggiornaTemperaturaEsterna(outdoorTemp);


    /* ----------------------------------------------
       2. Update zone temperatures
       Each zone may have its own sensor.
       ---------------------------------------------- */
    livingRoom.aggiornaTemperatura(20.0);
    bedroom.aggiornaTemperatura(18.2);


    /* ----------------------------------------------
       3. Optional: window‑open detection
       If true for long enough, compressor is disabled.
       ---------------------------------------------- */
    bool windowOpen = false;
    pdc.setFinestraAperta(windowOpen);


    /* ----------------------------------------------
       4. Debug output
       Useful to understand system behavior.
       ---------------------------------------------- */
    Serial.print("Mode: ");
    Serial.println(pdc.getModalita());

    Serial.print("Compressor: ");
    Serial.println(pdc.isCompressoreAttivo() ? "ON" : "OFF");

    Serial.print("Fan speed: ");
    Serial.println(pdc.getVelocitaVentola());

    Serial.println("-----------------------------");

    delay(2000);
}
