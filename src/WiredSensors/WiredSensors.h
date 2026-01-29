#ifndef WiredSensors_H
#define WiredSensors_H

#include "Signal.h"
#include <Arduino.h>
#include <vector>
#include <unordered_map>

/*
ALARM SYSTEM – ARCHITECTURE & USAGE GUIDE
=========================================

1. OVERVIEW
-----------
This system implements a modular, event‑driven alarm framework composed of:

- SensorChannel: represents a single physical or logical input
- Sensor: groups multiple channels into a single device
- ZoneManager: groups sensors into zones and manages alarm logic
- AlarmDispatcher: fires callbacks when alarms occur

The architecture supports:
- Debouncing
- On‑delay timers (TON)
- Latching (memory)
- Startup inhibit
- Enable/disable
- Engage/disengage
- Snooze/acknowledge
- Rising‑edge alarm detection
- Global, per‑type, per‑zone, and per‑zone+type callbacks


2. SENSORCHANNEL
----------------
A SensorChannel represents one monitored input.

Each channel includes:
- pin: hardware pin or -1 for simulated input
- timer: TON delay before alarm becomes active
- debounce: 20 ms default
- mem: latched alarm memory
- inhibit: suppresses alarm
- type: enum identifying the channel (RT, H24, MASK, etc.)


3. SENSOR
---------
A Sensor contains multiple channels and handles:

- Debouncing
- TON timing
- Latching
- Startup inhibit (2 seconds)
- Enable/disable
- Engage/disengage
- Per‑channel alarm queries

Key methods:
- Run(inputs): processes all channels
- Enable(mode): disables or enables the sensor
- Engage(mode): enables latching
- Reset(): clears timers and memory
- ChannelAlarm(type): returns alarm state for a specific channel


4. ZONEMANAGER
--------------
The ZoneManager groups sensors into named zones and provides:

Alarm queries:
- AnyAlarm()
- ZoneAlarm(zone)
- AnyAlarmByType(type)
- ZoneAlarmByType(zone, type)

Snooze logic per channel type:
- Snoozed alarms do not trigger callbacks
- Snooze resets automatically when the alarm clears

Rising‑edge detection:
- NewAlarmByType(type)

Zone‑level control:
- EnableZone(zone, mode)
- EngageZone(zone, mode)
- ResetZone(zone)


5. ALARMDISPATCHER
------------------
The dispatcher provides a flexible callback system.

Callback types supported:
1. Global: any alarm in any zone
2. By alarm type: e.g., all RT alarms
3. By zone: any alarm inside a specific zone
4. By zone + type: e.g., RT alarms in the “Perimeter” zone

Callback signature:
(zoneName, channelType, listOfSensors)

Registration:
- OnAnyAlarm(cb)
- OnAlarmType(type, cb)
- OnZoneAlarm(zone, cb)
- OnZoneAlarmType(zone, type, cb)

Dispatching:
The ZoneManager automatically dispatches events when:
- A new alarm appears (rising edge)
- The alarm is not snoozed
- At least one sensor in a zone is in alarm for that type


6. ALARM FLOW SUMMARY
---------------------
1. Inputs are read and debounced
2. TON timers evaluate
3. If engaged, alarms latch
4. ZoneManager checks alarm states
5. Rising‑edge detection identifies new alarms
6. Snooze logic filters out acknowledged alarms
7. Dispatcher fires callbacks:
   - Global
   - Type‑specific
   - Zone‑specific
   - Zone+type specific


7. TYPICAL USAGE PATTERN
------------------------
Setup:
- Create sensors
- Add sensors to ZoneManager
- Assign sensors to zones
- Register callbacks

Loop:
- Call sensor.Run(inputs) for each sensor
- Call manager.NewAlarmByType(type) for each channel type


8. EXAMPLE CALLBACK USE CASES
-----------------------------
- Trigger siren on any alarm
- Send notification for RT alarms only
- Log events per zone
- Trigger different actions for Entry vs Perimeter zones
- Implement UI indicators per alarm type


9. EXTENSIBILITY
----------------
The system is designed for easy expansion:

- Add new channel types
- Add new zones
- Add new callback categories
- Add logging, history, or persistence
- Add remote monitoring or network events

*/

const unsigned long RT_DELAY      = 250;   // 0.25 sec
const unsigned long INITIAL_DELAY = 500;   // 0.5 sec

enum class SensorChannelType {
    RT,
    H24,
    LEN,
    MASK
};

// -------------------------------------------------------------
//  SensorChannel
// -------------------------------------------------------------
class SensorChannel {
public:
    int pin;
    TON timer;
    Debounce debounce;
    Edge edge;
    bool mem = false;
    bool inhibit = false;
    SensorChannelType type;

    SensorChannel(int pin, float delayMs, SensorChannelType type)
        : pin(pin),
          type(type),
          timer(delayMs, Milliseconds),
          debounce(20)   // default debounce 20 ms
    {}
};


// -------------------------------------------------------------
//  Sensor
// -------------------------------------------------------------
class Sensor {
public:
    std::vector<SensorChannel> channels;
    std::unordered_map<SensorChannelType, SensorChannel*> lookup;

    bool _engage = false;
    bool _disabled = false;
    bool alarmOut = false;

    TON startupInhibit = TON(2, Seconds);  // ignore alarms for 2 seconds

    Sensor(std::initializer_list<SensorChannel> list)
        : channels(list)
    {
        for (auto& ch : channels)
            lookup[ch.type] = &ch;
    }

    SensorChannel* Get(SensorChannelType type) {
        auto it = lookup.find(type);
        return (it != lookup.end()) ? it->second : nullptr;
    }

    void Engage(bool mode) {
        _engage = mode;
    }

    void Enable(bool mode) {
        if (mode) {
            startupInhibit.Run(true);
        } else {
            for (auto& ch : channels) {
                ch.timer.Run(false);
                ch.mem = false;
            }
        }
        _disabled = !mode;
    }

    void Reset() {
        for (auto& ch : channels) {
            ch.timer.Run(false);
            ch.mem = false;
        }
    }

    // Check alarm for a specific channel type
    bool ChannelAlarm(SensorChannelType type) const {
        auto it = lookup.find(type);
        if (it == lookup.end())
            return false;

        const SensorChannel* ch = it->second;

        bool active = ch->timer.Q() && !ch->inhibit && startupInhibit.Q();
        return active || ch->mem;
    }

    // Main processing
    void Run(std::initializer_list<bool> inputs) {
        bool tempAlarm = false;
        auto in = inputs.begin();

        startupInhibit.Run(true);

        if (!_disabled) {
            for (auto& ch : channels) {

                bool raw = (ch.pin == -1 ? *in : digitalRead(ch.pin));
                ++in;

                bool debounced = ch.debounce.Run(raw);

                ch.timer.Run(debounced);

                if (ch.timer.Q() && !ch.inhibit && startupInhibit.Q()) {
                    tempAlarm = true;
                    if (_engage)
                        ch.mem = true;
                }

                if (!debounced)
                    ch.timer.Run(false);
            }
        } else {
            for (auto& ch : channels)
                ch.mem = false;
        }

        alarmOut = tempAlarm;
        for (auto& ch : channels)
            alarmOut |= ch.mem;
    }
};


// -------------------------------------------------------------
//  AlarmDispatcher
// -------------------------------------------------------------
class AlarmDispatcher {
public:
    using Callback = std::function<void(
        const std::string& zone,
        SensorChannelType type,
        const std::vector<Sensor*>& sensors
    )>;

    // Global callbacks (any zone, any type)
    std::vector<Callback> globalCallbacks;

    // Callbacks per alarm type
    std::unordered_map<SensorChannelType, std::vector<Callback>> typeCallbacks;

    // Callbacks per zone (any type)
    std::unordered_map<std::string, std::vector<Callback>> zoneCallbacks;

    // Callbacks per (zone + type)
    std::unordered_map<std::string,
        std::unordered_map<SensorChannelType, std::vector<Callback>>
    > zoneTypeCallbacks;


    // -------------------------------
    // Registration
    // -------------------------------
    void OnAnyAlarm(Callback cb) {
        globalCallbacks.push_back(cb);
    }

    void OnAlarmType(SensorChannelType type, Callback cb) {
        typeCallbacks[type].push_back(cb);
    }

    void OnZoneAlarm(const std::string& zone, Callback cb) {
        zoneCallbacks[zone].push_back(cb);
    }

    void OnZoneAlarmType(const std::string& zone, SensorChannelType type, Callback cb) {
        zoneTypeCallbacks[zone][type].push_back(cb);
    }


    // -------------------------------
    // Dispatch
    // -------------------------------
    void Dispatch(
        const std::string& zone,
        SensorChannelType type,
        const std::vector<Sensor*>& sensors
    ) {
        // Global callbacks
        for (auto& cb : globalCallbacks)
            cb(zone, type, sensors);

        // Type-specific callbacks
        auto itType = typeCallbacks.find(type);
        if (itType != typeCallbacks.end())
            for (auto& cb : itType->second)
                cb(zone, type, sensors);

        // Zone-specific callbacks
        auto itZone = zoneCallbacks.find(zone);
        if (itZone != zoneCallbacks.end())
            for (auto& cb : itZone->second)
                cb(zone, type, sensors);

        // Zone + Type callbacks
        auto itZT = zoneTypeCallbacks.find(zone);
        if (itZT != zoneTypeCallbacks.end()) {
            auto itZT2 = itZT->second.find(type);
            if (itZT2 != itZT->second.end())
                for (auto& cb : itZT2->second)
                    cb(zone, type, sensors);
        }
    }
};


// -------------------------------------------------------------
//  ZoneManager
// -------------------------------------------------------------
class ZoneManager {
public:
    using ZoneName = std::string;

    std::vector<Sensor*> sensors;
    std::unordered_map<ZoneName, std::vector<Sensor*>> zones;

    // Track last alarm state per channel type
    std::unordered_map<SensorChannelType, bool> lastState;

    // Track snoozed alarms per channel type
    std::unordered_map<SensorChannelType, bool> snoozed;

    AlarmDispatcher dispatcher;


    // -------------------------------
    // Registration
    // -------------------------------
    void AddSensor(Sensor* sensor) {
        sensors.push_back(sensor);
    }

    void AddToZone(const ZoneName& zone, Sensor* sensor) {
        zones[zone].push_back(sensor);
    }

    const std::vector<Sensor*>& GetZone(const ZoneName& zone) const {
        static const std::vector<Sensor*> empty;
        auto it = zones.find(zone);
        return (it != zones.end()) ? it->second : empty;
    }


    // -------------------------------
    // Alarm Queries
    // -------------------------------
    bool ZoneAlarm(const ZoneName& zone) const {
        for (auto* s : GetZone(zone))
            if (s->alarmOut)
                return true;
        return false;
    }

    bool AnyAlarm() const {
        for (auto* s : sensors)
            if (s->alarmOut)
                return true;
        return false;
    }

    bool ZoneAlarmByType(const ZoneName& zone, SensorChannelType type) const {
        for (auto* s : GetZone(zone))
            if (s->ChannelAlarm(type))
                return true;
        return false;
    }

    bool AnyAlarmByType(SensorChannelType type) const {
        for (auto* s : sensors)
            if (s->ChannelAlarm(type))
                return true;
        return false;
    }

    std::vector<Sensor*> SensorsInAlarm(SensorChannelType type) const {
        std::vector<Sensor*> out;
        for (auto* s : sensors)
            if (s->ChannelAlarm(type))
                out.push_back(s);
        return out;
    }


    // -------------------------------
    // New Alarm Detection + Snooze
    // -------------------------------
    bool NewAlarmByType(SensorChannelType type) {
        bool current = AnyAlarmByType(type);
        bool previous = lastState[type];
        bool isSnoozed = snoozed[type];

        if (!current)
            snoozed[type] = false;

        bool newAlarm = current && !previous && !isSnoozed;
        lastState[type] = current;

        if (!newAlarm)
            return false;

        // Dispatch per-zone events
        for (auto& [zoneName, zoneSensors] : zones) {
            std::vector<Sensor*> active;

            for (auto* s : zoneSensors)
                if (s->ChannelAlarm(type))
                    active.push_back(s);

            if (!active.empty())
                dispatcher.Dispatch(zoneName, type, active);
        }

        return true;
    }

    void Snooze(SensorChannelType type) {
        snoozed[type] = true;
    }


    // -------------------------------
    // Reset
    // -------------------------------
    void ResetZone(const ZoneName& zone) {
        for (auto* s : GetZone(zone))
            s->Reset();
    }

    void ResetAll() {
        for (auto* s : sensors)
            s->Reset();
    }


    // -------------------------------
    // Enable / Disable
    // -------------------------------
    void EnableZone(const ZoneName& zone, bool mode) {
        for (auto* s : GetZone(zone))
            s->Enable(mode);
    }

    void EnableAll(bool mode) {
        for (auto* s : sensors)
            s->Enable(mode);
    }


    // -------------------------------
    // Engage / Disengage
    // -------------------------------
    void EngageZone(const ZoneName& zone, bool mode) {
        for (auto* s : GetZone(zone))
            s->Engage(mode);
    }

    void EngageAll(bool mode) {
        for (auto* s : sensors)
            s->Engage(mode);
    }
};




#endif
