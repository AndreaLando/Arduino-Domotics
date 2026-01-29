#ifndef Power_H
#define Power_H

#pragma once
#include <Arduino.h>
#include <vector>
#include <functional>
#include <math.h>
#include <algorithm>

/* ============================================================
   PowerManager – Intelligent Electrical Load Manager
   ============================================================

   This class implements an advanced management system for
   domestic/industrial electrical loads based on:
     • solar production forecasting
     • grid power draw limits
     • load priorities
     • dynamic hysteresis
     • automatic parameter auto‑tuning
     • automatic suggestions (attach/detach)
     • thermal load management (heating/cooling)

   ---------------------------------------------------------
   1. INITIALIZATION
   ---------------------------------------------------------

       PowerManager pm(gridLimitWatt, nominalSolarWatt, hysteresisOn, hysteresisOff);

   Parameters:
       gridLimitWatt     → maximum allowed grid power draw
       nominalSolarWatt  → nominal power of the PV system
       hysteresisOn      → margin for enabling loads
       hysteresisOff     → margin for disabling loads

   ---------------------------------------------------------
   2. CALLBACK REGISTRATION
   ---------------------------------------------------------

       pm.setOnLoadChange([](const String& name, bool state){ ... });
       pm.setOnLimitWarning([](float net, float limit){ ... });
       pm.setOnLimitExceeded([](float net, float limit){ ... });
       pm.setOnError([](int code, const String& msg){ ... });
       pm.setOnSuggestion([](const String& s, int sev, const String& reason){ ... });

   Available callbacks:
       onLoadChange     → notifies every load state change
       onLimitWarning   → warning when approaching the limit
       onLimitExceeded  → limit exceeded
       onError          → sensor or parameter errors
       onSuggestion     → suggestions “attach:” / “detach:”

   Note:
       If autoExecuteSuggestions is enabled, suggestions are
       executed automatically (default: true).

   ---------------------------------------------------------
   3. ADDING LOADS
   ---------------------------------------------------------

   Normal loads:

       pm.addLoad("WashingMachine", PowerManager::Priority::MEDIUM, 1200, 30, 60);

   Parameters:
       name          → load name
       priority      → HIGH / MEDIUM / LOW
       nominalPower  → nominal power
       minOnSec      → minimum ON time
       minOffSec     → minimum OFF time

   Thermal loads:

       pm.addThermalLoad("HeatPump", true, 21.0, 19.0, 23.0, 60, 60);

   Parameters:
       heatingMode   → true = heating, false = cooling
       baseTarget    → base target temperature
       comfortMin    → minimum comfort limit
       comfortMax    → maximum comfort limit

   ---------------------------------------------------------
   4. INPUT DATA UPDATE
   ---------------------------------------------------------

       pm.setGridPower(gridWatt);
       pm.setSolarPower(pvWatt);
       pm.setEnvironmentalData(lux, outdoorTemp);

   Notes:
       • lux is used to estimate production volatility
       • outdoor temperature corrects PV efficiency

   ---------------------------------------------------------
   5. MAIN LOOP (CALL PERIODICALLY)
   ---------------------------------------------------------

       pm.updateLoads(month, hour, minute);
       pm.updateThermalControl(indoorTemp, month, hour, minute);
       pm.autoTuneStep();

   Functions:
       updateLoads()          → manages normal load attach/detach
       updateThermalControl() → manages thermal loads
       autoTuneStep()         → updates hysteresis and minOn/minOff

   ---------------------------------------------------------
   6. OPTIMIZATION MODES
   ---------------------------------------------------------

       pm.setOptimizationMode(PowerManager::OptimizationMode::MAX_SELF_CONSUMPTION);

   Available modes:
       MAX_SELF_CONSUMPTION  → increases dynamic limit
       ECONOMIC_SAVING       → reduces dynamic limit
       MAX_COMFORT           → adds fixed margin
       GRID_PROTECTION       → strongly limits grid draw
       BALANCED              → standard behavior

   ---------------------------------------------------------
   7. AUTOMATIC SUGGESTIONS
   ---------------------------------------------------------

   The system generates strings such as:

       "attach:WashingMachine"
       "detach:Oven"

   If autoExecuteSuggestions = true:
       → the load is automatically enabled/disabled

   If autoExecuteSuggestions = false:
       → the suggestion is only notified via callback

   ---------------------------------------------------------
   8. AUTO‑TUNING
   ---------------------------------------------------------

       pm.enableAutoTune(true);
       pm.setTuneInterval(300000);     // 5 minutes
       pm.setTuneStepPercent(0.05f);   // 5%

   Auto‑tuning automatically adjusts:
       • on/off hysteresis
       • minOn/minOff of loads

   Based on:
       • number of load cycles
       • light volatility (lux)
       • PV forecast error

   ---------------------------------------------------------
   9. SOLAR FORECAST
   ---------------------------------------------------------

       float forecast = pm.getSolarForecastNow(month, hour, minute);

   Based on:
       • theoretical sinusoidal curve
       • current lux
       • outdoor temperature
       • volatility (short‑term correction)

   ---------------------------------------------------------
   10. ERRORS
   ---------------------------------------------------------

       pm.reportError(code, "message");

   Automatically generated errors:
       • invalid sensor values
       • parameters out of range

   ---------------------------------------------------------
   FINAL NOTES
   ---------------------------------------------------------

   The class is designed to be called cyclically (e.g., every 1–5 seconds).
   All attach/detach logic is protected by:
       • minOn/minOff
       • hysteresis
       • automatic rollback if the limit is exceeded

   The system is modular and can be extended with:
       • new load types
       • more advanced forecast logic
       • integration with inverters or smart‑meters

   ============================================================ */


class PowerManager {
public:
    enum class Priority { ALTA = 0, MEDIA = 1, BASSA = 2 };
    enum class OptimizationMode { MASSIMO_AUTOCONSUMO, RISPARMIO_ECONOMICO, MASSIMO_COMFORT, PROTEZIONE_RETE, BILANCIATO };

    struct Load {
        String name;
        Priority priority;
        float nominalPower;
        bool state;
        unsigned long minOnTimeMs;
        unsigned long minOffTimeMs;
        unsigned long lastChangeTime;

        // statistiche per auto-tuning
        unsigned int cyclesCount;
        unsigned long lastCycleTimestamp;
        float avgCycleTimeSec;

        bool suggestedOn = false;
        bool suggestedOff = false;
    };

    struct ThermalLoad {
        String name;
        bool heatingMode;
        float baseTargetTemp;
        float comfortMin;
        float comfortMax;
        bool state;
        unsigned long minOnTimeMs;
        unsigned long minOffTimeMs;
        unsigned long lastChangeTime;

        bool suggestedOn = false;
        bool suggestedOff = false;
    };

    // Callback types
    using LoadCb = std::function<void(const String&, bool)>;
    using LimitCb = std::function<void(float netPower, float limit)>;
    using ErrorCb = std::function<void(int code, const String& msg)>;
    using SuggestionCb = std::function<void(const String& suggestion, int severity, const String& reason)>;

private:
    // Potenze e parametri
    float gridPower = 0.0f;
    float solarPower = 0.0f;
    float gridLimit;
    float nominalSolarPower;
    float hysteresisOn;
    float hysteresisOff;

    // Ambiente e previsione breve
    float luxValue = 0.0f;
    float temperatureExt = 25.0f;

    // Soglia warning
    float limitWarningThreshold = 0.9f;

    // Modalità ottimizzazione
    OptimizationMode mode = OptimizationMode::BILANCIATO;

    // Liste carichi
    std::vector<Load> loads;
    std::vector<ThermalLoad> thermalLoads;

    // Callback registrabili
    LoadCb onLoadChange = nullptr;
    LimitCb onLimitWarning = nullptr;
    LimitCb onLimitExceeded = nullptr;
    ErrorCb onError = nullptr;
    SuggestionCb onSuggestion = nullptr;

    // Suggestion auto-exec
    bool autoExecuteSuggestions = true;
    String lastSuggestion = "";

    // Auto-tuning parameters
    bool autoTuneEnabled = false;
    unsigned long tuneIntervalMs = 5 * 60 * 1000UL; // default 5 minuti
    unsigned long lastTuneTime = 0;
    float tuneStepPercent = 0.05f; // 5% per step
    float hysteresisMin = 50.0f;   // W
    float hysteresisMax = 2000.0f; // W
    unsigned long minOnMinSec = 5;     // sec
    unsigned long minOnMaxSec = 3600;  // sec
    unsigned long minOffMinSec = 5;    // sec
    unsigned long minOffMaxSec = 3600; // sec

    // forecast error EMA
    float forecastErrorMA = 0.0f;
    float forecastErrorAlpha = 0.1f;

    // storico lux per volatilità
    static const int LUX_HISTORY_SIZE = 60;
    float luxHistory[LUX_HISTORY_SIZE];
    int luxIndex = 0;
    bool luxFilled = false;

    struct DayInfo { int sunrise; int sunset; };
    DayInfo months[12] = {
        {480,1020},{450,1050},{420,1080},{390,1110},
        {360,1140},{360,1140},{390,1110},{420,1080},
        {450,1050},{480,1020},{510,990},{540,960}
    };

public:
    PowerManager(float limitWatt, float nominalSolarWatt, float hOn = 200.0f, float hOff = 200.0f)
        : gridLimit(limitWatt), nominalSolarPower(nominalSolarWatt),
          hysteresisOn(hOn), hysteresisOff(hOff) {
        for (int i = 0; i < LUX_HISTORY_SIZE; ++i) luxHistory[i] = 0.0f;
    }

    // Registrazione callback
    void setOnLoadChange(LoadCb cb) { onLoadChange = cb; }
    void setOnLimitWarning(LimitCb cb) { onLimitWarning = cb; }
    void setOnLimitExceeded(LimitCb cb) { onLimitExceeded = cb; }
    void setOnError(ErrorCb cb) { onError = cb; }
    void setOnSuggestion(SuggestionCb cb) { onSuggestion = cb; }

    // Controllo esecuzione automatica delle proposte
    void setAutoExecuteSuggestions(bool enable) { autoExecuteSuggestions = enable; }

    // Auto-tune API
    void enableAutoTune(bool enable) { autoTuneEnabled = enable; }
    void setTuneInterval(unsigned long ms) { tuneIntervalMs = ms; }
    void setTuneStepPercent(float pct) { if (pct > 0.0f && pct < 0.5f) tuneStepPercent = pct; }

    // Configurazioni
    void setOptimizationMode(OptimizationMode m) { mode = m; }
    void setLimitWarningThreshold(float pct) { if (pct > 0.0f && pct < 1.0f) limitWarningThreshold = pct; }

    // Aggiunta carichi
    void addLoad(const String& name, Priority prio, float nominalPower,
                 unsigned long minOnSec = 5, unsigned long minOffSec = 5) {
        Load l;
        l.name = name;
        l.priority = prio;
        l.nominalPower = nominalPower;
        l.state = false;
        l.minOnTimeMs = minOnSec * 1000UL;
        l.minOffTimeMs = minOffSec * 1000UL;
        l.lastChangeTime = 0;
        l.cyclesCount = 0;
        l.lastCycleTimestamp = 0;
        l.avgCycleTimeSec = 0.0f;
        loads.push_back(l);
    }

    void addThermalLoad(const String& name, bool heatingMode,
                        float baseTarget, float comfortMin, float comfortMax,
                        unsigned long minOnSec = 30, unsigned long minOffSec = 30) {
        ThermalLoad t;
        t.name = name;
        t.heatingMode = heatingMode;
        t.baseTargetTemp = baseTarget;
        t.comfortMin = comfortMin;
        t.comfortMax = comfortMax;
        t.state = false;
        t.minOnTimeMs = minOnSec * 1000UL;
        t.minOffTimeMs = minOffSec * 1000UL;
        t.lastChangeTime = 0;
        thermalLoads.push_back(t);
    }

    // Impostazione potenze e ambiente
    void setGridPower(float watt) { gridPower = watt; }
    void setSolarPower(float watt) { solarPower = watt; }
    void setEnvironmentalData(float lux, float tempExt) {
        if (lux < 0.0f || tempExt < -50.0f || tempExt > 80.0f) {
            if (onError) onError(2, "Valore sensore esterno non valido");
            return;
        }
        luxValue = lux;
        temperatureExt = tempExt;
        updateLuxHistory(lux);
    }

    // Storico lux
    void updateLuxHistory(float lux) {
        luxHistory[luxIndex] = lux;
        luxIndex = (luxIndex + 1) % LUX_HISTORY_SIZE;
        if (luxIndex == 0) luxFilled = true;
    }

    float getLuxVolatility() const {
        if (!luxFilled) return 0.0f;
        float mean = 0.0f;
        for (int i = 0; i < LUX_HISTORY_SIZE; ++i) mean += luxHistory[i];
        mean /= LUX_HISTORY_SIZE;
        float var = 0.0f;
        for (int i = 0; i < LUX_HISTORY_SIZE; ++i) {
            float d = luxHistory[i] - mean;
            var += d * d;
        }
        return sqrt(var / LUX_HISTORY_SIZE);
    }

    // Forecast e net power
    float getNetGridPower() const {
        float net = gridPower - solarPower;
        return net > 0.0f ? net : 0.0f;
    }

    float getSolarForecastNow(int month, int hour, int minute) {
        int t = hour * 60 + minute;
        DayInfo d = months[month - 1];
        if (t < d.sunrise || t > d.sunset) return 0.0f;
        float daySpan = d.sunset - d.sunrise;
        float x = (float)(t - d.sunrise) / daySpan;
        float theoretical = nominalSolarPower * sin(PI * x);
        float lightFactor = luxValue / 100000.0f; if (lightFactor > 1.0f) lightFactor = 1.0f;
        float tempFactor = 1.0f - 0.0045f * (temperatureExt - 25.0f); if (tempFactor < 0.0f) tempFactor = 0.0f;
        float base = theoretical * lightFactor * tempFactor;
        // breve termine: semplice correzione basata su volatilità (opzionale)
        float vol = getLuxVolatility();
        float shortCorr = 1.0f;
        if (vol > 40000.0f) shortCorr = 0.8f;
        else if (vol > 20000.0f) shortCorr = 0.9f;
        return base * shortCorr;
    }

    // aggiornamento forecast error da chiamare quando hai produzione reale vs previsione
    void updateForecastError(float forecastW, float actualProductionW) {
        float err = 0.0f;
        if (forecastW > 0.0f) err = fabs(forecastW - actualProductionW) / forecastW;
        forecastErrorMA = forecastErrorAlpha * err + (1.0f - forecastErrorAlpha) * forecastErrorMA;
    }

private:
    bool canChangeState(unsigned long lastChange, unsigned long minTime) const {
        return (millis() - lastChange) >= minTime;
    }

    void notifyLoadChange(const String& name, bool state) {
        // aggiorna statistiche per auto-tuning
        recordLoadStateChange(name, state);
        if (onLoadChange) onLoadChange(name, state);
    }

    void checkLimitsAndNotify(float dynamicLimit) {
        float net = getNetGridPower();
        if (net >= dynamicLimit * limitWarningThreshold && net < dynamicLimit) {
            if (onLimitWarning) onLimitWarning(net, dynamicLimit);
        }
        if (net >= dynamicLimit) {
            if (onLimitExceeded) onLimitExceeded(net, dynamicLimit);
        }
    }

    // Invia suggerimento e, se abilitato, lo esegue
    void suggestAction(const String& suggestion, int severity, const String& reason) {
        // evita duplicati 
        if (suggestion == lastSuggestion) return; 
        lastSuggestion = suggestion;

        if (onSuggestion) onSuggestion(suggestion, severity, reason);
        if (!autoExecuteSuggestions) return;

        if (suggestion.startsWith("stacca:")) {
            String name = suggestion.substring(7);

            // Carichi normali
            for (auto& l : loads) {
                if (l.name == name && l.state) {
                    if (!canChangeState(l.lastChangeTime, l.minOnTimeMs)) return;
                    l.state = false;
                    l.lastChangeTime = millis();

                    // reset debounce 
                    l.suggestedOn = false; l.suggestedOff = false;

                    notifyLoadChange(l.name, false);
                }
            }

            // Carichi termici
            for (auto& t : thermalLoads) {
                if (t.name == name && t.state) {
                    if (!canChangeState(t.lastChangeTime, t.minOnTimeMs)) return;
                    t.state = false;
                    t.lastChangeTime = millis();

                    // reset debounce 
                    t.suggestedOn = false; t.suggestedOff = false;

                    notifyLoadChange(t.name, false);
                }
            }
        } else if (suggestion.startsWith("attacca:")) {
            String name = suggestion.substring(8);

            // Carichi normali
            for (auto& l : loads) {
                if (l.name == name && !l.state) {
                    if (!canChangeState(l.lastChangeTime, l.minOffTimeMs)) return;
                    l.state = true;
                    l.lastChangeTime = millis();

                    // reset debounce 
                    l.suggestedOn = false; l.suggestedOff = false;
                    
                    notifyLoadChange(l.name, true);
                }
            }

            // Carichi termici
            for (auto& t : thermalLoads) {
                if (t.name == name && !t.state) {
                    if (!canChangeState(t.lastChangeTime, t.minOffTimeMs)) return;
                    t.state = true;
                    t.lastChangeTime = millis();

                    // reset debounce 
                    t.suggestedOn = false; t.suggestedOff = false;

                    notifyLoadChange(t.name, true);
                }
            }
        }
    }

public:
    // Funzione che aggiorna i carichi logici (usa dynamicLimit)
    void updateLoads(int month, int hour, int minute) {
        float forecast = getSolarForecastNow(month, hour, minute);
        float dynamicLimit = applyOptimizationMode(gridLimit + forecast);

        // notifica warning/exceeded prima di agire
        checkLimitsAndNotify(dynamicLimit);

        float net = getNetGridPower();

        // 🔥 FIX: evita suggerimenti inutili all'avvio 
        bool allOff = true; 
        for (auto& l : loads) { 
            if (l.state) { 
                allOff = false; 
                break; 
            } 
        } 
                
        // Se tutti i carichi sono spenti e non c’è alcun superamento limite, 
        // non ha senso proporre "attacca" al primo ciclo 
        if (allOff && net < dynamicLimit) { 
            return; 
        } 
            
        // ------------------------- // LOGICA DI ATTACCO CARICHI // -------------------------
        if (net <= (dynamicLimit - hysteresisOn)) {
            std::sort(loads.begin(), loads.end(), [](const Load& a, const Load& b){
                return (int)a.priority < (int)b.priority;
            });
            for (auto& load : loads) {
                if (!load.state) {
                    if (load.suggestedOn) continue; // evita duplicati 
                    load.suggestedOn = true; load.suggestedOff = false;

                    String s = "attacca:" + load.name;
                    suggestAction(s, 0, "margine disponibile");
                    // se autoExecuteSuggestions==false, l'azione non verrà eseguita automaticamente
                    // se autoExecuteSuggestions==true, suggestAction ha già eseguito e notificato
                    if (load.state) {
                        if (getNetGridPower() > dynamicLimit) {
                            // rollback se necessario
                            load.state = false;
                            load.lastChangeTime = millis();
                            notifyLoadChange(load.name, false);
                        }
                    }
                }
            }
            return;
        }

        // ------------------------- // LOGICA DI STACCO CARICHI // -------------------------
        if (net >= (dynamicLimit + hysteresisOff)) {
            std::sort(loads.begin(), loads.end(), [](const Load& a, const Load& b){
                return (int)a.priority > (int)b.priority;
            });
            for (auto& load : loads) {
                if (load.state) {
                    if (load.suggestedOff) continue; // evita duplicati 
                    load.suggestedOff = true; load.suggestedOn = false;

                    String s = "stacca:" + load.name;
                    suggestAction(s, 2, "superamento limite");
                    // se autoExecuteSuggestions==false, l'azione rimane suggerita ma non eseguita
                    if (!load.state) {
                        if (getNetGridPower() <= dynamicLimit) break;
                    }
                }
            }
        }
    }

    // Gestione termica (semplice) integrata
    void updateThermalControl(float indoorTemp, int month, int hour, int minute) {
        float forecast = getSolarForecastNow(month, hour, minute);
        for (auto& t : thermalLoads) {
            float boost = 0.0f;
            if (forecast > nominalSolarPower * 0.6f) boost = t.heatingMode ? 1.0f : -1.0f;
            float target = t.baseTargetTemp + boost;
            if (t.heatingMode) {
                if (indoorTemp < target && indoorTemp < t.comfortMax) {
                    String s = "attacca:" + t.name;
                    suggestAction(s, 1, "thermal control");
                } else if (indoorTemp > target + 0.5f) {
                    String s = "stacca:" + t.name;
                    suggestAction(s, 1, "thermal control");
                }
            } else {
                if (indoorTemp > target && indoorTemp > t.comfortMin) {
                    String s = "attacca:" + t.name;
                    suggestAction(s, 1, "thermal control");
                } else if (indoorTemp < target - 0.5f) {
                    String s = "stacca:" + t.name;
                    suggestAction(s, 1, "thermal control");
                }
            }
        }
    }

    // Applicazione semplice della modalità di ottimizzazione
    float applyOptimizationMode(float dynamicLimit) {
        switch (mode) {
            case OptimizationMode::MASSIMO_AUTOCONSUMO: return dynamicLimit * 1.2f;
            case OptimizationMode::RISPARMIO_ECONOMICO: return dynamicLimit * 0.8f;
            case OptimizationMode::MASSIMO_COMFORT: return dynamicLimit + 500.0f;
            case OptimizationMode::PROTEZIONE_RETE: return dynamicLimit * 0.6f;
            case OptimizationMode::BILANCIATO:
            default: return dynamicLimit;
        }
    }

    // registrare cambi stato per statistiche auto-tuning
    void recordLoadStateChange(const String& name, bool newState) {
        unsigned long now = millis();
        for (auto& l : loads) {
            if (l.name == name) {
                // consideriamo ogni cambio come possibile parte di un ciclo
                if (l.lastCycleTimestamp == 0) {
                    l.lastCycleTimestamp = now;
                } else {
                    unsigned long dtMs = now - l.lastCycleTimestamp;
                    l.lastCycleTimestamp = now;
                    l.cyclesCount++;
                    float dtSec = dtMs / 1000.0f;
                    float alpha = 0.2f;
                    if (l.avgCycleTimeSec == 0.0f) l.avgCycleTimeSec = dtSec;
                    else l.avgCycleTimeSec = alpha * dtSec + (1.0f - alpha) * l.avgCycleTimeSec;
                }
                break;
            }
        }
    }

    // AUTO-TUNING: chiamare periodicamente (es. nel loop)
    void autoTuneStep() {
        if (!autoTuneEnabled) return;
        unsigned long now = millis();
        if (now - lastTuneTime < tuneIntervalMs) return;
        lastTuneTime = now;

        // metriche globali
        float luxVol = getLuxVolatility();
        float avgCycles = 0.0f;
        int cnt = 0;
        for (auto& l : loads) {
            avgCycles += (float)l.cyclesCount;
            cnt++;
        }
        if (cnt > 0) avgCycles /= cnt;

        float forecastErr = forecastErrorMA;

        // decisione su isteresi
        bool increaseHyst = (avgCycles > 3.0f) || (luxVol > 20000.0f) || (forecastErr > 0.25f);
        bool decreaseHyst = (avgCycles < 1.0f) && (luxVol < 5000.0f) && (forecastErr < 0.1f);

        float deltaHystOn = hysteresisOn * tuneStepPercent;
        float deltaHystOff = hysteresisOff * tuneStepPercent;

        if (increaseHyst) {
            hysteresisOn = min(hysteresisOn + deltaHystOn, hysteresisMax);
            hysteresisOff = min(hysteresisOff + deltaHystOff, hysteresisMax);
        } else if (decreaseHyst) {
            hysteresisOn = max(hysteresisOn - deltaHystOn, hysteresisMin);
            hysteresisOff = max(hysteresisOff - deltaHystOff, hysteresisMin);
        }

        // regola minOn/minOff per carico
        for (auto& l : loads) {
            if (l.cyclesCount >= 3 && l.avgCycleTimeSec < 120.0f) {
                unsigned long newMinOn = min((unsigned long)(l.minOnTimeMs * (1.0f + tuneStepPercent)), minOnMaxSec * 1000UL);
                unsigned long newMinOff = min((unsigned long)(l.minOffTimeMs * (1.0f + tuneStepPercent)), minOffMaxSec * 1000UL);
                l.minOnTimeMs = newMinOn;
                l.minOffTimeMs = newMinOff;
            } else if (l.cyclesCount == 0 && l.avgCycleTimeSec > 600.0f) {
                unsigned long newMinOn = max((unsigned long)(l.minOnTimeMs * (1.0f - tuneStepPercent)), minOnMinSec * 1000UL);
                unsigned long newMinOff = max((unsigned long)(l.minOffTimeMs * (1.0f - tuneStepPercent)), minOffMinSec * 1000UL);
                l.minOnTimeMs = newMinOn;
                l.minOffTimeMs = newMinOff;
            }
            // reset contatori per prossimo intervallo
            l.cyclesCount = 0;
            l.avgCycleTimeSec = 0.0f;
            l.lastCycleTimestamp = 0;
        }

        // notifica via suggestion callback
        if (onSuggestion) {
            String s = "AutoTune applied: hystOn=" + String(hysteresisOn) + "W hystOff=" + String(hysteresisOff) + "W";
            onSuggestion(s, 0, "auto-tuning");
        }
    }

    // Metodo pubblico per forzare un errore (esempio di uso)
    void reportError(int code, const String& msg) {
        if (onError) onError(code, msg);
    }
};


#endif
