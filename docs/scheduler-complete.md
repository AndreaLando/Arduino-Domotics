\# 🧭 AsyncScheduler – Documentazione Tecnica Completa



L’\*\*AsyncScheduler\*\* è un motore di esecuzione non bloccante progettato per microcontrollori.

Permette di creare workflow complessi tramite:



\- step sequenziali

\- branching condizionale

\- skip condition

\- delay post‑step

\- callback finale

\- job multipli con priorità

\- funzioni non bloccanti



È il cuore delle automazioni avanzate del framework.



---



\# 📘 INDICE



1\. Panoramica generale

2\. Flusso interno (diagramma)

3\. API Reference completa

4\. Guida alla progettazione e tuning



---



\# 1️⃣ PANORAMICA GENERALE



\## Obiettivo dello Scheduler



\- eseguire sequenze di operazioni senza bloccare il loop

\- gestire condizioni dinamiche

\- supportare branching complesso

\- permettere automazioni robuste e leggibili

\- evitare `delay()` e blocchi CPU



---



\## Tipi di Step



\### 🔹 NORMAL\_STEP

Esegue una funzione non bloccante:



```cpp

bool fnc(DomoManager\\\* dm);

```



\- ritorna `false` → step ancora in esecuzione

\- ritorna `true` → step completato



---



\### 🔹 BRANCH\_STEP

Valuta una condizione:



```cpp

bool condition(DomoManager\\\* dm);

```



\- `true`  → salta a `thenStep`

\- `false` → salta a `elseStep`



---



\## Skip Condition



Ogni step può essere saltato:



```cpp

step.skipIf = myCondition;

```



---



\## Delay post‑step



```cpp

step.delayAfterMs = 1000;

```



---



\## Callback finale



```cpp

job.onComplete = myCallback;

```



---



\## Priorità Job



I job vengono eseguiti in ordine di priorità decrescente.



---



\# 2️⃣ FLUSSO INTERNO (SCHEDULER-FLOW)



\## Diagramma del ciclo `run()`



```

┌────────────────────────────────┐

│ 1. Selezione job attivo  		      │

│    - priorità         		         │

│    - stato            		         │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 2. Lettura step corrente       │

│    - NORMAL\\\_STEP               │

│    - BRANCH\\\_STEP               │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 3. Skip condition?             │

│    - sì → salta step           │

│    - no → esegui               │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 4. Esecuzione step             │

│    - fnc() non bloccante       │

│    - condition() per branch    │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 5. Delay post-step             │

│    - attesa non bloccante      │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 6. Avanzamento indice          │

│    - nextStep                  │

│    - thenStep / elseStep       │

└───────────────┬────────────────┘

\&nbsp;               ▼

┌────────────────────────────────┐

│ 7. Fine job?                   │

│    - sì → callback finale      │

│    - no → continua             │

└────────────────────────────────┘

```



---



\# 3️⃣ API REFERENCE COMPLETA



\## Strutture principali



\### Step



```cpp

struct Step {

\&nbsp;   StepType type;              // NORMAL\\\_STEP o BRANCH\\\_STEP

\&nbsp;   bool (\\\*fnc)(DomoManager\\\*);  // per NORMAL\\\_STEP

\&nbsp;   bool (\\\*condition)(DomoManager\\\*); // per BRANCH\\\_STEP

\&nbsp;   int thenStep;

\&nbsp;   int elseStep;

\&nbsp;   int delayAfterMs;

\&nbsp;   bool (\\\*skipIf)(DomoManager\\\*);

\&nbsp;   String description;

};

```



---



\### Job



```cpp

struct Job {

\&nbsp;   int priority;

\&nbsp;   std::vector<Step> steps;

\&nbsp;   void (\\\*onComplete)();

};

```



---



\## Creazione Job



```cpp

AsyncScheduler::Job job;

job.priority = 10;

job.onComplete = myCallback;

```



---



\## Aggiunta Step



\### NORMAL\_STEP



```cpp

AsyncScheduler::Step s;

s.type = AsyncScheduler::NORMAL\\\_STEP;

s.fnc = myStepFunction;

s.delayAfterMs = 500;

job.steps.push\\\_back(s);

```



---



\### BRANCH\_STEP



```cpp

AsyncScheduler::Step b;

b.type = AsyncScheduler::BRANCH\\\_STEP;

b.condition = myCondition;

b.thenStep = 3;

b.elseStep = 1;

job.steps.push\\\_back(b);

```



---



\## Registrazione Job



```cpp

int id = scheduler.addJob(job);

scheduler.startJob(id);

```



---



\## Esecuzione Scheduler



Da chiamare nel loop:



```cpp

scheduler.run();

```



---



\# 4️⃣ GUIDA ALLA PROGETTAZIONE E TUNING



\## Scrivere funzioni non bloccanti



Esempio corretto:



```cpp

bool step(DomoManager\\\* dm) {

\&nbsp;   static unsigned long t = 0;

\&nbsp;   if (t == 0) t = millis();



\&nbsp;   if (millis() - t >= 2000) {

\&nbsp;       t = 0;

\&nbsp;       return true;

\&nbsp;   }

\&nbsp;   return false;

}

```



Esempio sbagliato:



```cpp

delay(2000);  // BLOCCA TUTTO

return true;

```



---



\## Quando usare BRANCH\_STEP



\- logiche condizionali

\- percorsi alternativi

\- automazioni intelligenti



Esempio:



```

Step 0 → Branch (isHot)

\&nbsp; true  → Step 2 (coolDown)

\&nbsp; false → Step 1 (heatUp)

```



---



\## Quando usare skipIf



\- condizioni temporanee

\- bypass di step non necessari

\- ottimizzazione di workflow



---



\## Delay post‑step



Usalo per:



\- stabilizzare transizioni

\- evitare rimbalzi logici

\- sincronizzare con hardware



---



\## Best Practices



\- mantieni gli step piccoli

\- evita logiche complesse dentro un singolo step

\- usa descrizioni per debug

\- usa callback finale per notifiche

\- usa priorità per job critici



---



\# 📌 Fine documento

