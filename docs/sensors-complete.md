\# 🚨 Sensors (Intrusion) – Documentazione Tecnica Completa



Il modulo \*\*Sensors (Intrusion)\*\* gestisce sensori multi‑canale per sistemi di sicurezza e automazione:



\- canali RT, H24, MASK, LEN

\- debounce

\- TON (ritardo attivazione)

\- latch memoria

\- startup inhibit

\- allarme combinato

\- change‑tracking per ogni canale



È progettato per essere \*\*robusto\*\*, \*\*reattivo\*\* e \*\*non bloccante\*\*, ideale per sistemi antifurto e logiche di sicurezza.



---



\# 📘 INDICE



1\. Panoramica generale

2\. Flusso interno (diagramma)

3\. API Reference completa

4\. Guida alla calibrazione e tuning



---



\# 1️⃣ PANORAMICA GENERALE



\## Tipi di canale



| Tipo | Nome | Funzione |

|------|------|----------|

| RT | Real Time | Allarme immediato con debounce |

| H24 | 24h | Memoria 24h, resta attivo finché non resettato |

| MASK | Mascheramento | Rileva tentativi di sabotaggio |

| LEN | Logica estesa | Combinazioni avanzate |



---



\## Funzioni principali



\### 🔹 Debounce

Stabilizza ingressi rumorosi.



\### 🔹 TON (ritardo attivazione)

Evita falsi allarmi.



\### 🔹 Latch memoria

Mantiene lo stato anche dopo che l’ingresso torna normale.



\### 🔹 Startup inhibit

Ignora allarmi nei primi secondi dall’avvio.



\### 🔹 Allarme combinato

`alarmOut` diventa TRUE se \*\*qualunque canale\*\* è in allarme.



---



\## Struttura interna



Ogni canale è un oggetto:



```cpp

SensorChannel {

\&nbsp;   int pin;

\&nbsp;   int delay;

\&nbsp;   SensorChannelType type;

\&nbsp;   bool mem;

\&nbsp;   bool raw;

\&nbsp;   bool filtered;

}

```



---



\# 2️⃣ FLUSSO INTERNO (SENSORS-FLOW)



\## Diagramma del ciclo `Run()`



```

┌────────────────────────────────┐

│ 1. Lettura ingressi            │

│    - RT                        │

│    - H24                       │

│    - MASK                      │

│    - LEN                       │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 2. Debounce                    │

│    - stabilizzazione           │

│    - TON                       │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 3. Latch memoria               │

│    - mem = mem || filtered     │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 4. Startup inhibit             │

│    - ignora primi 2s           │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 5. Calcolo allarme             │

│    - OR tra canali             │

│    - alarmOut                  │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 6. Change tracking             │

│    - hasChanged()              │

│    - get()                     │

└────────────────────────────────┘

```



---



\# 3️⃣ API REFERENCE COMPLETA



\## Costruttore



```cpp

Sensor sensore({

\&nbsp;   SensorChannel(pinRT,  RT\\\_DELAY,  SensorChannelType::RT),

\&nbsp;   SensorChannel(pinH24, H24\\\_DELAY, SensorChannelType::H24),

\&nbsp;   SensorChannel(pinMASK,MASK\\\_DELAY,SensorChannelType::MASK),

\&nbsp;   SensorChannel(pinLEN, LEN\\\_DELAY, SensorChannelType::LEN)

});

```



---



\## Abilitazione



```cpp

sensore.Enable(true);

```



Attiva startup inhibit.



---



\## Modalità ENGAGE (latch memoria)



```cpp

sensore.Engage(true);

```



---



\## Esecuzione logica



Da chiamare nel loop:



```cpp

sensore.Run({ rtInput, h24Input, maskInput, lenInput });

```



---



\## Lettura canali



```cpp

auto\\\* rt = sensore.Get(SensorChannelType::RT);

bool stato = rt->filtered;

bool memoria = rt->mem;

```



---



\## Allarme combinato



```cpp

bool allarme = sensore.alarmOut;

```



---



\## Reset memoria



```cpp

sensore.Reset();

```



---



\## Change tracking



Ogni canale usa `Cell<bool>`:



```cpp

if (sensore.Get(SensorChannelType::RT)->cell.hasChanged()) {

\&nbsp;   bool v = sensore.Get(SensorChannelType::RT)->cell.get();

}

```



---



\# 4️⃣ GUIDA ALLA CALIBRAZIONE E TUNING



\## Debounce consigliato



| Ambiente | Debounce |

|----------|----------|

| Pulito | 1–2 |

| Rumoroso | 3–5 |

| Industriale | 5–8 |



---



\## TON (ritardo attivazione)



| Tipo sensore | TON consigliato |

|--------------|-----------------|

| Magnetico | 50–150 ms |

| PIR | 200–500 ms |

| Vibrazione | 300–800 ms |



---



\## Latch memoria



Usare quando:



\- serve registrare eventi brevi

\- serve mantenere allarme fino a reset manuale

\- si vuole compatibilità con centrali antifurto



---



\## Startup inhibit



Valore tipico:



```

2000 ms

```



Evita falsi allarmi all’avvio.



---



\## MASK (anti‑sabotaggio)



Consigli:



\- debounce alto

\- TON medio

\- latch attivo



---



\## LEN (logica estesa)



Usi tipici:



\- combinazioni RT + MASK

\- logiche di presenza

\- sensori multipli in OR/AND



---



\## Procedura tuning consigliata



1\. Raccogli dati grezzi per 1h

2\. Imposta debounce minimo

3\. Aumenta finché spariscono falsi positivi

4\. Imposta TON in base al sensore

5\. Attiva latch se necessario

6\. Testa in condizioni reali



---



\# 📌 Fine documento

