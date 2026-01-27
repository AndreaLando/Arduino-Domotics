\# ModbusBuffer Module



Gestore avanzato di buffer Modbus per framework di automazione embedded.  

Fornisce astrazione, tracciamento modifiche, storicizzazione minima e sincronizzazione pannello ↔ logica.



---



\## 📌 Caratteristiche principali



\- Gestione di più tipi di dato per area Modbus  

\- Tracciamento automatico delle modifiche (`changed`)  

\- Memorizzazione valore precedente + timestamp  

\- Supporto lettura/scrittura pannello  

\- Rilevamento aree non inizializzate o inizializzate più volte  

\- API semplici e non bloccanti  

\- Pensato per microcontrollori e sistemi embedded



---



\# 🔄 Funzionamento del Modulo



Il modulo `ModbusBuffer` funge da livello di astrazione tra la logica applicativa e il protocollo Modbus.  

Il suo compito è garantire che ogni valore proveniente o destinato al pannello venga:



\- memorizzato in modo coerente  

\- confrontato con il valore precedente  

\- marcato come “cambiato” quando necessario  

\- reso disponibile per la sincronizzazione Modbus  

\- tracciato nel tempo (timestamp)  

\- gestito per tipo (stato, comando, valore analogico, ecc.)



---



\# ModbusBuffer — Documentazione dei Metodi



Questo documento descrive in modo completo e dettagliato tutti i metodi pubblici della classe `ModbusBuffer`.  

Ogni metodo include: scopo, parametri, comportamento, note operative ed esempi.



---



\# 📌 Costruttore



\## `ModbusBuffer(unsigned int items)`



\### Scopo

Crea un buffer Modbus composto da un numero definito di aree.



\### Parametri

| Nome | Tipo | Descrizione |

|------|------|-------------|

| `items` | unsigned int | Numero totale di aree Modbus gestite |



\### Comportamento

\- Alloca un array di `ModbusBufferInfo`

\- Inizializza il tracker interno

\- Prepara la struttura dati per i valori



---



\# ⚙️ Configurazione



\## `void SetElement(int modbusArea, int modbusAreaToWrite, bool WriteToPanel, bool ReadFromPanel, bool Reverse, char\* name)`



\### Scopo

Configura una singola area Modbus.



\### Parametri

| Nome | Tipo | Descrizione |

|------|------|-------------|

| `modbusArea` | int | Indice dell’area da configurare |

| `modbusAreaToWrite` | int | Area Modbus di destinazione per la scrittura |

| `WriteToPanel` | bool | Abilita scrittura verso pannello |

| `ReadFromPanel` | bool | Abilita lettura dal pannello |

| `Reverse` | bool | Inverte il valore logico |

| `name` | char\* | Nome simbolico dell’area |



\### Comportamento

\- Registra l’inizializzazione nel tracker

\- Imposta proprietà dell’area

\- Prepara l’area per l’aggiunta dei tipi



---



\## `void AddType(int modbusArea, long initialValue, ModbusBufferFlagType type)`



\### Scopo

Aggiunge un nuovo tipo di dato all’area.



\### Parametri

| Nome | Tipo | Descrizione |

|------|------|-------------|

| `modbusArea` | int | Area Modbus |

| `initialValue` | long | Valore iniziale |

| `type` | ModbusBufferFlagType | Tipo del dato |



\### Comportamento

\- Crea un nuovo `BufferSourceInfo`

\- Imposta `prevValue = 0`

\- Imposta `changed = true`

\- Registra timestamp



---



\# 🔄 Inizializzazione Lettura Pannello



\## `void Init()`



\### Scopo

Costruisce la lista delle aree da leggere dal pannello.



\### Comportamento

\- Conta le aree con `ReadFromPanel = true`

\- Alloca dinamicamente l’array `\_toPanelRead.itemsPtr`

\- Popola la lista con gli indici delle aree leggibili



---



\# ✏️ Scrittura dei Valori



\## `bool WriteElement(int modbusArea, ModbusBufferFlagType type, long value)`

\## `bool WriteElement(int modbusArea, ModbusBufferFlagType type, long value, bool silent)`



\### Scopo

Scrive o aggiorna un valore nel buffer.



\### Parametri

| Nome | Tipo | Descrizione |

|------|------|-------------|

| `modbusArea` | int | Area Modbus |

| `type` | ModbusBufferFlagType | Tipo del dato |

| `value` | long | Valore da scrivere |

| `silent` | bool | Se true, non imposta `changed` |



\### Comportamento

\- Se il tipo non esiste → lo crea

\- Se esiste e il valore cambia → aggiorna e imposta `changed = true`

\- Se esiste e il valore è uguale → nessuna modifica

\- Se `silent = true` → non genera evento di cambiamento



---



\# 🔍 Lettura e Confronto



\## `int Compare(int modbusArea, ModbusBufferFlagType type, long value)`



\### Scopo

Confronta un valore esterno con quello memorizzato.



\### Ritorna

| Valore | Significato |

|--------|-------------|

| `-1` | Errore |

| `0` | Tipo non trovato |

| `1` | Valore diverso |

| `2` | Valore uguale |



---



\## `bool GetData(int modbusArea, ModbusBufferFlagType type, BufferSourceInfo \&dataOut)`



\### Scopo

Recupera un valore dal buffer.



\### Comportamento

\- Se il tipo esiste → copia i dati in `dataOut`

\- Se non esiste → ritorna valori fittizi



---



\# 🚨 Gestione dei Cambiamenti



\## `bool HasChanged(int modbusArea, ModbusBufferFlagType type)`



\### Scopo

Verifica se un valore è cambiato.



---



\## `void SetChangeFlag(int modbusArea, ModbusBufferFlagType type, bool value)`



\### Scopo

Modifica manualmente il flag `changed`.



---



\## `int getChanged(ModbusBufferItemInfo2\* items, ModbusBufferFlagType type, bool preserveChanges=false)`



\### Scopo

Recupera tutti gli elementi modificati.



\### Parametri

| Nome | Tipo | Descrizione |

|------|------|-------------|

| `items` | ModbusBufferItemInfo2\* | Array di output |

| `type` | ModbusBufferFlagType | Tipo da filtrare |

| `preserveChanges` | bool | Se false, resetta `changed` |



\### Comportamento

\- Scansiona tutte le aree

\- Raccoglie gli elementi con `changed = true`

\- Se `preserveChanges = false` → resetta il flag



---



\# 🧭 Utility



\## `char\* GetName(int modbusArea)`



Ritorna il nome simbolico dell’area.



---



\## `int GetAreaToWrite(int modbusArea)`



Ritorna l’area Modbus di destinazione.



---



\## `bool IsReverse(int modbusArea)`



Ritorna se l’area ha inversione logica.



---



\## `bool CanReadFromPanel(int modbusArea)`

\## `bool CanWriteToPanel(int modbusArea)`



Ritorna le capacità dell’area.



---



\## `ModbusBufferArrayInfo GetToReadFromPanel()`



Ritorna la lista delle aree leggibili dal pannello.



---



\# 🧪 Tracker



\## `std::vector<int> getNeverInitialized()`



Ritorna le aree mai inizializzate.



---



\## `std::vector<int> getInitializedMultipleTimes()`



Ritorna le aree inizializzate più volte.



---



\# 📘 Conclusione



Questo documento fornisce una descrizione completa e dettagliata di tutti i metodi della classe `ModbusBuffer`, utile per sviluppo, debugging e manutenzione del framework di automazione.







