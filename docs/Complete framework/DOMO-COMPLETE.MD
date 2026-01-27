\# 🧩 DomoManager – Documentazione Tecnica Completa



Il \*\*DomoManager\*\* è il cuore logico dell’intero framework.  

Coordina e collega tutti i moduli:



\- PowerManager  

\- HVAC  

\- Scheduler  

\- Sensors  

\- WeatherStation  

\- Modbus  

\- IOT  

\- Initialization  



È progettato per essere:



\- leggero  

\- centralizzato  

\- estensibile  

\- non bloccante  

\- altamente integrabile  



---



\# 📘 INDICE



1\. Panoramica generale  

2\. Flusso interno (diagramma)  

3\. API Reference completa  

4\. Best practices e tuning  



---



\# 1️⃣ PANORAMICA GENERALE



\## Obiettivi del DomoManager



\- fornire un punto unico di accesso allo stato del sistema  

\- gestire variabili globali e flag di sistema  

\- coordinare moduli indipendenti  

\- fornire un sistema di \*state management\* semplice e robusto  

\- evitare dipendenze circolari tra moduli  

\- permettere automazioni ad alto livello  



---



\## Cosa gestisce il DomoManager



\### 🔹 Stati di sistema

\- allarmi  

\- modalità operative  

\- flag di sicurezza  

\- stato casa (in casa, fuori, in arrivo)  

\- prossimità  

\- override manuali  



---



\### 🔹 Variabili globali

\- temperature  

\- potenze  

\- condizioni meteo  

\- stato rete  

\- stato moduli  



---



\### 🔹 Eventi

\- cambi di stato  

\- trigger per scheduler  

\- notifiche per IOT  



---



\### 🔹 Integrazione moduli

Il DomoManager è il \*\*collante\*\* tra:



\- PowerManager → suggerimenti, limiti, carichi  

\- HVAC → richieste zone, compressore, fancoil  

\- Sensors → intrusioni, mask, H24  

\- Weather → giorno/notte, pioggia, vento  

\- IOT → comandi remoti  

\- Scheduler → job condizionali  



---



\# 2️⃣ FLUSSO INTERNO (DOMO-FLOW)



\## Diagramma del ciclo `update()`



```

┌────────────────────────────────┐

│ 1. Aggiornamento variabili     │

│    - temperature               │

│    - potenze                   │

│    - stati moduli              │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 2. Valutazione condizioni      │

│    - allarmi                   │

│    - stato casa                │

│    - prossimità                │

│    - flags sicurezza           │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 3. Aggiornamento moduli        │

│    - PowerManager              │

│    - HVAC                      │

│    - Sensors                   │

│    - Weather                   │

│    - Scheduler                 │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 4. Trigger eventi              │

│    - change tracking           │

│    - callback IOT              │

│    - callback utente           │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 5. Sincronizzazione stato      │

│    - IOT                       │

│    - log                       │

└────────────────────────────────┘

```



---



\# 3️⃣ API REFERENCE COMPLETA



\## Classe principale



```cpp

class DomoManager {

public:

&nbsp;   void update();

&nbsp;   void reset();

&nbsp;   void systemManagerSet(SystemManager flag, bool value);

&nbsp;   bool systemManagerGet(SystemManager flag);

&nbsp;   void setTemperatureInterna(float t);

&nbsp;   void setTemperatureEsterna(float t);

&nbsp;   void setStatoCasa(int stato);

&nbsp;   void setProximity(bool value);

&nbsp;   ...

};

```



---



\## Gestione flag di sistema



\### Impostazione



```cpp

Manager.systemManagerSet(SystemManager::ALLARME\_INTRUSIONE, true);

```



\### Lettura



```cpp

bool intrusione = Manager.systemManagerGet(SystemManager::ALLARME\_INTRUSIONE);

```



---



\## Stati casa



```cpp

Manager.setStatoCasa(0); // fuori

Manager.setStatoCasa(1); // in casa

Manager.setStatoCasa(2); // in arrivo

```



---



\## Prossimità



```cpp

Manager.setProximity(true);  // arrivando

Manager.setProximity(false); // partendo

```



---



\## Temperature



```cpp

Manager.setTemperatureInterna(21.5);

Manager.setTemperatureEsterna(12.0);

```



---



\## Integrazione con PowerManager



```cpp

float net = Manager.getNetPower();

bool limite = Manager.systemManagerGet(SystemManager::LIMIT\_EXCEEDED);

```



---



\## Integrazione con HVAC



```cpp

float t = Manager.getTemperatureInterna();

bool richiesta = Manager.systemManagerGet(SystemManager::RICHIESTA\_CALDO);

```



---



\## Integrazione con Sensors



```cpp

bool intrusione = Manager.systemManagerGet(SystemManager::ALLARME\_INTRUSIONE);

```



---



\## Integrazione con IOT



```cpp

auto\& cmd = iot.GetStatus();

if (cmd.onArrivoACasaChange.hasChanged()) {

&nbsp;   Manager.setStatoCasa(cmd.onArrivoACasaChange.get());

}

```



---



\## Integrazione con Scheduler



```cpp

if (Manager.systemManagerGet(SystemManager::EVENTO\_SPECIALE)) {

&nbsp;   scheduler.startJob(jobId);

}

```



---



\# 4️⃣ BEST PRACTICES E TUNING



\## 1. Usare SEMPRE il DomoManager come punto centrale



Evita:



❌ moduli che parlano direttamente tra loro  

✔ usa sempre il DomoManager come intermediario  



---



\## 2. Usa flag chiari e documentati



Esempio:



```

ALLARME\_INTRUSIONE  

ALLARME\_ALLAGAMENTO  

RICHIESTA\_CALDO  

RICHIESTA\_FREDDO  

LIMIT\_EXCEEDED  

```



---



\## 3. Evita logiche complesse nei moduli



Le decisioni globali devono stare nel DomoManager.



---



\## 4. Usa change‑tracking per evitare loop



Esempio:



```cpp

if (cmd.onProximity.hasChanged()) {

&nbsp;   Manager.setProximity(cmd.onProximity.get());

}

```



---



\## 5. Mantieni il DomoManager leggero



Non deve:



\- fare calcoli pesanti  

\- contenere logiche di controllo avanzate  

\- bloccare il loop  



---



\## 6. Debug consigliato



Loggare:



\- cambi di stato  

\- flag attivati  

\- eventi generati  

\- input da IOT  



---



\# 📌 Fine documento



