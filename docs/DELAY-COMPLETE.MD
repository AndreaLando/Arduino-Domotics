\# ⏱️ Delay / Timing Engine – Documentazione Tecnica Completa



Il modulo \*\*Delay / Timing Engine\*\* fornisce funzioni di temporizzazione non bloccanti utilizzate da tutto il framework:



\- TON (Timer ON Delay)  

\- TOFF (Timer OFF Delay)  

\- ritardi non bloccanti  

\- edge detection (rising/falling)  

\- timer ciclici  

\- timer one‑shot  

\- funzioni di debounce temporale  



È progettato per essere \*\*ultra‑leggero\*\*, \*\*deterministico\*\* e \*\*perfetto per microcontrollori\*\*.



---



\# 📘 INDICE



1\. Panoramica generale  

2\. Flusso interno (diagramma)  

3\. API Reference completa  

4\. Best practices e tuning  



---



\# 1️⃣ PANORAMICA GENERALE



\## Obiettivi del modulo Delay



\- sostituire completamente `delay()`  

\- fornire temporizzazioni affidabili  

\- evitare blocchi del loop  

\- permettere logiche complesse basate sul tempo  

\- essere integrato in PowerManager, HVAC, Sensors, Scheduler, IOT  



---



\## Funzionalità principali



\### 🔹 TON (ritardo all’attivazione)

Attiva un segnale solo se rimane TRUE per un certo tempo.



\### 🔹 TOFF (ritardo allo spegnimento)

Mantiene TRUE un segnale anche dopo che è tornato FALSE.



\### 🔹 Delay non bloccante

Simile a `delay()`, ma senza bloccare il loop.



\### 🔹 Edge detection

\- rising edge  

\- falling edge  

\- toggle detection  



\### 🔹 Timer ciclici

Esecuzione periodica di funzioni.



\### 🔹 Timer one‑shot

Esecuzione singola dopo un ritardo.



---



\# 2️⃣ FLUSSO INTERNO (DELAY-FLOW)



\## Diagramma del ciclo `update()`



```

┌────────────────────────────────┐

│ 1. Lettura stato input         │

│    - valore attuale            │

│    - valore precedente         │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 2. Edge detection              │

│    - rising                    │

│    - falling                   │

│    - toggle                    │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 3. TON                         │

│    if input==TRUE              │

│       start timer              │

│       if elapsed >= ton        │

│           output = TRUE        │

│    else reset timer            │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 4. TOFF                        │

│    if input==FALSE             │

│       start timer              │

│       if elapsed >= toff       │

│           output = FALSE       │

│    else keep TRUE              │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 5. Delay non bloccante         │

│    - check elapsed             │

│    - return true/false         │

└────────────────────────────────┘

```



---



\# 3️⃣ API REFERENCE COMPLETA



\## Classe TON



```cpp

class TON {

public:

&nbsp;   TON(unsigned long delayMs);

&nbsp;   void setDelay(unsigned long ms);

&nbsp;   bool update(bool input);

&nbsp;   void reset();

};

```



\### Funzionamento



```cpp

if (ton.update(input)) {

&nbsp;   // attivato dopo delay

}

```



---



\## Classe TOFF



```cpp

class TOFF {

public:

&nbsp;   TOFF(unsigned long delayMs);

&nbsp;   void setDelay(unsigned long ms);

&nbsp;   bool update(bool input);

&nbsp;   void reset();

};

```



---



\## Delay non bloccante



```cpp

bool delayNonBlocking(unsigned long ms);

```



Esempio:



```cpp

if (delayNonBlocking(2000)) {

&nbsp;   // 2 secondi trascorsi

}

```



---



\## Edge detection



```cpp

class Edge {

public:

&nbsp;   bool rising(bool input);

&nbsp;   bool falling(bool input);

&nbsp;   bool toggle(bool input);

};

```



---



\## Timer ciclico



```cpp

class CycleTimer {

public:

&nbsp;   CycleTimer(unsigned long period);

&nbsp;   bool tick(); // TRUE ogni periodo

};

```



---



\## Timer one‑shot



```cpp

class OneShot {

public:

&nbsp;   OneShot(unsigned long delay);

&nbsp;   bool trigger(); // avvia

&nbsp;   bool done();    // TRUE quando completato

};

```



---



\# 4️⃣ BEST PRACTICES E TUNING



\## TON consigliati



| Uso | TON |

|-----|-----|

| Debounce pulsanti | 30–80 ms |

| Sensori magnetici | 50–150 ms |

| PIR | 200–500 ms |

| Allarmi | 100–300 ms |



---



\## TOFF consigliati



| Uso | TOFF |

|-----|------|

| Ventole | 30–90 s |

| Circolatori | 20–60 s |

| Carichi termici | 60–180 s |



---



\## Delay non bloccante



Usare quando:



\- serve un ritardo semplice  

\- non si vuole bloccare il loop  

\- si vuole sincronizzare operazioni  



---



\## Edge detection



Usare per:



\- pulsanti  

\- sensori digitali  

\- trigger di automazioni  

\- avvio job scheduler  



---



\## Timer ciclici



Esempi:



\- aggiornamento sensori ogni 1s  

\- invio dati IOT ogni 5s  

\- log periodici  



---



\## Timer one‑shot



Esempi:



\- ritardi di sicurezza  

\- attivazioni temporizzate  

\- reset automatici  



---



\## Consigli pratici



\- evitare TON/TOFF troppo brevi → instabilità  

\- evitare TON/TOFF troppo lunghi → lentezza  

\- usare edge detection per evitare rimbalzi  

\- preferire delay non bloccante a `delay()`  

\- usare timer ciclici per task periodici  



---



\# 📌 Fine documento



