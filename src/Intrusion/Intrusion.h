#ifndef Intrusion_H
#define Intrusion_H

#include "Delay.h"
#include "Arduino.h"
#include <vector>
#include <unordered_map>

// ============================================================================
//  Sensor System - Unified Multi‑Channel Sensor Handler
// ============================================================================
//
//  Overview
//  --------
//  This module implements a generic, extensible sensor system capable of
//  handling any number of input channels (RT, H24, LEN, MASK, etc.).
//  Each channel includes:
//      - a digital input pin (or software input if pin = -1)
//      - a delay timer (DelayTimer)
//      - a memory latch (latched alarm state)
//      - a SensorChannelType enum identifier
//
//  The Sensor class manages:
//      - enabling/disabling the entire sensor
//      - engage mode (controls whether memory latching is active)
//      - per‑channel delay processing
//      - per‑channel memory latching
//      - combined alarm output
//      - O(1) channel lookup via enum
//
//  This design replaces multiple specialized classes (1‑way, 2‑way, 3‑way,
//  external sensors) with a single flexible implementation.
//
//
//  Key Concepts
//  ------------
//
//  • Channels
//      Each channel is represented by a SensorChannel object containing:
//          - pin number (or -1 for software input)
//          - delay timer
//          - memory latch
//          - enum type (RT, H24, LEN, MASK)
//
//  • Engage Mode
//      When Engage(true) is active, channels latch their alarm state
//      after their delay expires. Latches remain active until Reset().
//
//  • Enable/Disable
//      Enable(false) clears all timers and memory latches and prevents
//      further processing. Enable(true) reactivates the sensor.
//
//  • Run()
//      Processes all channels once per loop iteration. Inputs may come
//      from hardware pins or software booleans.
//
//  • Lookup Map
//      The class provides O(1) access to channels using:
//          Get(SensorChannelType::RT)
//      This avoids manual iteration and simplifies logic.
//
//
//  Public API
//  ----------
//
//      Sensor(std::initializer_list<SensorChannel> list)
//          Constructs a sensor with the specified channels.
//
//      void Engage(bool mode)
//          Enables or disables memory latching.
//
//      void Enable(bool mode)
//          Enables or disables the sensor. Disabling clears timers/latches.
//
//      void Reset()
//          Clears all delay timers and memory latches.
//
//      void Run(std::initializer_list<bool> inputs)
//          Executes delay logic for each channel. If pins are used,
//          pass an empty list: Run({}).
//
//      SensorChannel* Get(SensorChannelType type)
//          Returns a pointer to the channel with the given enum type,
//          or nullptr if the channel does not exist.
//
//      bool alarmOut
//          True if any channel is currently active or latched.
//
//
//  Usage Examples
//  --------------
//
//  • Creating a 3‑way sensor (RT + H24 + MASK):
//
//      Sensor sensor3Way({
//          SensorChannel(5, RT_DELAY,      SensorChannelType::RT),
//          SensorChannel(6, INITIAL_DELAY, SensorChannelType::H24),
//          SensorChannel(7, INITIAL_DELAY, SensorChannelType::MASK)
//      });
//
//  • Running the sensor using hardware pins:
//
//      sensor3Way.Run({});
//      if (sensor3Way.alarmOut) { ... }
//
//  • Running the sensor using software inputs:
//
//      sensor3Way.Run({ rtValue, h24Value, maskValue });
//
//  • Accessing a channel:
//
//      auto* rt = sensor3Way.Get(SensorChannelType::RT);
//      if (rt && rt->mem) { ... }
//
//  • Clearing a latch:
//
//      sensor3Way.Get(SensorChannelType::MASK)->mem = false;
//
//
//  Notes
//  -----
//  - Channels are processed in the order they are declared.
//  - The number of inputs passed to Run() must match the number of channels
//    only when using software inputs (pin = -1).
//  - Memory latching only occurs when Engage(true) is active.
//  - alarmOut combines both temporary and latched alarms.
//
// ============================================================================
//  End of Documentation Block
// ============================================================================

const unsigned long RT_DELAY      = 250;   // 0.25 sec
const unsigned long INITIAL_DELAY = 500;   // 0.5 sec

enum class SensorChannelType {
    RT,
    H24,
    LEN,
    MASK
};

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
          debounce(20)   // 20 ms debounce default
    {}
};


class Sensor {
public:
    std::vector<SensorChannel> channels;
    std::unordered_map<SensorChannelType, SensorChannel*> lookup;

    bool _engage = false;
    bool _disabled = false;
    bool alarmOut = false;

    TON startupInhibit = TON(2, Seconds);  // ignore alarms for 2 seconds after enabling

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
            startupInhibit.Run(true);  // start inhibit timer
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



#endif
