\# 📡 IOT – Documentazione Tecnica Completa



Il modulo \*\*IOT\*\* gestisce la comunicazione UDP bidirezionale tra il sistema di automazione e un’app o server remoto.  

È progettato per essere:



\- leggero  

\- non bloccante  

\- affidabile  

\- semplice da integrare  

\- compatibile con qualsiasi client UDP  



---



\# 📘 INDICE



1\. Panoramica generale  

2\. Flusso interno (diagramma)  

3\. Protocollo di comunicazione  

4\. API Reference completa  

5\. Guida alla calibrazione e best practices  



---



\# 1️⃣ PANORAMICA GENERALE



\## Obiettivi del modulo IOT



\- inviare notifiche e stati del sistema  

\- ricevere comandi remoti  

\- mantenere sincronizzazione tra app e automazione  

\- fornire un protocollo semplice e leggibile  

\- evitare loop di messaggi grazie al change‑tracking  



---



\## Componenti principali



\### 🔹 SystemInfo  

Contiene lo stato interno del sistema (temperatura, allarmi, messaggi).



\### 🔹 SystemCmdInfo  

Contiene i comandi ricevuti dall’esterno, con change‑tracking tramite `Cell<T>`.



\### 🔹 UDP Engine  

Basato su `EthernetUDP`, gestisce:



\- ricezione pacchetti  

\- parsing  

\- invio messaggi  

\- gestione errori  



---



\# 2️⃣ FLUSSO INTERNO (IOT-FLOW)



\## Diagramma del ciclo `Update()`



```

┌────────────────────────────────┐

│ 1. Ricezione pacchetto UDP     │

│    - buffer                    │

│    - lunghezza                 │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 2. Parsing comando::valore     │

│    - split su "::"             │

│    - validazione               │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 3. Aggiornamento SystemCmdInfo │

│    - hasChanged()              │

│    - set()                     │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 4. Invio eventuale risposta    │

│    - Check::Ok                 │

│    - ack                       │

└───────────────┬────────────────┘

&nbsp;               ▼

┌────────────────────────────────┐

│ 5. Invio messaggi di stato     │

│    - solo se variati           │

│    - change-tracking           │

└────────────────────────────────┘

```



---



\# 3️⃣ PROTOCOLLO DI COMUNICAZIONE



\## Formato messaggio



```

<command>::<value>

```



Esempi:



```

onArrivoACasaChange::1

onLuciEsterneChange::0

updateProximity::1

Check::ping

temperaturaMediaInterna::21.4

```



---



\## Comandi ricevuti (App → Arduino)



| Comando | Valori | Descrizione |

|--------|--------|-------------|

| `onArrivoACasaChange` | 0/1/2 | Stato casa (fuori, in casa, in arrivo) |

| `onLuciEsterneChange` | 0/1 | Accende/spegne luci esterne |

| `updateProximity` | 0/1 | Stato prossimità |

| `Check` | string | Test comunicazione |



---



\## Messaggi inviati (Arduino → App)



| Comando | Tipo | Descrizione |

|--------|------|-------------|

| `statoSistema` | string | Stato generale |

| `allarmeAllagamento` | 0/1 | Allarme acqua |

| `allarmeIntrusione` | 0/1 | Allarme intrusione |

| `temperaturaMediaInterna` | float | Temperatura media |

| `onLuciEsterneChange` | 0/1 | Stato luci esterne |

| `onProximity` | 0/1 | Stato prossimità |

| `onArrivoACasaChange` | 0/1/2 | Stato casa |



Il modulo invia messaggi \*\*solo quando il valore cambia\*\*.



---



\# 4️⃣ API REFERENCE COMPLETA



\## Costruttore



```cpp

IOT iot(

&nbsp;   EthernetUDP\& udp,

&nbsp;   IPAddress remoteIP,

&nbsp;   int localPort,

&nbsp;   int remotePort

);

```



---



\## Avvio



```cpp

iot.begin("Sistema avviato");

```



---



\## Aggiornamento logica



Da chiamare nel loop:



```cpp

bool received = iot.Update();

```



Restituisce `true` se un comando è stato ricevuto.



---



\## Lettura stato comandi



```cpp

auto\& cmd = iot.GetStatus();



if (cmd.onLuciEsterneChange.hasChanged()) {

&nbsp;   bool v = cmd.onLuciEsterneChange.get();

}

```



---



\## Invio messaggi



```cpp

iot.setStatoSistema("OK");

iot.setTemperaturaMediaInterna("Temp:", 21.5);

iot.setLuciEsterne("Luci:", true);

iot.setProximity("Prox:", false);

iot.setArrivoACasa("Casa:", 2);

```



Ogni funzione:



1\. confronta valore precedente  

2\. invia solo se variato  

3\. aggiorna SystemInfo  



---



\## Parsing interno



```cpp

int marker = input.indexOf("::");

String cmd = input.substring(0, marker);

String val = input.substring(marker + 2);

```



---



\# 5️⃣ GUIDA ALLA CALIBRAZIONE E BEST PRACTICES



\## Porte consigliate



```

6000 → locale (Arduino)

6001 → remoto (App)

```



---



\## Frequenza invio messaggi



\- inviare solo valori variati  

\- evitare spam di pacchetti  

\- usare change‑tracking  



---



\## Gestione Check



Client invia:



```

Check::ping

```



Arduino risponde:



```

Check::Ok

```



---



\## Prossimità



Valori:



\- `0` → Partendo  

\- `1` → Arrivando  



Usi tipici:



\- accensione luci ingresso  

\- attivazione riscaldamento  

\- disattivazione allarme  



---



\## Stato casa



Valori:



\- `0` → Fuori casa  

\- `1` → In casa  

\- `2` → In arrivo  



---



\## Debug consigliato



\- loggare pacchetti ricevuti  

\- loggare pacchetti inviati  

\- verificare parsing comando::valore  

\- controllare change‑tracking  



---



\# 📌 Fine documento



