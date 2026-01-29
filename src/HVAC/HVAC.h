#ifndef HVAC_H
#define HVAC_H

#pragma once

#include <Arduino.h>
#include <vector>
#include <functional>

/* INSTRUCTIONS FOR ZONA AND POMPA DI CALORE CLASSES

1. CLASS ZONA
- Represents an individual thermal zone (room or area).
- Stores: name, current temperature, setpoint, and fancoil ID.
- Automatically determines whether the zone requires heating or cooling.
- Heating request: temperature < setpoint − hysteresis.
- Cooling request: temperature > setpoint + hysteresis.
- Update logic is triggered whenever temperature or setpoint changes.
- Provides getters for name, temperature, setpoint, and fancoil number.

2. CLASS POMPA DI CALORE
- Manages the entire heat pump system.
- Handles compressor, fan speed, circulator pump, defrost, safety, and multi-zone logic.
- Supports modes: SPENTO, MANUALE, INVERNO, ESTATE, AUTO, DEFROST.
- Automatically reacts to internal and external temperature updates.

3. SAFETY LOGIC
- Activates when external temperature is outside allowed range.
- Forces compressor off, sets fan to low, activates circulator.
- Clears automatically when temperature returns to safe range.

4. WINDOW LOGIC
- If a window remains open longer than the configured timeout:
  - Compressor forced OFF
  - Fan set to LOW
  - Circulator remains active

5. MODE HANDLING
- SPENTO: system off.
- MANUALE: compressor always on, fan high, circulator active.
- AUTO: evaluates all zones to decide heating or cooling.
- INVERNO/ESTATE: compressor controlled by hysteresis.

6. DEFROST MANAGEMENT
- Triggered when external temperature drops below threshold.
- Compressor forced off, fan medium, circulator active.
- Ends after configured duration.

7. COMPRESSOR PROTECTION
- Enforces minimum OFF time before restart.
- Enforces minimum ON time.
- Prevents rapid cycling and protects hardware.

8. FAN SPEED LOGIC
- Adjusts based on temperature difference from setpoint.
- Uses hysteresis to avoid rapid switching.

9. FANCOIL CONTROL
- Each zone activates its fancoil only when needed.
- Callback triggered on state change.

10. USAGE FLOW
- Create one or more Zona objects.
- Create a PompaDiCalore controller.
- Add zones using aggiungiZona().
- Periodically update:
  - Zone temperatures
  - Internal temperature
  - External temperature
- The controller automatically manages:
  - Compressor
  - Fan speed
  - Circulator pump
  - Defrost cycle
  - Fancoils
  - Safety and window logic
*/

/* ============================================================
   CLASS Zona — Temperature-Controlled Room Zone
   ============================================================

   Represents an individual thermal zone with:
     • Name
     • Current temperature
     • Setpoint
     • Associated fancoil index
     • Heating/cooling requests based on hysteresis

   ------------------------------------------------------------
   1. CONSTRUCTION
   ------------------------------------------------------------

       Zona(String name, float setpoint, int fancoilIndex);

   Initializes:
       nome                    → zone name
       temperatura             → default 20°C
       setpoint                → desired temperature
       numeroFancoil           → associated fancoil ID
       richiestaCaldo          → heating request flag
       richiestaFreddo         → cooling request flag
       statoFancoilPrecedente  → last fancoil state

   ------------------------------------------------------------
   2. TEMPERATURE AND SETPOINT UPDATE
   ------------------------------------------------------------

       void aggiornaTemperatura(float t);
       void setSetpoint(float sp);

   Both functions automatically recalculate:
       • richiestaCaldo
       • richiestaFreddo

   Logic:
       richiestaCaldo  = temperatura < setpoint - ISTERESI
       richiestaFreddo = temperatura > setpoint + ISTERESI

   ------------------------------------------------------------
   3. QUERY FUNCTIONS
   ------------------------------------------------------------

       bool richiedeCaldo();
       bool richiedeFreddo();

       int   getNumeroFancoil();
       float getTemperatura();
       float getSetpoint();
       String getNome();

   Used by the heat pump controller to determine zone needs.

   ============================================================
   CLASS PompaDiCalore — Heat Pump Controller (Multizone)
   ============================================================

   Implements a complete HVAC control system with:

     • Heating / cooling logic
     • Multizone demand evaluation
     • External temperature safety
     • Window-open detection
     • Defrost cycle
     • Intelligent fan speed control
     • Compressor protection (minimum ON/OFF times)
     • Circulator pump management
     • Fancoil activation per zone

   ------------------------------------------------------------
   1. CONSTRUCTION
   ------------------------------------------------------------

       PompaDiCalore(Modalita mode, float setpoint);

   Initializes:
       modalita                 → operating mode (AUTO, INVERNO, ESTATE, etc.)
       setpoint                 → target temperature
       temperaturaAmbiente      → default 20°C
       temperaturaEsterna       → default 10°C
       compressoreAttivo        → compressor state
       velocitaVentola          → LOW by default
       sicurezzaAttiva          → external temp safety
       tempoUltimoCambioStato   → compressor timing
       tempoAccensione          → ON duration tracking
       inizioDefrost            → defrost start time
       finestraAperta           → window state
       tempoInizioFinestra      → window timer
       circolatoreAttivo        → pump state
       tempoUltimoCambioCircolatore → pump timing

   ------------------------------------------------------------
   2. INPUT UPDATES
   ------------------------------------------------------------

       void aggiornaTemperaturaInterna(float t);
       void aggiornaTemperaturaEsterna(float t);
       void aggiungiZona(Zona* z);
       void setFinestraAperta(bool stato);

   Any temperature update triggers a full control cycle.

   ------------------------------------------------------------
   3. MAIN CONTROL LOGIC
   ------------------------------------------------------------

       void controllo();

   Includes:

     • External temperature safety:
         - If outside limits → force compressor OFF, pump ON
         - Auto‑recovery when temperature returns to safe range

     • Window-open logic:
         - After TEMPO_FINESTRA_MS → compressor OFF, fan LOW, pump ON

     • Mode handling:
         SPENTO   → everything OFF
         MANUALE  → compressor ON, fan HIGH, pump ON
         DEFROST  → automatic cycle
         AUTO     → mode chosen based on zone requests

     • Anti‑ON protection:
         - Compressor forced OFF after MAX_TEMPO_ON_MS

     • Heating/cooling logic:
         controlloRiscaldamento()
         controlloRaffrescamento()

     • Intelligent fan speed:
         aggiornaVelocitaVentola()

     • Fancoil activation per zone:
         aggiornaFancoil()

     • Circulator pump:
         gestisciCircolatore()

   ------------------------------------------------------------
   4. HEATING / COOLING LOGIC
   ------------------------------------------------------------

       void controlloRiscaldamento();
       void controlloRaffrescamento();

   Heating:
       temperaturaAmbiente < setpoint - ISTERESI → ON
       temperaturaAmbiente > setpoint + ISTERESI → OFF

   Cooling:
       temperaturaAmbiente > setpoint + ISTERESI → ON
       temperaturaAmbiente < setpoint - ISTERESI → OFF

   ------------------------------------------------------------
   5. MULTIZONE MANAGEMENT
   ------------------------------------------------------------

       void valutaRichiesteZone(bool& caldo, bool& freddo);

   Aggregates zone requests:
       caldo  = true if any zone requires heating
       freddo = true if any zone requires cooling

       void aggiornaFancoil(Modalita effettiva);

   Activates each zone’s fancoil based on:
       • effective mode (INVERNO / ESTATE)
       • zone heating/cooling request
       • previous state (edge-triggered callback)

   ------------------------------------------------------------
   6. FAN SPEED CONTROL
   ------------------------------------------------------------

       void aggiornaVelocitaVentola(Modalita effettiva);
       void setVelocitaVentola(VelocitaVentola v);

   Speed is based on temperature difference:
       LOW / MED / HIGH with hysteresis

   ------------------------------------------------------------
   7. COMPRESSOR CONTROL
   ------------------------------------------------------------

       bool ritardoTrascorso();
       bool puoAccendersi();
       void attivaCompressore();
       void disattivaCompressore();
       void spegniCompressoreForzato();

   Includes:
       • Minimum OFF time (MIN_OFF_MS)
       • Minimum delay between state changes (RITARDO_MINIMO_MS)
       • ON duration tracking (tempoAccensione)

   ------------------------------------------------------------
   8. DEFROST CYCLE
   ------------------------------------------------------------

       void avviaDefrost();
       void gestisciDefrost();

   Triggered when:
       temperaturaEsterna < SOGLIA_DEFROST

   Ends after:
       DURATA_DEFROST_MS

   ------------------------------------------------------------
   9. CIRCULATOR PUMP
   ------------------------------------------------------------

       void gestisciCircolatore();
       void attivaCircolatore();
       void disattivaCircolatore();

   Rules:
       • Compressor ON → pump ON
       • Post‑circulation after compressor OFF
       • Minimum cycle time (MIN_CICLO_CIRCOLATORE_MS)

   ------------------------------------------------------------
   10. GETTERS
   ------------------------------------------------------------

       bool isCompressoreAttivo();
       VelocitaVentola getVelocitaVentola();
       Modalita getModalita();
       float getSetpoint();
       float getTemperaturaInterna();
       float getTemperaturaEsterna();

   ============================================================ */
   
// =========================
//      COSTANTI GLOBALI
// =========================
#define ISTERESI 0.5
#define ISTERESI_VENTOLA 0.3

#define DIFF_LOW 0.5
#define DIFF_MED 1.5

#define SOGLIA_DEFROST 3
#define DURATA_DEFROST_MS 300000

#define MAX_TEMPO_ON_MS 7200000   // 2 ore
#define RITARDO_MINIMO_MS 120000  // 2 minuti

#define MIN_OFF_MS 180000         // 3 minuti anti-ciclo OFF

#define TEMP_ESTERNA_MIN -7
#define TEMP_ESTERNA_MAX 45

#define TEMPO_FINESTRA_MS 15000

#define POST_CIRCOLAZIONE_MS 60000
#define MIN_CICLO_CIRCOLATORE_MS 5000


// =========================
//          ZONA
// =========================
class Zona {
public:
    Zona(String n, float sp, int fancoil);

    void aggiornaTemperatura(float t);
    void setSetpoint(float sp);

    bool richiedeCaldo();
    bool richiedeFreddo();

    int getNumeroFancoil();
    float getTemperatura();
    float getSetpoint();
    String getNome();

    bool statoFancoilPrecedente = false;

private:
    void aggiornaRichieste();

    String nome;
    float temperatura;
    float setpoint;
    int numeroFancoil;

    bool richiestaCaldo;
    bool richiestaFreddo;
};


// =========================
//     POMPA DI CALORE
// =========================
class PompaDiCalore {
public:

    enum Modalita { SPENTO, MANUALE, INVERNO, ESTATE, AUTO, DEFROST };
    enum VelocitaVentola { LOW, MED, HIGH };

    PompaDiCalore(Modalita m, float sp);

    void setModalita(Modalita m);
    void setSetpoint(float sp);

    void aggiornaTemperaturaInterna(float t);
    void aggiornaTemperaturaEsterna(float t);

    void aggiungiZona(Zona* z);

    void setFinestraAperta(bool stato);

    void setCallbackCompressore(std::function<void(bool)> cb) { callbackCompressore = cb; }
    void setCallbackFancoil(std::function<void(String,int,bool)> cb) { callbackFancoil = cb; }
    void setCallbackCircolatore(std::function<void(bool)> cb) { callbackCircolatore = cb; }

    bool isCompressoreAttivo();
    VelocitaVentola getVelocitaVentola();
    Modalita getModalita();
    float getSetpoint();
    float getTemperaturaInterna();
    float getTemperaturaEsterna();

private:
    void controllo();
    void controlloRiscaldamento();
    void controlloRaffrescamento();

    void valutaRichiesteZone(bool &richiestaCaldo, bool &richiestaFreddo);
    void aggiornaFancoil(Modalita effettiva);

    void aggiornaVelocitaVentola(Modalita effettiva);
    void setVelocitaVentola(VelocitaVentola v);

    bool ritardoTrascorso();
    bool puoAccendersi();

    void attivaCompressore();
    void disattivaCompressore();
    void spegniCompressoreForzato();

    void avviaDefrost();
    void gestisciDefrost();

    void gestisciCircolatore();
    void attivaCircolatore();
    void disattivaCircolatore();

    void attivaSicurezza();
    void disattivaSicurezza();

    Modalita modalita;
    float setpoint;

    float temperaturaAmbiente;
    float temperaturaEsterna;

    bool compressoreAttivo;
    VelocitaVentola velocitaVentola;

    bool sicurezzaAttiva;

    unsigned long tempoUltimoCambioStato;
    unsigned long tempoAccensione;

    unsigned long inizioDefrost;

    bool finestraAperta = false;
    unsigned long tempoInizioFinestra = 0;

    bool circolatoreAttivo = false;
    unsigned long tempoUltimoCambioCircolatore = 0;

    std::vector<Zona*> zone;

    std::function<void(bool)> callbackCompressore;
    std::function<void(String,int,bool)> callbackFancoil;
    std::function<void(bool)> callbackCircolatore;
};

#endif
