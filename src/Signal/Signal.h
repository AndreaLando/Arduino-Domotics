#ifndef Signal_H
#define Signal_H

#pragma once
#include <Arduino.h>

class ToggleSignal {
public:
    ToggleSignal() : _oldStatus(0) {}

    long getOldValue() const {
        return _oldStatus;
    }

    bool change(int statusIn, long &value) {
        if (_oldStatus != statusIn) {

            _oldStatus = statusIn;

            // Toggle only on rising edge
            if (statusIn == 1) {
                value = (value == 0 ? 1 : 0);
                return true;
            }
        }

        return false;
    }

private:
    int _oldStatus;
};


enum TimeFMT {
    Microseconds,
    Milliseconds,
    Seconds,
    Minutes,
    Hours,
    Days,
    Months
};

//---------------------------------------------------------
// Base class for all timers
//---------------------------------------------------------
class TimerBase {
public:
    TimerBase() :
        _preset(0),
        _scale(1.0f),
        _startMicros(0),
        _running(false),
        _Q(false)
    {}

    TimerBase(float preset, TimeFMT fmt) :
        _startMicros(0),
        _running(false),
        _Q(false)
    {
        SetPreset(preset, fmt);
    }

    void SetPreset(float preset, TimeFMT fmt) {
        _preset = preset;
        _scale = formatToScale(fmt);
    }

    bool Q() const {
        return _Q;
    }

    float ET() const {
        if (!_running) return 0;
        return (micros() - _startMicros) / _scale;
    }

protected:
    float _preset;
    float _scale;
    unsigned long _startMicros;
    bool _running;
    bool _Q;

    float formatToScale(TimeFMT fmt) {
        switch (fmt) {
            case Microseconds: return 1.0f;
            case Milliseconds: return 1000.0f;
            case Seconds:      return 1000000.0f;
            case Minutes:      return 60000000.0f;
            case Hours:        return 3600000000.0f;
            case Days:         return 86400000000.0f;
            case Months:       return 2592000000000.0f;
        }
        return 1.0f;
    }
};

//---------------------------------------------------------
// TON — On‑Delay Timer
//---------------------------------------------------------
class TON : public TimerBase {
public:
    TON() : TimerBase() {}
    TON(float preset, TimeFMT fmt) : TimerBase(preset, fmt) {}

    void Run(bool IN) {
        if (IN) {
            if (!_running) {
                _running = true;
                _startMicros = micros();
            }
            if (!_Q && ET() >= _preset) {
                _Q = true;
                _running = false;
            }
        } else {
            _running = false;
            _Q = false;
        }
    }
};

//---------------------------------------------------------
// TOF — Off‑Delay Timer
//---------------------------------------------------------
class TOF : public TimerBase {
public:
    TOF() : TimerBase() {}
    TOF(float preset, TimeFMT fmt) : TimerBase(preset, fmt) {}

    void Run(bool IN) {
        if (IN) {
            _running = false;
            _Q = false;
        } else {
            if (!_running) {
                _running = true;
                _startMicros = micros();
            }
            if (!_Q && ET() >= _preset) {
                _Q = true;
                _running = false;
            }
        }
    }
};

//---------------------------------------------------------
// TP — Pulse Timer
//---------------------------------------------------------
class TP : public TimerBase {
public:
    TP() : TimerBase() {}
    TP(float preset, TimeFMT fmt) : TimerBase(preset, fmt) {}

    void Run(bool IN) {
        if (IN && !_running) {
            _running = true;
            _Q = true;
            _startMicros = micros();
        }

        if (_running && ET() >= _preset) {
            _Q = false;
            _running = false;
        }

        if (!IN && !_running) {
            _Q = false;
        }
    }
};

class Debounce {
public:
    TON ton;
    bool stable = false;

    Debounce(unsigned long ms = 20) : ton(ms, Milliseconds) {}

    bool Run(bool raw) {
        if (raw == stable) {
            ton.Run(false);   // no change → reset timer
        } else {
            ton.Run(true);    // change detected → start timer
            if (ton.Q()) {
                stable = raw; // accept new stable state
            }
        }
        return stable;
    }
};

class Edge {
public:
    bool last = false;

    bool Rising(bool in) {
        bool r = (!last && in);
        last = in;
        return r;
    }

    bool Falling(bool in) {
        bool f = (last && !in);
        last = in;
        return f;
    }

    bool Change(bool in) {
        bool c = (last != in);
        last = in;
        return c;
    }
};

#endif
