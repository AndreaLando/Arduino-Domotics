#include <Arduino.h>
#include "WiredSensors.h"

// =============================================================
//  Example Arduino Main File Demonstrating Full Alarm System
// =============================================================

// --- Include your classes here ---
// SensorChannel
// Sensor
// AlarmDispatcher
// ZoneManager
// (Paste the full class implementations above this file)

// -------------------------------------------------------------
//  Create Sensors
// -------------------------------------------------------------

Sensor doorSensor({
    SensorChannel(2, 250, SensorChannelType::RT),
    SensorChannel(3, 500, SensorChannelType::H24)
});

Sensor windowSensor({
    SensorChannel(4, 250, SensorChannelType::RT)
});

Sensor maskSensor({
    SensorChannel(5, 250, SensorChannelType::MASK)
});

// -------------------------------------------------------------
//  Create Zone Manager
// -------------------------------------------------------------

ZoneManager manager;

// -------------------------------------------------------------
//  Callback Functions
// -------------------------------------------------------------

void GlobalAlarmCallback(const std::string& zone, SensorChannelType type, const std::vector<Sensor*>& list) {
    Serial.print("[GLOBAL] Alarm in zone: ");
    Serial.print(zone.c_str());
    Serial.print("  Type: ");
    Serial.println((int)type);
}

void RTAlarmCallback(const std::string& zone, SensorChannelType type, const std::vector<Sensor*>& list) {
    Serial.print("[TYPE] RT Alarm in zone: ");
    Serial.println(zone.c_str());
}

void PerimeterAlarmCallback(const std::string& zone, SensorChannelType type, const std::vector<Sensor*>& list) {
    Serial.print("[ZONE] Perimeter alarm type ");
    Serial.println((int)type);
}

void EntryRTAlarmCallback(const std::string& zone, SensorChannelType type, const std::vector<Sensor*>& list) {
    Serial.println("[ZONE+TYPE] Entry zone RT alarm!");
}

// -------------------------------------------------------------
//  Setup
// -------------------------------------------------------------

void setup() {
    Serial.begin(115200);

    // Register sensors
    manager.AddSensor(&doorSensor);
    manager.AddSensor(&windowSensor);
    manager.AddSensor(&maskSensor);

    // Assign zones
    manager.AddToZone("Perimeter", &doorSensor);
    manager.AddToZone("Perimeter", &windowSensor);
    manager.AddToZone("Entry", &doorSensor);
    manager.AddToZone("Masking", &maskSensor);

    // Enable all sensors
    manager.EnableAll(true);

    // Engage all sensors (latching mode)
    manager.EngageAll(true);

    // ---------------------------------------------------------
    // Register Callbacks
    // ---------------------------------------------------------

    manager.dispatcher.OnAnyAlarm(GlobalAlarmCallback);

    manager.dispatcher.OnAlarmType(SensorChannelType::RT, RTAlarmCallback);

    manager.dispatcher.OnZoneAlarm("Perimeter", PerimeterAlarmCallback);

    manager.dispatcher.OnZoneAlarmType("Entry", SensorChannelType::RT, EntryRTAlarmCallback);

    Serial.println("System initialized.");
}

// -------------------------------------------------------------
//  Loop
// -------------------------------------------------------------

void loop() {

    // ---------------------------------------------------------
    // Simulated Inputs (replace with real digitalRead)
    // ---------------------------------------------------------

    bool doorRT     = digitalRead(2);
    bool doorH24    = digitalRead(3);
    bool windowRT   = digitalRead(4);
    bool maskInput  = digitalRead(5);

    // ---------------------------------------------------------
    // Run sensors
    // ---------------------------------------------------------

    doorSensor.Run({doorRT, doorH24});
    windowSensor.Run({windowRT});
    maskSensor.Run({maskInput});

    // ---------------------------------------------------------
    // Rising-edge alarm detection
    // ---------------------------------------------------------

    manager.NewAlarmByType(SensorChannelType::RT);
    manager.NewAlarmByType(SensorChannelType::H24);
    manager.NewAlarmByType(SensorChannelType::MASK);

    // ---------------------------------------------------------
    // Example Snooze Logic
    // ---------------------------------------------------------

    if (Serial.available()) {
        char c = Serial.read();
        if (c == 's') {
            Serial.println("SNOOZE RT alarms");
            manager.Snooze(SensorChannelType::RT);
        }
    }

    delay(50);
}
