#ifndef Weather_H
#define Weather_H

#pragma once
#include <Arduino.h>

/* ============================================================
   ISTRUZIONI D’USO — WeatherStation
   ============================================================

   La classe WeatherStation gestisce:
   - Lettura sensori tramite funzioni esterne iniettate dall’utente
   - Media mobile su finestra circolare
   - Allarmi edge‑triggered con debounce configurabile
   - Eventi meteorologici: pioggia inizio/fine, raffiche di vento,
     transizione giorno/notte con isteresi

   ------------------------------------------------------------
   1) INIZIALIZZAZIONE
   ------------------------------------------------------------
   È necessario fornire al costruttore quattro funzioni di lettura:
       int readTemp();
       int readWind();
       int readRain();
       int readLight();

   Esempio:
       WeatherStation ws(readTemp, readWind, readRain, readLight);

   È possibile specificare opzionalmente:
       - Fattori di conversione ADC → unità fisiche
       - Soglie allarmi (temperatura, vento, pioggia)
       - Soglie giorno/notte (isteresi)

   ------------------------------------------------------------
   2) CALLBACK EVENTI E ALLARMI
   ------------------------------------------------------------
   La classe supporta due callback opzionali:

       void setEventCallback(void (*cb)(WeatherEvent));
       void setAlarmCallback(void (*cb)(const WeatherAlarm*, int));

   EVENTI generati:
       - RainStart / RainStop
       - WindGustStart / WindGustEnd
       - DayStart / NightStart

   ALLARMI generati (edge-triggered):
       - TempLow
       - TempHigh
       - WindHigh
       - RainHigh

   La callback allarmi riceve un array di allarmi attivi e il numero
   di elementi.

   ------------------------------------------------------------
   3) DEBOUNCE
   ------------------------------------------------------------
   Il debounce degli allarmi è configurabile:
       ws.setAlarmDebounce(n);

   Pioggia e raffiche di vento hanno debounce dedicato interni.

   ------------------------------------------------------------
   4) LETTURA SENSORI
   ------------------------------------------------------------
   Le letture fornite all’utente sono già filtrate tramite media mobile:

       float t = ws.getTemperature();
       float w = ws.getWind();
       float r = ws.getRain();
       float l = ws.getLight();

   ------------------------------------------------------------
   5) CICLO PRINCIPALE
   ------------------------------------------------------------
   È necessario chiamare update() ad ogni iterazione del loop:

       void loop() {
           ws.update();
           ...
       }

   L’update esegue:
       - Acquisizione sensori
       - Aggiornamento media mobile
       - Rilevamento giorno/notte
       - Rilevamento pioggia inizio/fine
       - Rilevamento raffiche di vento
       - Controllo allarmi con edge-trigger

   ------------------------------------------------------------
   6) NOTE DI UTILIZZO
   ------------------------------------------------------------
   - Le funzioni di lettura devono essere NON bloccanti.
   - La finestra della media mobile è di 10 campioni.
   - Gli eventi vengono generati solo su cambiamento di stato.
   - Gli allarmi sono edge-triggered: la callback viene chiamata
     solo quando cambia almeno uno stato di allarme.
   - La classe non usa memoria dinamica ed è sicura per MCU a
     risorse limitate.

   ============================================================ */

/* ============================================================
   ENUM: Eventi meteorologici (edge-triggered)
   ============================================================ */
enum class WeatherEvent {
    RainStart,
    RainStop,
    WindGustStart,
    WindGustEnd,
    DayStart,
    NightStart
};

/* ============================================================
   ENUM: Allarmi meteorologici (edge-triggered)
   ============================================================ */
enum class WeatherAlarm {
    TempLow,
    TempHigh,
    WindHigh,
    RainHigh
};

/* ============================================================
   CLASSE: WeatherStation
   Gestisce sensori analogici, allarmi, eventi e media mobile
   ============================================================ */
class WeatherStation {
private:

    /* ============================================================
       Funzioni esterne di lettura sensori (iniettate dall’utente)
       ============================================================ */
    int (*readTemp)();
    int (*readWind)();
    int (*readRain)();
    int (*readLight)();

    /* ============================================================
       Fattori di conversione ADC → unità fisiche
       ============================================================ */
    float tempFactor;
    float windFactor;
    float rainFactor;
    float lightFactor;

    /* ============================================================
       Media mobile (buffer circolare)
       ============================================================ */
    static const int WINDOW = 10;
    int tempBuf[WINDOW];
    int windBuf[WINDOW];
    int rainBuf[WINDOW];
    int lightBuf[WINDOW];

    int index = 0;
    bool filled = false;

    /* ============================================================
       Soglie allarmi
       ============================================================ */
    float lowTempThreshold;
    float highTempThreshold;
    float highWindThreshold;
    float highRainThreshold;

    /* ============================================================
       Debounce allarmi
       ============================================================ */
    int debounceThreshold = 3;

    int debounceCountTempLow  = 0;
    int debounceCountTempHigh = 0;
    int debounceCountWindHigh = 0;
    int debounceCountRainHigh = 0;

    /* ============================================================
       Stati precedenti allarmi (edge-trigger)
       ============================================================ */
    bool prevTempLow  = false;
    bool prevTempHigh = false;
    bool prevWindHigh = false;
    bool prevRainHigh = false;

    /* ============================================================
       Giorno / Notte con isteresi
       ============================================================ */
    float lightDayThreshold;
    float lightNightThreshold;

    bool isCurrentlyDay = true;
    bool prevDayState = true;

    /* ============================================================
       Pioggia inizio/fine (eventi)
       ============================================================ */
    bool isRaining = false;
    int rainStartDebounce = 3;
    int rainStopDebounce  = 3;

    int rainStartCounter = 0;
    int rainStopCounter  = 0;

    float rainStartThreshold = 40.0;
    float rainStopThreshold  = 20.0;

    /* ============================================================
       Raffiche di vento (eventi)
       ============================================================ */
    bool isGust = false;
    int gustDebounce = 2;

    int gustCounter = 0;

    float windGustDelta = 5.0;
    float lastWindValue = 0.0;

    /* ============================================================
       Callback eventi e allarmi
       ============================================================ */
    void (*eventCallback)(WeatherEvent) = nullptr;
    void (*alarmCallback)(const WeatherAlarm *, int) = nullptr;

    /* ============================================================
       Utility: conversione ADC → tensione
       ============================================================ */
    float toVoltage(int raw) {
        return raw * (5.0 / 1023.0);
    }

    /* ============================================================
       Utility: media mobile
       ============================================================ */
    float movingAverage(int *buffer) {
        long sum = 0;
        int count = filled ? WINDOW : index;

        for (int i = 0; i < count; i++)
            sum += buffer[i];

        return (float)sum / count;
    }

    /* ============================================================
       Debounce generico per allarmi
       ============================================================ */
    bool debounceAlarm(bool condition, int &counter) {
        if (condition) {
            counter++;
            if (counter >= debounceThreshold) {
                counter = debounceThreshold;
                return true;
            }
        } else {
            counter = 0;
        }
        return false;
    }

    /* ============================================================
       Controllo allarmi con edge-trigger + debounce
       ============================================================ */
    void checkAlarms() {
        bool tLow  = debounceAlarm(getTemperature() <= lowTempThreshold,  debounceCountTempLow);
        bool tHigh = debounceAlarm(getTemperature() >= highTempThreshold, debounceCountTempHigh);
        bool wHigh = debounceAlarm(getWind()        >= highWindThreshold, debounceCountWindHigh);
        bool rHigh = debounceAlarm(getRain()        >= highRainThreshold, debounceCountRainHigh);

        bool changed =
            (tLow  != prevTempLow)  ||
            (tHigh != prevTempHigh) ||
            (wHigh != prevWindHigh) ||
            (rHigh != prevRainHigh);

        if (changed && alarmCallback != nullptr) {
            WeatherAlarm active[4];
            int count = 0;

            if (tLow)  active[count++] = WeatherAlarm::TempLow;
            if (tHigh) active[count++] = WeatherAlarm::TempHigh;
            if (wHigh) active[count++] = WeatherAlarm::WindHigh;
            if (rHigh) active[count++] = WeatherAlarm::RainHigh;

            alarmCallback(active, count);
        }

        prevTempLow  = tLow;
        prevTempHigh = tHigh;
        prevWindHigh = wHigh;
        prevRainHigh = rHigh;
    }

    /* ============================================================
       Rilevamento pioggia inizio/fine
       ============================================================ */
    void checkRainEvents() {
        float rain = getRain();

        if (!isRaining) {
            if (rain >= rainStartThreshold) {
                rainStartCounter++;
                if (rainStartCounter >= rainStartDebounce) {
                    isRaining = true;
                    rainStartCounter = 0;
                    if (eventCallback) eventCallback(WeatherEvent::RainStart);
                }
            } else {
                rainStartCounter = 0;
            }
        } else {
            if (rain <= rainStopThreshold) {
                rainStopCounter++;
                if (rainStopCounter >= rainStopDebounce) {
                    isRaining = false;
                    rainStopCounter = 0;
                    if (eventCallback) eventCallback(WeatherEvent::RainStop);
                }
            } else {
                rainStopCounter = 0;
            }
        }
    }

    /* ============================================================
       Rilevamento raffiche di vento
       ============================================================ */
    void checkWindGust() {
        float wind = getWind();
        float delta = wind - lastWindValue;

        if (!isGust) {
            if (delta >= windGustDelta) {
                gustCounter++;
                if (gustCounter >= gustDebounce) {
                    isGust = true;
                    gustCounter = 0;
                    if (eventCallback) eventCallback(WeatherEvent::WindGustStart);
                }
            } else {
                gustCounter = 0;
            }
        } else {
            if (delta <= 0) {
                gustCounter++;
                if (gustCounter >= gustDebounce) {
                    isGust = false;
                    gustCounter = 0;
                    if (eventCallback) eventCallback(WeatherEvent::WindGustEnd);
                }
            } else {
                gustCounter = 0;
            }
        }

        lastWindValue = wind;
    }

    /* ============================================================
       Rilevamento giorno/notte con isteresi
       ============================================================ */
    void updateDayNightState() {
        float light = getLight();

        if (light >= lightDayThreshold)
            isCurrentlyDay = true;
        else if (light <= lightNightThreshold)
            isCurrentlyDay = false;
    }

    void checkDayNightEvent() {
        if (isCurrentlyDay != prevDayState) {
            if (eventCallback) {
                if (isCurrentlyDay)
                    eventCallback(WeatherEvent::DayStart);
                else
                    eventCallback(WeatherEvent::NightStart);
            }
        }
        prevDayState = isCurrentlyDay;
    }

public:

    /* ============================================================
       COSTRUTTORE
       ============================================================ */
    WeatherStation(
        int (*tempFunc)(),
        int (*windFunc)(),
        int (*rainFunc)(),
        int (*lightFunc)(),
        float tFactor = 100.0,
        float wFactor = 6.0,
        float rFactor = 20.0,
        float lFactor = 100.0,
        float lowT = 0.0,
        float highT = 40.0,
        float highW = 20.0,
        float highR = 70.0,
        float dayTh = 300.0,
        float nightTh = 200.0
    ) :
        readTemp(tempFunc),
        readWind(windFunc),
        readRain(rainFunc),
        readLight(lightFunc),
        tempFactor(tFactor),
        windFactor(wFactor),
        rainFactor(rFactor),
        lightFactor(lFactor),
        lowTempThreshold(lowT),
        highTempThreshold(highT),
        highWindThreshold(highW),
        highRainThreshold(highR),
        lightDayThreshold(dayTh),
        lightNightThreshold(nightTh)
    {
        for (int i = 0; i < WINDOW; i++) {
            tempBuf[i] = windBuf[i] = rainBuf[i] = lightBuf[i] = 0;
        }
    }

    /* ============================================================
       SETTER callback
       ============================================================ */
    void setEventCallback(void (*cb)(WeatherEvent)) {
        eventCallback = cb;
    }

    void setAlarmCallback(void (*cb)(const WeatherAlarm *, int)) {
        alarmCallback = cb;
    }

    /* ============================================================
       SETTER debounce allarmi
       ============================================================ */
    void setAlarmDebounce(int n) {
        if (n < 1) n = 1;
        debounceThreshold = n;
    }

    /* ============================================================
       LETTURE sensori (media mobile)
       ============================================================ */
    float getTemperature() {
        return toVoltage(movingAverage(tempBuf)) * tempFactor;
    }

    float getWind() {
        return toVoltage(movingAverage(windBuf)) * windFactor;
    }

    float getRain() {
        return toVoltage(movingAverage(rainBuf)) * rainFactor;
    }

    float getLight() {
        return toVoltage(movingAverage(lightBuf)) * lightFactor;
    }

    /* ============================================================
       UPDATE principale (da chiamare nel loop)
       ============================================================ */
    void update() {
        tempBuf[index] = readTemp();
        windBuf[index] = readWind();
        rainBuf[index] = readRain();
        lightBuf[index] = readLight();

        index++;
        if (index >= WINDOW) {
            index = 0;
            filled = true;
        }

        updateDayNightState();
        checkRainEvents();
        checkWindGust();
        checkDayNightEvent();
        checkAlarms();
    }
};

#endif
