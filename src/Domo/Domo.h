#ifndef Domo_H
#define Domo_H

#pragma once
#include <Arduino.h>
#include "Fncs.h"
#include <vector>

// ************ IO BUFFER *******************************
//RESERVED
const int AREA_SYSTEM_ERRORS=0;
const int AREA_SYSTEM_RUNNING_T=1;
const int AREA_SYSTEM_FLAGS=2; //Bit flags

// END RESERVED

//System status manager
class SystemManager {
public:
    typedef struct {
        Cell<bool> allarmeIntrusione;
        Cell<bool> allarmeIntrusioneH24;
        Cell<bool> allarmeAllagamento;
        Cell<bool> allarmeFumo;
        Cell<bool> finestreAperte;
        Cell<bool> porteAperte;
        Cell<bool> allarmeVideosorveglianza;
        Cell<bool> mancanzaTensione;
        Cell<bool> interventoProtezioneCasa;
        Cell<bool> devicesInAllarme;
    } SystemInfo;

    enum SystemField {
        ALLARME_INTRUSIONE,
        ALLARME_INTRUSIONE_H24,
        ALLARME_ALLAGAMENTO,
        ALLARME_FUMO,
        FINESTRE_APERTE,
        PORTE_APERTE,
        ALLARME_VIDEOSORVEGLIANZA,
        MANCANZA_TENSIONE,
        INTERVENTO_PROTEZIONE_CASA,
        DEVICES_IN_ALLARME
    };

    SystemManager() {
        // inizializza tutto a false
        info.allarmeIntrusione.set(false);
        info.allarmeIntrusioneH24.set(false);
        info.allarmeAllagamento.set(false);
        info.allarmeFumo.set(false);
        info.finestreAperte.set(false);
        info.porteAperte.set(false);
        info.allarmeVideosorveglianza.set(false);
        info.mancanzaTensione.set(false);
        info.interventoProtezioneCasa.set(false);
        info.devicesInAllarme.set(false);
    }

    int getBitmask() {
        int mask = 0;

        if (info.allarmeIntrusione.get())      mask |= (1 << ALLARME_INTRUSIONE);
        if (info.allarmeIntrusioneH24.get())   mask |= (1 << ALLARME_INTRUSIONE_H24);
        if (info.allarmeAllagamento.get())     mask |= (1 << ALLARME_ALLAGAMENTO);
        if (info.allarmeFumo.get())            mask |= (1 << ALLARME_FUMO);
        if (info.finestreAperte.get())         mask |= (1 << FINESTRE_APERTE);
        if (info.porteAperte.get())            mask |= (1 << PORTE_APERTE);
        if (info.allarmeVideosorveglianza.get())    mask |= (1 << ALLARME_VIDEOSORVEGLIANZA);
        if (info.mancanzaTensione.get())            mask |= (1 << MANCANZA_TENSIONE);
        if (info.interventoProtezioneCasa.get())    mask |= (1 << INTERVENTO_PROTEZIONE_CASA);
        if (info.devicesInAllarme.get())        mask |= (1 << DEVICES_IN_ALLARME);

        return mask;
    }

    inline bool hasChanged() const {
        return info.allarmeIntrusione.hasChanged() ||
            info.allarmeIntrusioneH24.hasChanged() ||
            info.allarmeAllagamento.hasChanged() ||
            info.allarmeFumo.hasChanged() ||
            info.finestreAperte.hasChanged() ||
            info.porteAperte.hasChanged() ||
            info.allarmeVideosorveglianza.hasChanged() ||
            info.mancanzaTensione.hasChanged() ||
            info.interventoProtezioneCasa.hasChanged() ||
            info.devicesInAllarme.hasChanged();
        }

    void set(SystemField field, bool value) {
        switch (field) {
            case ALLARME_INTRUSIONE:      info.allarmeIntrusione.setIfDiff(value); break;
            case ALLARME_INTRUSIONE_H24:  info.allarmeIntrusioneH24.setIfDiff(value); break;
            case ALLARME_ALLAGAMENTO:     info.allarmeAllagamento.setIfDiff(value); break;
            case ALLARME_FUMO:            info.allarmeFumo.setIfDiff(value); break;
            case FINESTRE_APERTE:         info.finestreAperte.setIfDiff(value); break;
            case PORTE_APERTE:            info.porteAperte.setIfDiff(value); break;
            case ALLARME_VIDEOSORVEGLIANZA: info.allarmeVideosorveglianza.setIfDiff(value); break;
            case MANCANZA_TENSIONE: info.mancanzaTensione.setIfDiff(value); break;
            case INTERVENTO_PROTEZIONE_CASA: info.interventoProtezioneCasa.setIfDiff(value); break;
            case DEVICES_IN_ALLARME: info.devicesInAllarme.setIfDiff(value); break;
        }
    }

    SystemInfo getInfo() {
        return info;
    }

private:
    SystemInfo info;
};


struct WatchdogStatus {
    bool overload = false;        // true se i tempi sono troppo alti
    bool blocked = false;         // callback bloccato
    bool unstable = false;        // troppi spike
    bool inactive = false;        // callback non chiamato
    const char* reason = nullptr; // testo descrittivo
};

// ---- Timing structs ----
struct ExecTiming {
    const char* name = nullptr;
    unsigned long last = 0;     // last execution time
    float avg = 0;              // exponential moving average
    bool spike = false;         // spike flag
    unsigned long maxSpike = 0; // largest spike seen

    // Per Watchdog 
    unsigned int spikeCount = 0; unsigned long lastSpikeTime = 0;
};

/*
2.0-4.0	molto alta	rileva spike piccoli
4.0-6.0	media (default)	buon equilibrio
7.0–10.0	bassa	rileva solo spike seri
*/
struct CallbackTimings {
    ExecTiming somethingChanged;
    ExecTiming route;
    ExecTiming activityLoop;
    ExecTiming updateCycle;

    float spikeThresholdFactor = 11.5; // configurable multiplier
};

class DomoManager {
public:
    using InitDevicesFn = void (*)(DomoManager&);
    using InitBufferFn  = void (*)(DomoManager&);
    using ActivityLoopFn = void (*)(ModbusBuffer &);

private:
    const int PNL_POLL = 150; //150 is good for fast

    std::vector<GenericPrgDevice> PrgDevices;
    List<structIP> IPs;
    ModbusBuffer Buffer;
    ToggleManager Toggles;

    InitDevicesFn initDevicesFn;
    InitBufferFn initBufferFn;

    SomethingChangedFn somethingChanged;
    RouteFn route;
    ActivityLoopFn activityLoop;

    pin_size_t ledR, ledW, ledPnl, ledErr;
    int areaErrors, areaRunningT;

    // NEW: timing struct
    CallbackTimings timings;

    // Static instance for wrappers 
    static DomoManager* instance;

    //Watchdog
    using WatchdogFn = void (*)(const WatchdogStatus&);
    WatchdogFn watchdogCallback = nullptr;
    
    // ---------- Timing Helpers ----------
    template<typename Fn>
    unsigned long Measure(Fn f) {
        unsigned long t0 = millis();
        f();
        return millis() - t0;
    }

    void UpdateTiming(ExecTiming &t, unsigned long exec, float threshold) {
        t.last = exec;

        if (t.avg == 0) {
            t.avg = exec;
            return;
        }

        const float alpha = 0.1f;
        t.avg = t.avg * (1.0f - alpha) + exec * alpha;

        bool wasSpike = t.spike;
        t.spike = exec > t.avg * threshold;

        if (t.spike) {
            if (exec > t.maxSpike)
                t.maxSpike = exec;

            //Aggiorna i contatori per il Watchdog
            t.spikeCount++; t.lastSpikeTime = millis();

            // 🔥 LOG AUTOMATICO
            if (!wasSpike) {  // log solo quando lo spike inizia
                Serial.print("[SPIKE] "); 
                if (t.name) { 
                    Serial.print(t.name); 
                    Serial.print(" "); 
                } 
                Serial.print("exec=");
                Serial.print(exec);
                Serial.print("ms avg=");
                Serial.print(t.avg);
                Serial.print(" threshold=");
                Serial.println(t.avg * threshold);
            }
        }
    }


    int DeviceHasErrors(std::vector<GenericPrgDevice> prgDevices) {
        static unsigned long Mask = 0;
        short _errors = 0;

        int _deviceIndex = 0;
        for (auto& prgDevice : prgDevices) {
            if (prgDevice.IsInError()) {
                _errors += 1;

                if (!bitRead(Mask, _deviceIndex)) {
                    Serial.print(">>>> MAX ERRORS REACHED - DEVICE EXCLUSION ");
                    Serial.print(prgDevice.GetName());
                    Serial.print(" IP: ");
                    Serial.print(prgDevice.GetIp());
                    Serial.print(" Address: ");
                    Serial.println(prgDevice.GetDeviceAddress());
                    bitSet(Mask, _deviceIndex);
                }
            } else {
                bitClear(Mask, _deviceIndex);
            }
            _deviceIndex++;
        }

        return _errors;
    }

    //Invece di passare i callback originali direttamente a ManageMdbCli, passi dei wrapper che misurano il tempo e poi chiamano il callback vero.
    static void SomethingChangedWrapper(ModbusBuffer &buf) { 
        if (instance && instance->somethingChanged) { 
            unsigned long exec = instance->Measure([&]() { 
                instance->somethingChanged(buf); 
            }); 
            instance->UpdateTiming(instance->timings.somethingChanged, exec, instance->timings.spikeThresholdFactor); 
        } 
    } 
    
    static void RouteWrapper(BufferSourceInfo in, int area, ModbusBuffer &buf) { 
        if (instance && instance->route) { 
            unsigned long exec = instance->Measure([&]() { 
                instance->route(in, area, buf); 
            }); 
            instance->UpdateTiming(instance->timings.route, exec, instance->timings.spikeThresholdFactor); 
        } 
    }

    void CheckWatchdog() {
        WatchdogStatus st;
        unsigned long now = millis();

        //Analizza timing di activityLoop
        ExecTiming &t = timings.activityLoop;
        // 1. Blocco
        if (t.last > 120) {
            st.blocked = true;
            st.reason = "activityLoop blocked (>120ms)";
        }

        // 2. Media troppo alta
        else if (t.avg > 70) {
            st.overload = true;
            st.reason = "activityLoop avg too high (>70ms)";
        }

        // 3. Troppi spike
        else if (t.spikeCount > 10 &&
                (now - t.lastSpikeTime) < 60000) {
            st.unstable = true;
            st.reason = "too many spikes in 60s";
        }

        /*
        // 4. Nessuna attività
        static unsigned long lastExec = 0;
        if (t.last > 0)
            lastExec = now;

        if (!st.reason && (now - lastExec > 60000)) {
            st.inactive = true;
            st.reason = "no activityLoop execution for 60s";
        } */

        //Analizza timing di updateCycle
        ExecTiming &u = timings.updateCycle;

        // Ciclo troppo lento → congestione o blocco
        if (u.last > 150) {
            st.overload = true;
            st.reason = "Update cycle too slow (>150ms)";
        }

        if (u.avg > 120) {
            st.overload = true;
            st.reason = "Update cycle avg too high (>120ms)";
        }


        //Per tutti i flag settati
        // Se c’è un problema, chiama la callback esterna
        if (st.reason && watchdogCallback) {
            watchdogCallback(st);
        }
    }

    SystemManager system;
public:

    DomoManager(int areas, InitDevicesFn initDevices, InitBufferFn initBuffer,
                pin_size_t ledR, pin_size_t ledW, pin_size_t ledPnl, pin_size_t ledErr)
        : Buffer(areas), initDevicesFn(initDevices), initBufferFn(initBuffer)
    {
        this->ledR = ledR;
        this->ledW = ledW;
        this->ledPnl = ledPnl;
        this->ledErr = ledErr;

        this->areaErrors = AREA_SYSTEM_ERRORS;
        this->areaRunningT = AREA_SYSTEM_RUNNING_T;

        //Battezza nomi dei timings del watchdog
        timings.somethingChanged.name = "somethingChanged";
        timings.route.name = "route";
        timings.activityLoop.name = "activityLoop";
        timings.updateCycle.name = "updateCycle";
    }

    void Begin(SomethingChangedFn somethingChanged, RouteFn route, ActivityLoopFn activityLoop) {
        this->somethingChanged = somethingChanged;
        this->route = route;
        this->activityLoop = activityLoop;

        // set static instance pointer 
        instance = this;

        if (initDevicesFn) initDevicesFn(*this);
        if (initBufferFn) initBufferFn(*this);

        BuildIps(PrgDevices, &IPs);

        Serial.print("Hw items to query: ");
        Serial.println(PrgDevices.size());
    }

    void SetWatchdogCallback(WatchdogFn fn) {
        watchdogCallback = fn;
    }

    ModbusBuffer& GetBuffer() {
        return this->Buffer;
    }

    const CallbackTimings& GetTimings() const {
        return timings;
    }

    bool ExistDevicesByIp(int ipIdx) {
        for (auto& prgDevice : PrgDevices) {
            if (prgDevice.GetIp() == IPs.get(ipIdx).IP)
                return true;
        }
        return false;
    }

    void Update(EthernetClient &client, MgsModbus &modbusTCPServer, ModbusTCPClient &modbusTCPClient)
    {
        unsigned long _runningT = millis();
        static unsigned long _lastPnlPoll = millis();
        static short ipIdx = 0;
        static bool _rw = false;             

        if (ExistDevicesByIp(ipIdx)) {
           // ManageMdbCli(this->ledR, this->ledW, modbusTCPClient, &IPs, ipIdx,
           //              Buffer, PrgDevices, Toggles, this->somethingChanged, this->route);
           ManageMdbCli(this->ledR, this->ledW, modbusTCPClient, &IPs, ipIdx, Buffer, PrgDevices, Toggles,
                         &DomoManager::SomethingChangedWrapper, 
                         &DomoManager::RouteWrapper);
        }

        bool restartIP = true;
        for (short i = 0; i < IPs.getSize(); i++) {
            if (!(IPs.get(i).InError && IPs.get(i).Errors > 5)) {
                restartIP = false;
                break;
            }
        }

        if (restartIP) {
            NVIC_SystemReset();
        } else {
            if (ipIdx < IPs.getSize() - 1){
                ipIdx++;
            }
            else {
                ipIdx = 0;
            
                if ((millis() - _lastPnlPoll >= PNL_POLL)) {
                    ManageMdbSvr(this->ledPnl,client, modbusTCPServer, Buffer, Toggles, "Server 01", _rw);
                    _rw = !_rw;
                    _lastPnlPoll = millis();
                } else {
                    // ---- TIMED CALLBACKS ----
                    timings.activityLoop.last     = Measure([&]() { this->activityLoop(Buffer); });
                    UpdateTiming(timings.activityLoop,     timings.activityLoop.last,     timings.spikeThresholdFactor);

                    //Se sono variati
                    if(this->system.hasChanged()) {
                        Buffer.WriteElement(AREA_SYSTEM_FLAGS, ToPanel, this->system.getBitmask());
                    } 
                }
            }

            int _errors = DeviceHasErrors(PrgDevices);
            digitalWrite(this->ledErr, _errors > 0);
            Buffer.WriteElement(this->areaErrors, ToPanel, _errors);
            this->system.set(SystemManager::DEVICES_IN_ALLARME, _errors > 0);
        }

        static unsigned long lastWatchdogCheck = 0;
        if (millis() - lastWatchdogCheck >= 1000) {   // controlla ogni 1s
            CheckWatchdog();
            lastWatchdogCheck = millis();

            Buffer.WriteElement(this->areaRunningT, ToPanel,timings.updateCycle.last);
        }
        else {
            //Aggiorna i dati del watchdog di loop
            unsigned long _exec = millis() - _runningT; 
            UpdateTiming(timings.updateCycle, _exec, timings.spikeThresholdFactor);
        }
    }

    void addDevice(const char* name, arduino::IPAddress ip, unsigned int deviceAddress,
                   GenericPrgDevice::GenericPrgDeviceChannel channels[], size_t channelSize,
                   std::vector<int> ioAreas, short ErrorCnt, GenericPrgDevicePriority priority)
    {
        PrgDevices.emplace_back(name, ip, deviceAddress, channels, channelSize, ioAreas, ErrorCnt, priority);
    }

    void DefineBufferElement(int modbusArea, int modbusAreaToWrite, bool WriteToPanel,
                             bool ReadFromPanel, bool Reverse, char* name)
    {
        Buffer.SetElement(modbusArea, modbusAreaToWrite, WriteToPanel, ReadFromPanel, Reverse, name);
    }

    void addToggle(int areaRead, std::vector<int> forwardsFromAreas = std::vector<int>()) {
        Toggles.addToggle(areaRead, forwardsFromAreas);
    }

    void initBuffer() {
        // First 9 Areas are reserved, starts from 10.
        // Diagnostica
        DefineBufferElement(AREA_SYSTEM_ERRORS, 0, true, false, false, "Devices in error");  
        DefineBufferElement(AREA_SYSTEM_RUNNING_T, 0, true, false, false, "Current cycle"); 

        Buffer.Init();

        std::vector<int> neverInit=Buffer.getNeverInitialized();
        if (!neverInit.empty()) { Serial.println(" - Trovate Aree non inizializzate - "); for (int area : neverInit) { Serial.print(" Area: "); Serial.println(area); } }

        std::vector<int> multipleInit=Buffer.getInitializedMultipleTimes();
        if (!multipleInit.empty()) { Serial.println(" - ERRORE Trovate Aree inizializzate piu volte - "); for (int area : multipleInit) { Serial.print(" Area: "); Serial.println(area); } }
    }

    void systemManagerSet(SystemManager::SystemField field, bool value) {
        this->system.set(field, value);
    }

    SystemManager::SystemInfo systemManagerGet() {
        return this->system.getInfo();
    }
};

// ---- Static member definition ---- 
DomoManager* DomoManager::instance = nullptr;

#endif
