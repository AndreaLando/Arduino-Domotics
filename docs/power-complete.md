\# ⚡ PowerManager – Documentazione Tecnica Completa



Il \*\*PowerManager\*\* è il modulo responsabile della gestione energetica avanzata:



\- controllo carichi elettrici con priorità  

\- forecast fotovoltaico (FV)  

\- hysteresis dinamica  

\- auto‑tuning dei parametri  

\- suggerimenti automatici (attacca/stacca)  

\- gestione carichi termici (pompa di calore, boiler, ecc.)  

\- integrazione con sensori ambientali (lux, temperatura esterna)  



È progettato per essere \*\*reattivo\*\*, \*\*non bloccante\*\* e \*\*ottimizzato per microcontrollori\*\*.



---



\# 📘 INDICE



1\. Panoramica generale  

2\. Flusso interno (diagramma)  

3\. API Reference completa  

4\. Guida alla calibrazione (tuning)  



---



\# 1️⃣ PANORAMICA GENERALE



\## Obiettivi del PowerManager



\- massimizzare autoconsumo FV  

\- evitare superamento limite di potenza contrattuale  

\- ottimizzare comfort e consumi  

\- gestire carichi in base a priorità e condizioni ambientali  

\- fornire suggerimenti intelligenti all’utente  



---



\## Tipi di carichi gestiti



\### 🔌 Carichi elettrici normali

\- lavatrice  

\- asciugatrice  

\- lavastoviglie  

\- forno  

\- wallbox EV  

\- boiler elettrico  



Ogni carico ha:

\- potenza nominale  

\- priorità  

\- minOn / minOff  

\- hysteresis  



---



\### 🔥 Carichi termici

\- pompa di calore  

\- fancoil  

\- boiler ACS  

\- resistenze  



Gestiti tramite:

\- setpoint  

\- isteresi termica  

\- temperatura interna/esterna  

\- logiche anti‑ciclo  



---



\## Forecast FV



Basato su:

\- curva solare teorica  

\- lux  

\- temperatura esterna  

\- volatilità lux  

\- correzione a breve termine  



---



\## Suggerimenti automatici



Esempi:



```

attacca:Lavatrice

stacca:Forno

ritarda:Asciugatrice

```



Ogni suggerimento ha:

\- severità  

\- motivazione  

\- timestamp  



---



\## Modalità di ottimizzazione



\- MASSIMO\_AUTOCONSUMO  

\- RISPARMIO\_ECONOMICO  

\- MASSIMO\_COMFORT  

\- PROTEZIONE\_RETE  

\- BILANCIATO  



---



\# 2️⃣ FLUSSO INTERNO (POWER-FLOW)



\## Diagramma del ciclo `updateLoads()` + `updateThermalControl()`



```

┌────────────────────────────────┐

│ 1. Lettura potenze             │

│    - gridPower                 │

│    - solarPower                │

│    - netPower                  │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 2. Forecast FV                 │

│    - curva solare              │

│    - lux                       │

│    - volatilità                │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 3. Analisi margine disponibile │

│    - surplus FV                │

│    - rischio superamento       │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 4. Gestione carichi elettrici  │

│    - priorità                   │

│    - hysteresis                 │

│    - minOn/minOff               │

│    - auto-tuning                │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 5. Gestione carichi termici    │

│    - setpoint                   │

│    - isteresi                   │

│    - anti-ciclo                 │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 6. Generazione suggerimenti    │

│    - attacca/stacca            │

│    - motivazioni               │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 7. Callback utente             │

│    - onLoadChange              │

│    - onLimitWarning            │

│    - onLimitExceeded           │

│    - onSuggestion              │

└────────────────────────────────┘

```



---



\# 3️⃣ API REFERENCE COMPLETA



\## Costruttore



```cpp

PowerManager pm(

&nbsp;   maxGridPower,

&nbsp;   nominalSolarPower,

&nbsp;   minOnDefault,

&nbsp;   minOffDefault

);

```



---



\## Impostazione potenze



```cpp

void setGridPower(float w);

void setSolarPower(float w);

float getNetGridPower();   // grid - solar

```



---



\## Dati ambientali



```cpp

void setEnvironmentalData(float lux, float tempExt);

```



---



\## Aggiunta carichi elettrici



```cpp

pm.addLoad(

&nbsp;   "Lavatrice",

&nbsp;   PowerManager::Priority::MEDIA,

&nbsp;   1200,   // potenza nominale

&nbsp;   30,     // minOn

&nbsp;   60      // minOff

);

```



---



\## Aggiunta carichi termici



```cpp

pm.addThermalLoad(

&nbsp;   "PompaCalore",

&nbsp;   true,       // abilitato

&nbsp;   21.0,       // setpoint

&nbsp;   19.0,       // minTemp

&nbsp;   23.0,       // maxTemp

&nbsp;   60,         // minOn

&nbsp;   60          // minOff

);

```



---



\## Aggiornamento logiche



```cpp

pm.updateLoads(month, hour, minute);

pm.updateThermalControl(indoorTemp, month, hour, minute);

```



---



\## Auto‑tuning



```cpp

pm.enableAutoTune(true);

pm.autoTuneStep();

```



---



\## Callback



```cpp

pm.setOnLoadChange(\[](const String\& name, bool state){ ... });

pm.setOnLimitWarning(\[](float net, float limit){ ... });

pm.setOnLimitExceeded(\[](float net, float limit){ ... });

pm.setOnSuggestion(\[](const String\& sug, int sev, const String\& reason){ ... });

```



---



\## Modalità ottimizzazione



```cpp

pm.setOptimizationMode(PowerManager::OptimizationMode::MASSIMO\_AUTOCONSUMO);

```



---



\# 4️⃣ GUIDA ALLA CALIBRAZIONE (POWER-TUNING)



\## Limite rete



```

maxGridPower = potenza contrattuale - margine sicurezza

```



Esempio:

\- contratto 3 kW  

\- margine 200 W  

→ limite = 2800 W  



---



\## Carichi elettrici



\### minOn / minOff consigliati



| Carico | minOn | minOff |

|--------|-------|--------|

| Lavatrice | 20–40s | 60–120s |

| Asciugatrice | 60–120s | 120–180s |

| Lavastoviglie | 20–40s | 60–120s |

| Forno | 30–60s | 120–180s |



---



\## Hysteresis carichi



```

hysteresis = 100–300 W

```



Evita oscillazioni ON/OFF rapide.



---



\## Carichi termici



\### Setpoint consigliati



| Stagione | Setpoint | Isteresi |

|----------|----------|----------|

| Inverno | 20–22°C | ±0.5°C |

| Estate | 24–26°C | ±0.5°C |



---



\## Forecast FV



\### Parametri chiave



\- `lux` → stabilità e intensità  

\- `volatilità lux` → nuvole rapide  

\- `tempExt` → efficienza pannelli  



---



\## Suggerimenti automatici



\### Quando attivare



\- surplus FV > 500 W → attacca carichi medi  

\- surplus FV > 1000 W → attacca carichi pesanti  

\- netGridPower > limite → stacca carichi bassi/medi  



---



\## Procedura tuning consigliata



1\. Raccogli dati per 48h  

2\. Analizza cicli carichi  

3\. Regola minOn/minOff  

4\. Regola hysteresis  

5\. Attiva auto‑tuning  

6\. Test in condizioni reali  



---



\# 📌 Fine documento



