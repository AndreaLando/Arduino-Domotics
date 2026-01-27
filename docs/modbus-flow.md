\# Flusso Modbus



Il sistema utilizza sia un \*\*Modbus TCP Client\*\* (verso periferiche) sia un \*\*Modbus TCP Server\*\* (verso pannello).



---



\# 1. Modbus Client (ManageMdbCli)



\### Flusso:

1\. Connessione al dispositivo Modbus

2\. Lettura canali DI/AI

3\. Scrittura DO/AO

4\. Aggiornamento ModbusBuffer

5\. Routing automatico:

   - toggle

   - forward logic

   - aree virtuali

6\. Callback:

   - somethingChanged

   - route



\### Priorità dinamica

Ogni IP ha:

\- priorità (Low, Medium, Normal, High)

\- rotazione ciclica

\- gestione errori con esclusione temporanea



---



\# 2. Modbus Server (ManageMdbSvr)



\### Modalità alternate:

\- \*\*mode = true\*\* → BUFFER → PANEL

\- \*\*mode = false\*\* → PANEL → BUFFER



\### Funzioni:

\- sincronizzazione pannello

\- gestione comandi utente

\- aggiornamento aree ToPanel

\- lettura aree FromPanel



---



\# 3. Routing interno (ModbusBuffer)



\### Tipi di aree:

\- \*\*Field\*\* → valore reale da/per periferiche

\- \*\*ToPanel\*\* → valori da inviare al pannello

\- \*\*FromPanel\*\* → comandi provenienti dal pannello



\### Funzioni:

\- debounce

\- reverse logic

\- change‑tracking

\- forwarding toggle

\- aree virtuali



---



\# 4. Error Handling



\- retry automatici

\- esclusione temporanea device

\- reset IP se tutti in errore

\- logging spike Modbus

