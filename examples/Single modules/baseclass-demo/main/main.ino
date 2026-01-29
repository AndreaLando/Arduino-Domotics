/*==============================================================================
    ARDUINO EXAMPLE — USING MEASUREMENT MANAGEMENT & TREND ANALYSIS MODULE
--------------------------------------------------------------------------------

    This example demonstrates how to use the following components:

      • Cell<T>
          Tracks value changes and avoids unnecessary processing when a
          measurement remains the same.

      • Gruppo
          Maintains circular buffers for measurements and variations, computes
          moving averages, and determines trends (constant, increasing,
          decreasing).

      • CalcolatoreMedie
          Manages multiple measurement groups, automatically creating them
          when needed and providing aggregated statistics.

    In this demo:
      - A simulated temperature value is read from an analog pin.
      - Cell<float> detects when the value changes.
      - The measurement is added to a "Temperature" group.
      - The sketch prints the moving average and trend to the Serial Monitor.

    Replace the simulated sensor logic with your actual sensor code to use
    this module in real applications.

==============================================================================*/

#include <Arduino.h>
#include "BaseClass.h"   // Include your module

CalcolatoreMedie calc;   // Main manager for groups

// Example: using Cell<T> to track changes in a sensor reading
Cell<float> lastTemperature;

// Name of the measurement group
const String GROUP_NAME = "Temperature";

// Simulated analog pin (replace with a real sensor pin)
const int SENSOR_PIN = A0;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("=== Measurement Manager Demo ===");

    // Create a group with:
    // - buffer size = 10 samples
    // - trend sensitivity = 0.2
    calc.creaGruppo(GROUP_NAME, 10, 0.2f);

    Serial.println("Group 'Temperature' created.");
}

void loop() {
    // ---------------------------------------------------------------------
    // 1. Read a sensor value (simulated here using analogRead)
    // ---------------------------------------------------------------------
    int raw = analogRead(SENSOR_PIN);

    // Convert raw ADC to a temperature-like float (example only)
    float temperature = map(raw, 0, 1023, 200, 300) / 10.0;  
    // Produces values between 20.0 and 30.0

    // ---------------------------------------------------------------------
    // 2. Use Cell<T> to detect if the value changed
    // ---------------------------------------------------------------------
    lastTemperature.setIfDiff(temperature);

    if (lastTemperature.hasChanged()) {
        Serial.print("New temperature detected: ");
        Serial.println(lastTemperature.preserveGet());
    }

    // ---------------------------------------------------------------------
    // 3. Add measurement to the group
    // ---------------------------------------------------------------------
    calc.aggiungiMisura(GROUP_NAME, temperature);

    // ---------------------------------------------------------------------
    // 4. Read the moving average
    // ---------------------------------------------------------------------
    float avg = calc.mediaGruppo(GROUP_NAME);

    Serial.print("Average temperature: ");
    Serial.println(avg);

    // ---------------------------------------------------------------------
    // 5. Check the trend
    // ---------------------------------------------------------------------
    Gruppo::Trend t = calc.trendGruppo(GROUP_NAME);

    Serial.print("Trend: ");
    switch (t) {
        case Gruppo::COSTANTE:
            Serial.println("Constant");
            break;
        case Gruppo::CRESCENTE:
            Serial.println("Increasing");
            break;
        case Gruppo::DECRESCENTE:
            Serial.println("Decreasing");
            break;
    }

    Serial.println("-----------------------------");

    delay(1000);  // Wait 1 second before next reading
}
