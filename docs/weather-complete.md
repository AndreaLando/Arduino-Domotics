\# 🌦️ WeatherStation – Documentazione Tecnica Completa



La classe \*\*WeatherStation\*\* gestisce sensori ambientali e fornisce:



\- media mobile su finestra circolare

\- eventi edge‑triggered (RainStart, WindGustStart, DayStart…)

\- allarmi con debounce

\- rilevamento giorno/notte con isteresi

\- rilevamento pioggia inizio/fine

\- rilevamento raffiche di vento

\- callback per eventi e allarmi



È progettata per essere \*\*non bloccante\*\*, leggera e affidabile.



---



\# 📘 INDICE



1\. Panoramica generale

2\. Flusso interno (diagramma)

3\. API Reference completa

4\. Guida alla calibrazione (tuning)



---



\# 1️⃣ PANORAMICA GENERALE



\## Sensori gestiti



La classe richiede 4 funzioni di lettura:



```cpp

int readTemp();

int readWind();

int readRain();

int readLight();

```



Devono essere \*\*non bloccanti\*\*.



---



\## Media mobile



Ogni sensore usa una finestra circolare di \*\*10 campioni\*\* per stabilizzare il segnale.



---



\## Eventi edge‑triggered



\- RainStart / RainStop

\- WindGustStart / WindGustEnd

\- DayStart / NightStart



Callback:



```cpp

ws.setEventCallback(\\\[](WeatherEvent e){ ... });

```



---



\## Allarmi edge‑triggered



\- TempLow

\- TempHigh

\- WindHigh

\- RainHigh



Callback:



```cpp

ws.setAlarmCallback(\\\[](const WeatherAlarm\\\*, int){ ... });

```



---



\## Giorno / Notte con isteresi



```

light >= dayThreshold   → giorno

light <= nightThreshold → notte

```



Evita oscillazioni.



---



\## Pioggia (start/stop)



Basato su soglie + debounce:



```

rain >= startThreshold → RainStart

rain <= stopThreshold  → RainStop

```



---



\## Raffiche di vento



Basate su variazione improvvisa:



```

delta = wind - lastWindValue

```



---



\# 2️⃣ FLUSSO INTERNO (WEATHER-FLOW)



\## Diagramma del ciclo `update()`



```

┌──────────────────────────────┐

│ 1. Acquisizione sensori      │

└───────────────┬──────────────┘

\&nbsp;               ▼

┌──────────────────────────────┐

│ 2. Media mobile (buffer)     │

└───────────────┬──────────────┘

\&nbsp;               ▼

┌──────────────────────────────┐

│ 3. Giorno/Notte (isteresi)   │

└───────────────┬──────────────┘

\&nbsp;               ▼

┌──────────────────────────────┐

│ 4. Pioggia start/stop         │

│    (debounce dedicato)        │

└───────────────┬──────────────┘

\&nbsp;               ▼

┌──────────────────────────────┐

│ 5. Raffiche di vento          │

│    (delta + debounce)         │

└───────────────┬──────────────┘

\&nbsp;               ▼

┌──────────────────────────────┐

│ 6. Allarmi (debounce + edge) │

└───────────────┬──────────────┘

\&nbsp;               ▼

┌──────────────────────────────┐

│ 7. Callback eventi/allarmi   │

└──────────────────────────────┘

```



---



\# 3️⃣ API REFERENCE COMPLETA (WEATHER-API)



\## Costruttore



```cpp

WeatherStation ws(

\&nbsp;   readTemp,

\&nbsp;   readWind,

\&nbsp;   readRain,

\&nbsp;   readLight,

\&nbsp;   tFactor,

\&nbsp;   wFactor,

\&nbsp;   rFactor,

\&nbsp;   lFactor,

\&nbsp;   lowT,

\&nbsp;   highT,

\&nbsp;   highW,

\&nbsp;   highR,

\&nbsp;   dayTh,

\&nbsp;   nightTh

);

```



\### Parametri principali



| Parametro | Descrizione |

|----------|-------------|

| `tFactor` | Conversione ADC → °C |

| `wFactor` | Conversione ADC → m/s o km/h |

| `rFactor` | Conversione ADC → mm/h |

| `lFactor` | Conversione ADC → lux |

| `lowT` / `highT` | Soglie allarmi temperatura |

| `highW` | Soglia vento forte |

| `highR` | Soglia pioggia intensa |

| `dayTh` / `nightTh` | Soglie giorno/notte |



---



\## update()



```cpp

void update();

```



Esegue l’intero ciclo di elaborazione.



---



\## Letture filtrate



```cpp

float getTemperature();

float getWind();

float getRain();

float getLight();

```



---



\## Callback eventi



```cpp

void setEventCallback(void (\\\*cb)(WeatherEvent));

```



Eventi:



\- RainStart

\- RainStop

\- WindGustStart

\- WindGustEnd

\- DayStart

\- NightStart



---



\## Callback allarmi



```cpp

void setAlarmCallback(void (\\\*cb)(const WeatherAlarm\\\*, int));

```



Allarmi:



\- TempLow

\- TempHigh

\- WindHigh

\- RainHigh



---



\## Debounce allarmi



```cpp

void setAlarmDebounce(int n);

```



---



\# 4️⃣ GUIDA ALLA CALIBRAZIONE (WEATHER-TUNING)



\## Temperatura



| Parametro | Valore consigliato |

|-----------|--------------------|

| `lowTempThreshold` | 0–5°C |

| `highTempThreshold` | 35–45°C |

| `debounce` | 3–5 |



---



\## Vento



| Parametro | Valore consigliato |

|-----------|--------------------|

| `highWindThreshold` | 20–30 km/h |

| `windGustDelta` | 3–6 |

| `gustDebounce` | 2–3 |



---



\## Pioggia



| Parametro | Valore consigliato |

|-----------|--------------------|

| `rainStartThreshold` | 30–50 |

| `rainStopThreshold` | 10–25 |

| `rainStartDebounce` | 2–4 |

| `rainStopDebounce` | 2–4 |



---



\## Giorno / Notte



| Parametro | Valore consigliato |

|-----------|--------------------|

| `dayThreshold` | 300–500 |

| `nightThreshold` | 150–250 |



Regola fondamentale:



```

nightThreshold < dayThreshold

```



---



\## Debounce allarmi



| Debounce | Effetto |

|----------|---------|

| 1 | molto reattivo, instabile |

| 3 | bilanciato |

| 5+ | molto stabile, meno reattivo |



---



\## Procedura consigliata



1\. Raccogli dati grezzi per 24h

2\. Analizza oscillazioni

3\. Imposta soglie conservative

4\. Regola isteresi

5\. Aumenta debounce finché spariscono falsi positivi

6\. Test in condizioni reali



---



\# 📌 Fine documento

