/*
===============================================================
                    ARDUINO OPTA CONTROL EXAMPLE
===============================================================
This example demonstrates how to use the Heater and Valve
classes on the Arduino Opta platform **without any analog or
PWM output**.

Instead of driving a physical heater pin, the PID output is
stored in a memory variable (`heaterOutput`). This allows the
system to run entirely in software and makes it compatible with
the Opta, which does not support analogWrite() on digital pins.

Typical uses for the memory-based output:
 • Send heater power to Modbus registers
 • Publish via MQTT
 • Log to serial or SD card
 • Drive external hardware through a different interface

The sketch reads a temperature sensor, computes heater power,
stores it in memory, and automatically opens/closes a valve
based on temperature thresholds.
===============================================================
*/

#include "Equipment.h"

// -------------------------------
// Hardware pins (digital only)
// -------------------------------
const int VALVE_SWITCH_PIN = 7;   // Digital input for valve switch
const int TEMP_SENSOR_PIN  = A1;  // Example analog temperature sensor

// -------------------------------
// Create objects
// -------------------------------
Heater heater(2.0, 0.5, 1.0, 255);   // PID gains + max power
Valve valve(3, 40000);              // address=3, timeout=40s

// -------------------------------
// Memory-based heater output
// -------------------------------
int heaterOutput = 0;   // 0–255, stored only in RAM


void setup() {
    Serial.begin(115200);

    pinMode(VALVE_SWITCH_PIN, INPUT_PULLUP);

    // -------------------------------
    // Heater configuration
    // -------------------------------
    heater.setMaxTemp(120);
    heater.setMinTemp(-10);
    heater.setStuckDetection(0.2, 10);

    heater.setSoftStartDuration(10);
    heater.enableAdaptiveSoftStart(true);
    heater.setAmbientTemperature(22.0);
    heater.setAdaptiveRange(20.0);

    heater.setTarget(60.0);   // Start heating
}


void loop() {

    // -------------------------------
    // Read temperature sensor
    // -------------------------------
    float raw = analogRead(TEMP_SENSOR_PIN);
    float temperature = (raw / 1023.0f) * 100.0f;  // Example conversion

    // -------------------------------
    // Update heater (memory only)
    // -------------------------------
    heaterOutput = heater.update(temperature);

    // No analogWrite, no PWM, no DAC — memory only

    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print("  HeaterOut: ");
    Serial.print(heaterOutput);
    Serial.print("  State: ");
    Serial.println(heater.state());

    // -------------------------------
    // Valve logic
    // -------------------------------
    int switchStatus = digitalRead(VALVE_SWITCH_PIN) == HIGH ? 1 : 0;
    valve.update(switchStatus);

    // Open valve above 50°C
    if (temperature > 50 && valve.state() != Valve::Opened) {
        valve.commandOpen();
    }

    // Close valve below 45°C
    if (temperature < 45 && valve.state() != Valve::Closed) {
        valve.commandClose();
    }

    // Handle valve errors
    if (valve.state() == Valve::Error) {
        Serial.println("VALVE ERROR — stopping movement");
        valve.commandStop();
    }

    delay(100);
}
