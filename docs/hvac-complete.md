\# ❄️🔥 HVAC – Documentazione Tecnica Completa



Il modulo \*\*HVAC\*\* gestisce climatizzazione multizona con:



\- compressore

\- ventola intelligente

\- circolatore

\- fancoil per zona

\- defrost automatico

\- protezioni (finestra aperta, temperatura esterna, anti‑ciclo)

\- modalità operative (AUTO, ESTATE, INVERNO, MANUALE…)



È progettato per essere \*\*reattivo\*\*, \*\*non bloccante\*\* e \*\*altamente configurabile\*\*.



---



\# 📘 INDICE



1\. Panoramica generale

2\. Flusso interno (diagramma)

3\. API Reference completa

4\. Guida alla calibrazione e tuning



---



\# 1️⃣ PANORAMICA GENERALE



\## Obiettivi del modulo HVAC



\- mantenere comfort termico

\- ottimizzare consumi

\- proteggere il compressore

\- gestire più zone indipendenti

\- reagire a condizioni ambientali reali

\- supportare modalità automatiche intelligenti



---



\## Componenti principali



\### 🌡 Zona

Ogni zona ha:



\- nome

\- temperatura interna

\- setpoint

\- richiesta caldo/freddo

\- callback fancoil



---



\### ❄️🔥 Pompa di Calore (PDC)

Gestisce:



\- compressore

\- ventola

\- circolatore

\- defrost

\- anti‑ciclo

\- modalità operative



---



\## Modalità operative



\- \*\*SPENTO\*\*

\- \*\*MANUALE\*\*

\- \*\*INVERNO\*\*

\- \*\*ESTATE\*\*

\- \*\*AUTO\*\*

\- \*\*DEFROST\*\*



---



\## Protezioni integrate



\- finestra aperta

\- temperatura esterna fuori range

\- anti‑ciclo ON/OFF

\- post‑circolazione

\- ritardi minimi



---



\# 2️⃣ FLUSSO INTERNO (HVAC-FLOW)



\## Diagramma del ciclo `aggiorna()`



```

┌────────────────────────────────┐

│ 1. Lettura temperature         │

│    - interna                   │

│    - esterna                   │

│    - zone                      │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 2. Valutazione richieste zone  │

│    - caldo/freddo              │

│    - priorità                  │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 3. Modalità operativa          │

│    - AUTO                      │

│    - ESTATE/INVERNO            │

│    - MANUALE                   │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 4. Protezioni                  │

│    - finestra aperta           │

│    - anti-ciclo                │

│    - defrost                   │

│    - temp esterna              │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 5. Gestione compressore        │

│    - ON/OFF                    │

│    - ritardi                   │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 6. Ventola intelligente        │

│    - LOW/MED/HIGH              │

│    - isteresi                  │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 7. Circolatore                 │

│    - ON con compressore        │

│    - post-circolazione         │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 8. Fancoil zone                │

│    - attiva/disattiva          │

│    - callback utente           │

└────────────────────────────────┘

```



---



\# 3️⃣ API REFERENCE COMPLETA



\## Classe Zona



```cpp

Zona::Zona(String nome, float setpoint, int id);

```



\### Metodi principali



```cpp

void aggiornaTemperatura(float t);

float getTemperatura();

float getSetpoint();

bool richiedeCaldo();

bool richiedeFreddo();

```



---



\## Classe PompaDiCalore



\### Costruttore



```cpp

PompaDiCalore pdc(

\&nbsp;   Modalita mode,

\&nbsp;   float setpoint

);

```



---



\## Aggiunta zone



```cpp

pdc.aggiungiZona(\\\&zonaGiorno);

pdc.aggiungiZona(\\\&zonaNotte);

```



---



\## Aggiornamento temperature



```cpp

pdc.aggiornaTemperaturaInterna(temp);

pdc.aggiornaTemperaturaEsterna(tempExt);

```



---



\## Callback



\### Compressore



```cpp

pdc.setCallbackCompressore(\\\[](bool on){ ... });

```



\### Fancoil



```cpp

pdc.setCallbackFancoil(\\\[](String nome, int id, bool stato){ ... });

```



\### Circolatore



```cpp

pdc.setCallbackCircolatore(\\\[](bool on){ ... });

```



---



\## Modalità operative



```cpp

pdc.setModalita(PompaDiCalore::AUTO);

```



---



\## Aggiornamento logica



Da chiamare nel loop:



```cpp

pdc.aggiorna();

```



---



\# 4️⃣ GUIDA ALLA CALIBRAZIONE E TUNING



\## Setpoint consigliati



| Stagione | Setpoint | Isteresi |

|----------|----------|----------|

| Inverno | 20–22°C | ±0.5°C |

| Estate | 24–26°C | ±0.5°C |



---



\## Anti‑ciclo



```

MIN\\\_OFF\\\_MS = 3–5 minuti

MIN\\\_ON\\\_MS  = 2–3 minuti

```



Evita usura compressore.



---



\## Ventola intelligente



\### Soglie consigliate



| ΔT (setpoint - temp) | Velocità |

|----------------------|----------|

| > 2.0°C | HIGH |

| 1.0–2.0°C | MED |

| < 1.0°C | LOW |



---



\## Defrost



Attivato quando:



```

tempEsterna < sogliaDefrost

```



e compressore ON.



---



\## Circolatore



\- ON con compressore

\- post‑circolazione 30–90s

\- anti‑ciclo dedicato



---



\## Fancoil zone



Attivazione quando:



```

zona.richiedeCaldo() || zona.richiedeFreddo()

```



---



\## Procedura tuning consigliata



1\. Imposta setpoint realistici

2\. Regola isteresi ventola

3\. Imposta anti‑ciclo compressore

4\. Testa defrost in condizioni fredde

5\. Verifica comportamento zone

6\. Ottimizza callback fancoil



---



\# 📌 Fine documento

