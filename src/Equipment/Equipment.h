#ifndef Equipment_H
#define Equipment_H


#include "Signal.h"

/*
===============================================================
                     HEATER CLASS — USAGE GUIDE
===============================================================

This class controls a heater using a PID algorithm with:
 - Temperature safety limits
 - Stuck sensor detection
 - Soft-start heating
 - Optional adaptive soft-start based on ambient temperature

---------------------------------------------------------------
1. CREATE THE HEATER OBJECT
---------------------------------------------------------------

Heater heater(Kp, Ki, Kd, maxPower);

Example:
    Heater heater(2.0, 0.5, 1.0, 255);

Where:
 - Kp, Ki, Kd = PID gains
 - maxPower   = maximum output (0–255 for PWM)

---------------------------------------------------------------
2. CONFIGURE SAFETY LIMITS (optional)
---------------------------------------------------------------

    heater.setMaxTemp(120);   // °C — shutdown if exceeded
    heater.setMinTemp(-10);   // °C — shutdown if too low

Stuck sensor detection:
    heater.setStuckDetection(0.2, 10);
        → If temperature changes less than 0.2°C for 10 seconds,
          the heater enters ERROR state.

---------------------------------------------------------------
3. CONFIGURE SOFT-START (optional)
---------------------------------------------------------------

Fixed soft-start:
    heater.setSoftStartDuration(10);  // seconds to ramp to full power

Adaptive soft-start:
    heater.enableAdaptiveSoftStart(true);  // enable/disable
    heater.setAmbientTemperature(22.0);    // °C
    heater.setAdaptiveRange(20.0);         // °C range for adaptation

Adaptive behavior:
 - If ambient is far below target → faster ramp
 - If ambient is close to target → slower ramp

---------------------------------------------------------------
4. SET THE TARGET TEMPERATURE
---------------------------------------------------------------

    heater.setTarget(60.0);   // °C

This starts the heating process and activates soft-start.

---------------------------------------------------------------
5. CALL update() FREQUENTLY (non-blocking)
---------------------------------------------------------------

Inside loop():

    float temp = readTemperatureSensor();
    int power = heater.update(temp);

    analogWrite(HEATER_PIN, power);

update() returns:
 - 0–maxPower → heater output
 - 0 if Off or Error

---------------------------------------------------------------
6. CHECK HEATER STATE
---------------------------------------------------------------

    if (heater.state() == Heater::Error) {
        // Shut down hardware, alert user, etc.
    }

    if (heater.state() == Heater::Heating) {
        // Normal operation
    }

---------------------------------------------------------------
7. STOP OR RESET
---------------------------------------------------------------

Stop heating manually:
    heater.stop();

Reset error state:
    heater.resetError();

---------------------------------------------------------------
8. TYPICAL LOOP EXAMPLE
---------------------------------------------------------------

void loop() {
    float temp = readTemperatureSensor();
    int power = heater.update(temp);
    analogWrite(HEATER_PIN, power);
}

===============================================================
*/

class Heater {
public:
    enum State {
        Off,
        Heating,
        Error
    };

    Heater(float kp, float ki, float kd, float maxPower = 255)
        : _kp(kp), _ki(ki), _kd(kd),
          _maxPower(maxPower),
          _state(Off),
          _setpoint(0),
          _maxTemp(120),
          _minTemp(-20),
          _lastTime(0),
          _lastError(0),
          _integral(0),
          _lastTemp(0),
          _lastTempChangeTime(0),
          _stuckThreshold(0.2),
          _stuckTimeout(10),
          _softStartDuration(10),
          _adaptiveSoftStartEnabled(true),
          _adaptiveRange(20),
          _ambientTemp(20),
          _heatStartTime(0)
    {}

    // --- Safety Configuration ---
    void setMaxTemp(float t) { _maxTemp = t; }
    void setMinTemp(float t) { _minTemp = t; }
    void setStuckDetection(float threshold, int timeoutSec) {
        _stuckThreshold = threshold;
        _stuckTimeout = timeoutSec;
    }

    // --- Soft-start configuration ---
    void setSoftStartDuration(int seconds) {
        _softStartDuration = seconds;
    }

    void enableAdaptiveSoftStart(bool enable) {
        _adaptiveSoftStartEnabled = enable;
    }

    void setAmbientTemperature(float t) {
        _ambientTemp = t;
    }

    void setAdaptiveRange(float r) {
        _adaptiveRange = r;
    }

    // --- Commands ---
    void setTarget(float temperature) {
        _setpoint = temperature;
        if (_state != Error) {
            _state = Heating;
            _heatStartTime = millis();
        }
    }

    void stop() {
        _state = Off;
    }

    void resetError() {
        if (_state == Error) {
            _integral = 0;
            _lastError = 0;
            _state = Off;
        }
    }

    // --- Update Loop ---
    int update(float currentTemp) {
        if (_state == Off) return 0;

        // --- SAFETY CHECKS ---
        if (!isValidTemp(currentTemp)) {
            _state = Error;
            return 0;
        }

        if (currentTemp > _maxTemp || currentTemp < _minTemp) {
            _state = Error;
            return 0;
        }

        if (isSensorStuck(currentTemp)) {
            _state = Error;
            return 0;
        }

        if (_state == Error) return 0;

        // --- PID LOGIC ---
        unsigned long now = millis();
        float dt = (now - _lastTime) / 1000.0f;

        if (_lastTime == 0) {
            _lastTime = now;
            _lastTemp = currentTemp;
            _lastTempChangeTime = now;
            return 0;
        }

        if (dt <= 0) return 0;

        float error = _setpoint - currentTemp;

        _integral += error * dt;
        if (_integral > 1000) _integral = 1000;
        if (_integral < -1000) _integral = -1000;

        float derivative = (error - _lastError) / dt;

        float output = _kp * error + _ki * _integral + _kd * derivative;

        // --- SOFT START LIMIT ---
        float maxAllowed = softStartLimit(now);
        if (output > maxAllowed) output = maxAllowed;
        if (output < 0) output = 0;

        if (output > _maxPower) output = _maxPower;

        _lastError = error;
        _lastTime = now;

        return (int)output;
    }

    State state() const { return _state; }

private:
    float _kp, _ki, _kd;
    float _maxPower;

    State _state;
    float _setpoint;

    float _maxTemp;
    float _minTemp;

    unsigned long _lastTime;
    float _lastError;
    float _integral;

    // Stuck sensor detection
    float _lastTemp;
    unsigned long _lastTempChangeTime;
    float _stuckThreshold;
    int _stuckTimeout;

    // Soft start
    int _softStartDuration;
    bool _adaptiveSoftStartEnabled;
    float _adaptiveRange;
    float _ambientTemp;
    unsigned long _heatStartTime;

    bool isValidTemp(float t) {
        if (isnan(t)) return false;
        if (t < -100 || t > 300) return false;
        return true;
    }

    bool isSensorStuck(float t) {
        unsigned long now = millis();

        if (fabs(t - _lastTemp) > _stuckThreshold) {
            _lastTemp = t;
            _lastTempChangeTime = now;
            return false;
        }

        if ((now - _lastTempChangeTime) / 1000 > _stuckTimeout)
            return true;

        return false;
    }

    float computeAdaptiveDuration() {
        if (!_adaptiveSoftStartEnabled)
            return _softStartDuration;

        float diff = _setpoint - _ambientTemp;

        // Clamp diff to range
        if (diff < 0) diff = 0;
        if (diff > _adaptiveRange) diff = _adaptiveRange;

        float factor = 1.0f - (diff / _adaptiveRange);

        return _softStartDuration * factor;
    }

    float softStartLimit(unsigned long now) {
        float adaptiveDuration = computeAdaptiveDuration();
        float elapsed = (now - _heatStartTime) / 1000.0f;

        if (elapsed >= adaptiveDuration)
            return _maxPower;

        float ramp = (elapsed / adaptiveDuration) * _maxPower;
        return ramp;
    }
};

/*
===============================================================
                        VALVE CLASS — USAGE GUIDE
===============================================================

This class models a motorized valve with:
 - Open / Close commands
 - End‑of‑course switch feedback
 - Timeout protection
 - Error handling
 - Idle state detection

It is designed for non‑blocking use inside a main loop.

---------------------------------------------------------------
1. CREATE THE VALVE OBJECT
---------------------------------------------------------------

Valve valve(address, timeoutMs);

Example:
    Valve valve(3, 40000);   // address = 3, timeout = 40 seconds

Where:
 - address   = logical ID of the valve (for your system)
 - timeoutMs = max allowed movement time before entering ERROR

---------------------------------------------------------------
2. COMMANDS
---------------------------------------------------------------

Open the valve:
    valve.commandOpen();

Close the valve:
    valve.commandClose();

Stop movement manually:
    valve.commandStop();

Reset error state:
    valve.resetError();

---------------------------------------------------------------
3. CALL update() FREQUENTLY (non-blocking)
---------------------------------------------------------------

Inside loop():

    int switchStatus = readValveSwitch();   // 0 = closed, 1 = open
    valve.update(switchStatus);

update() performs:
 - State transitions
 - Timeout detection
 - End‑switch detection
 - Error handling

---------------------------------------------------------------
4. SWITCH STATUS INPUT
---------------------------------------------------------------

switchStatus must be:
    0 → closed limit switch active
    1 → open limit switch active

Any other value triggers:
    state = Error

---------------------------------------------------------------
5. STATE MACHINE OVERVIEW
---------------------------------------------------------------

Idle:
    - Valve is not moving
    - update() auto-detects Opened/Closed based on switchStatus

Opening:
    - Movement started by commandOpen()
    - Ends when switchStatus == 1
    - Timeout → Error

Closing:
    - Movement started by commandClose()
    - Ends when switchStatus == 0
    - Timeout → Error

Opened:
    - Valve fully open

Closed:
    - Valve fully closed

StoppedByUser:
    - Movement manually stopped by commandStop()

Error:
    - Invalid switch input OR timeout
    - Must be cleared with resetError()

---------------------------------------------------------------
6. CHECKING VALVE STATE
---------------------------------------------------------------

    if (valve.state() == Valve::Opened) {
        // Valve is fully open
    }

    if (valve.state() == Valve::Error) {
        // Handle fault condition
    }

---------------------------------------------------------------
7. TYPICAL LOOP EXAMPLE
---------------------------------------------------------------

void loop() {
    int sw = digitalRead(SWITCH_PIN);   // 0 or 1
    valve.update(sw);

    if (valve.state() == Valve::Error) {
        // Stop motor, alert user, etc.
    }
}

===============================================================
*/

class Valve {
public:
    enum State {
        Idle,
        Opening,
        Closing,
        Opened,
        Closed,
        StoppedByUser,
        Error
    };

    Valve(int address, unsigned long timeout = 40000)
        : _address(address),
          _timeout(timeout),
          _state(Idle),
          _startTime(0)
    {}

    void commandOpen() {
        if (_state == Opening || _state == Opened) return;
        _state = Opening;
        _startTime = millis();
    }

    void commandClose() {
        if (_state == Closing || _state == Closed) return;
        _state = Closing;
        _startTime = millis();
    }

    void commandStop() {
        _state = StoppedByUser;
    }

    void resetError() {
        if (_state == Error) _state = Idle;
    }

    void update(int switchStatus) {
        if (switchStatus != 0 && switchStatus != 1) {
            _state = Error;
            return;
        }

        switch (_state) {
        case Idle:
            if (switchStatus == 1) _state = Opened;
            else if (switchStatus == 0) _state = Closed;
            break;

        case Opening:
            handleOpening(switchStatus);
            break;

        case Closing:
            handleClosing(switchStatus);
            break;

        default:
            break;
        }
    }

    State state() const { return _state; }

private:
    int _address;
    unsigned long _timeout;
    unsigned long _startTime;
    State _state;

    bool timedOut() const {
        return millis() - _startTime > _timeout;
    }

    void handleOpening(int switchStatus) {
        if (switchStatus == 1) {
            _state = Opened;
        } else if (timedOut()) {
            _state = Error;
        }
    }

    void handleClosing(int switchStatus) {
        if (switchStatus == 0) {
            _state = Closed;
        } else if (timedOut()) {
            _state = Error;
        }
    }
};



#endif
