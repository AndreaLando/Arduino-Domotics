#include <Arduino.h>
#include "Weather.h"

/* ============================================================
   EXAMPLE USAGE OF THE WeatherStation CLASS
   ------------------------------------------------------------
   This sketch demonstrates:
   - how to provide sensor-reading functions
   - how to set event and alarm callbacks
   - how to call update() inside the loop
   ============================================================ */


/* ============================================================
   MOCK SENSOR FUNCTIONS
   ------------------------------------------------------------
   In a real project, you would read actual ADC values here:
       analogRead(A0), analogRead(A1), etc.
   For this example we use simulated values.
   ============================================================ */

int readTempMock() {
    // Simulate an ADC value between 0 and 1023
    return random(400, 600);
}

int readWindMock() {
    return random(200, 800);
}

int readRainMock() {
    return random(100, 900);
}

int readLightMock() {
    return random(100, 900);
}

/* ============================================================
   EVENT CALLBACK (edge-triggered)
   ============================================================ */
void onWeatherEvent(WeatherEvent ev) {
    switch (ev) {
        case WeatherEvent::RainStart:
            Serial.println("[EVENT] Rain started");
            break;
        case WeatherEvent::RainStop:
            Serial.println("[EVENT] Rain stopped");
            break;
        case WeatherEvent::WindGustStart:
            Serial.println("[EVENT] Wind gust STARTED");
            break;
        case WeatherEvent::WindGustEnd:
            Serial.println("[EVENT] Wind gust ENDED");
            break;
        case WeatherEvent::DayStart:
            Serial.println("[EVENT] Daytime started");
            break;
        case WeatherEvent::NightStart:
            Serial.println("[EVENT] Nighttime started");
            break;
    }
}

/* ============================================================
   ALARM CALLBACK (edge-triggered)
   ------------------------------------------------------------
   Parameters:
   - array of active alarms
   - number of alarms
   ============================================================ */
void onWeatherAlarm(const WeatherAlarm *alarms, int count) {
    Serial.print("[ALARM] Active alarms: ");

    for (int i = 0; i < count; i++) {
        switch (alarms[i]) {
            case WeatherAlarm::TempLow:
                Serial.print("TempLow ");
                break;
            case WeatherAlarm::TempHigh:
                Serial.print("TempHigh ");
                break;
            case WeatherAlarm::WindHigh:
                Serial.print("WindHigh ");
                break;
            case WeatherAlarm::RainHigh:
                Serial.print("RainHigh ");
                break;
        }
    }
    Serial.println();
}

/* ============================================================
   WeatherStation INSTANCE
   ------------------------------------------------------------
   We pass the sensor-reading functions and scaling factors.
   ============================================================ */

WeatherStation station(
    readTempMock,
    readWindMock,
    readRainMock,
    readLightMock,
    100.0,   // tempFactor
    6.0,     // windFactor
    20.0,    // rainFactor
    100.0,   // lightFactor
    0.0,     // lowTempThreshold
    40.0,    // highTempThreshold
    20.0,    // highWindThreshold
    70.0,    // highRainThreshold
    300.0,   // day threshold
    200.0    // night threshold
);

/* ============================================================
   SETUP
   ============================================================ */
void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("=== WeatherStation Demo ===");

    // Register callbacks
    station.setEventCallback(onWeatherEvent);
    station.setAlarmCallback(onWeatherAlarm);

    // Set alarm debounce (optional)
    station.setAlarmDebounce(3);
}

/* ============================================================
   MAIN LOOP
   ------------------------------------------------------------
   - Calls update() every cycle
   - Prints filtered values (moving average)
   ============================================================ */
void loop() {
    station.update();

    // Print filtered sensor values
    Serial.print("Temp: ");
    Serial.print(station.getTemperature());
    Serial.print(" °C | Wind: ");
    Serial.print(station.getWind());
    Serial.print(" km/h | Rain: ");
    Serial.print(station.getRain());
    Serial.print(" mm | Light: ");
    Serial.println(station.getLight());

    delay(500);
}

