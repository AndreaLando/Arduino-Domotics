\# 🏁 Initialization – Documentazione Tecnica Completa



Il modulo \*\*Initialization\*\* gestisce l’avvio dell’intero framework:



\- inizializzazione dei moduli principali  

\- caricamento configurazioni  

\- setup di callback e dipendenze  

\- avvio dei manager (Power, HVAC, Scheduler, IOT…)  

\- gestione dello stato di boot  

\- diagnostica iniziale  



È progettato per garantire un \*\*avvio coerente, ordinato e sicuro\*\* dell’intero sistema.



---



\# 📘 INDICE



1\. Panoramica generale  

2\. Flusso interno (diagramma)  

3\. API Reference completa  

4\. Best practices e tuning  



---



\# 1️⃣ PANORAMICA GENERALE



\## Obiettivi del modulo Initialization



\- garantire che tutti i moduli vengano inizializzati nell’ordine corretto  

\- evitare dipendenze non pronte  

\- impostare callback e collegamenti tra moduli  

\- eseguire diagnostica iniziale  

\- preparare il sistema allo stato operativo  



---



\## Moduli inizializzati



Il modulo Initialization prepara:



\- \*\*DomoManager\*\*  

\- \*\*AsyncScheduler\*\*  

\- \*\*PowerManager\*\*  

\- \*\*HVAC\*\*  

\- \*\*WeatherStation\*\*  

\- \*\*Sensors (Intrusion)\*\*  

\- \*\*ModbusBuffer\*\*  

\- \*\*IOT\*\*  

\- \*\*Logging\*\*  

\- \*\*Timers e TON\*\*  



---



\## Stati di avvio



Il sistema attraversa:



1\. \*\*BOOT\_START\*\*  

2\. \*\*MODULE\_INIT\*\*  

3\. \*\*LINKING\_CALLBACKS\*\*  

4\. \*\*SELF\_TEST\*\*  

5\. \*\*READY\*\*  



---



\# 2️⃣ FLUSSO INTERNO (INITIALIZATION-FLOW)



\## Diagramma del ciclo `InitializeSystem()`



```

┌────────────────────────────────┐

│ 1. BOOT\_START                  │

│    - reset variabili           │

│    - diagnostica base          │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 2. MODULE\_INIT                 │

│    - WeatherStation            │

│    - PowerManager              │

│    - HVAC                      │

│    - Sensors                   │

│    - Modbus                    │

│    - IOT                       │

│    - Scheduler                 │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 3. LINKING\_CALLBACKS           │

│    - Power → callbacks         │

│    - HVAC → fancoil/compressore│

│    - Sensors → eventi          │

│    - IOT → comandi remoti      │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 4. SELF\_TEST                   │

│    - verifica sensori          │

│    - verifica rete             │

│    - verifica moduli           │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 5. READY                       │

│    - sistema operativo         │

│    - log “System Ready”        │

└────────────────────────────────┘

```



---



\# 3️⃣ API REFERENCE COMPLETA



\## Funzione principale



```cpp

void InitializeSystem();

```



Esegue l’intera sequenza di avvio.



---



\## Stato di inizializzazione



```cpp

InitializationState getInitializationState();

```



Stati possibili:



```cpp

enum InitializationState {

&nbsp;   BOOT\_START,

&nbsp;   MODULE\_INIT,

&nbsp;   LINKING\_CALLBACKS,

&nbsp;   SELF\_TEST,

&nbsp;   READY

};

```



---



\## Setup moduli



\### WeatherStation



```cpp

initWeatherStation();

```



\### PowerManager



```cpp

initPowerManager();

```



\### HVAC



```cpp

initHVAC();

```



\### Sensors



```cpp

initSensors();

```



\### Modbus



```cpp

initModbus();

```



\### IOT



```cpp

initIOT();

```



\### Scheduler



```cpp

initScheduler();

```



---



\## Callback linking



```cpp

linkPowerCallbacks();

linkHVACCallbacks();

linkSensorCallbacks();

linkIOTCallbacks();

```



---



\## Self test



```cpp

bool runSelfTest();

```



Verifica:



\- sensori ambientali  

\- rete  

\- moduli attivi  

\- memoria  

\- timer  



---



\## Logging



```cpp

logInitializationStep("PowerManager OK");

```



---



\# 4️⃣ BEST PRACTICES E TUNING



\## Ordine di inizializzazione



L’ordine è \*\*fondamentale\*\*:



1\. Weather  

2\. Sensors  

3\. Power  

4\. HVAC  

5\. Modbus  

6\. IOT  

7\. Scheduler  



---



\## Self-test consigliati



\- lettura sensori → valori plausibili  

\- rete → IP valido  

\- modbus → almeno un dispositivo risponde  

\- power → netPower calcolabile  

\- hvac → temperatura interna valida  



---



\## Logging



Attivare log verbose in fase di sviluppo:



```

\[INIT] Weather OK

\[INIT] Sensors OK

\[INIT] PowerManager OK

\[INIT] HVAC OK

...

```



---



\## Error handling



Se un modulo fallisce:



\- log errore  

\- fallback a modalità sicura  

\- evitare crash del sistema  



---



\## Consigli pratici



\- non inizializzare moduli dentro `setup()` → usare `InitializeSystem()`  

\- evitare dipendenze circolari tra moduli  

\- usare callback per collegare moduli tra loro  

\- eseguire self-test prima di dichiarare READY  



---



\# 📌 Fine documento



