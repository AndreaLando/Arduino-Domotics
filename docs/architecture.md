\# Architettura generale



Questo framework di automazione domestica è composto da moduli indipendenti ma perfettamente integrati.

Ogni componente è progettato per essere riutilizzabile, estensibile e non bloccante.



---



\## 🧩 Componenti principali



\### \*\*1. DomoManager\*\*

È il cuore del sistema. Gestisce:

\- polling Modbus TCP (client)

\- server Modbus TCP (pannello)

\- routing tra buffer e dispositivi

\- watchdog con spike detection

\- activity loop utente

\- gestione errori e priorità IP



---



\### \*\*2. ModbusBuffer\*\*

Un buffer circolare avanzato che gestisce:

\- change‑tracking

\- debounce logico

\- reverse logic

\- aree virtuali

\- routing Field → ToPanel → FromPanel

\- forwarding per toggle

\- sincronizzazione pannello ↔ campo



È il “bus interno” del sistema.



---



\### \*\*3. AsyncScheduler\*\*

Scheduler asincrono non bloccante con:

\- step normali

\- branching condizionale

\- skip condition

\- delay post‑step

\- callback finale

\- priorità job



Permette di creare workflow complessi senza bloccare il loop.



---



\### \*\*4. HVAC\*\*

Sistema di climatizzazione multizona con:

\- compressore

\- defrost

\- circolatore

\- ventola intelligente

\- fancoil

\- protezioni (finestra aperta, temperatura esterna)

\- anti‑ciclo ON/OFF



---



\### \*\*5. PowerManager\*\*

Gestione carichi elettrici con:

\- forecast FV (curva solare + lux + temperatura esterna)

\- hysteresis dinamica

\- auto‑tuning

\- suggerimenti automatici (attacca/stacca)

\- modalità ottimizzazione

\- gestione carichi termici



---



\### \*\*6. WeatherStation\*\*

Gestione sensori meteo:

\- media mobile

\- eventi edge‑trigger

\- allarmi con debounce

\- giorno/notte con isteresi

\- pioggia e raffiche di vento



---



\### \*\*7. Sensors (Intrusion)\*\*

Sensori multi‑canale:

\- RT

\- H24

\- MASK

\- LEN



Con:

\- debounce

\- TON

\- latch memoria

\- startup inhibit



---



\### \*\*8. IOT\*\*

Modulo UDP per:

\- comandi remoti

\- stato casa

\- notifiche

\- sincronizzazione app



---



\## 🔗 Interazione tra moduli





Ogni componente è indipendente ma orchestrato da DomoManager.



---



\## 🎯 Obiettivi architetturali



\- Non bloccare mai il loop

\- Gestire centinaia di I/O

\- Essere estensibile e modulare

\- Garantire stabilità anche in caso di errori Modbus

\- Supportare logiche avanzate (HVAC, power, intrusion)

