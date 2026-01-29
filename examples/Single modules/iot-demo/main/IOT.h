#ifndef IOT_H
#define IOT_H

  #include <Arduino.h>
  #include <EthernetUdp.h>
  #include "PLC.h"
  #include "BaseClass.h"

/* ============================================================
   IOT — UDP-Based Home Automation Communication Module
   ============================================================

   This module provides a lightweight UDP protocol for exchanging
   home automation events and system states between devices.

   Features:
     • Bidirectional UDP messaging
     • Command parsing with string-to-enum mapping
     • Automatic state-change notifications
     • Flooding, intrusion, proximity, and lighting events
     • System status broadcasting
     • Simple key/value protocol using MARKER_ID "::"

   ============================================================
   1. CONSTANTS AND COMMAND DEFINITIONS
   ============================================================

   MARKER_ID = "::"
       Separator between command and value in UDP packets.

   UDP_TIMEOUT = 400 ms
       Timeout for UDP receive operations.

   enum CommandIndex:
       CMD_onArrivoACasaChange
       CMD_onLuciEsterneChange
       CMD_updateProximity
       CMD_Check
       CMD_UNKNOWN

   commands[]
       String array mapping command names to enum indices.

   StatoCasaDesc(int stato)
       Converts numeric home-state codes to text:
         0 → "Fuori Casa"
         1 → "In Casa"
         2 → "In Arrivo"

   ============================================================
   2. CLASS IOT — CONSTRUCTION AND INITIALIZATION
   ============================================================

       IOT(EthernetUDP &udp, IPAddress ip, unsigned int localPort, unsigned int remotePort);

   Initializes:
       _udp        → UDP interface
       _ip         → remote IP
       _localPort  → local listening port
       _remotePort → remote destination port

   begin(message)
       • Sets UDP timeout
       • Begins listening on local port
       • Optionally sends initial system state

   ============================================================
   3. STATE UPDATE FUNCTIONS (SEND ON CHANGE)
   ============================================================

   All functions send a UDP packet only when the value changes.

       void setStatoSistema(String message);
       void setAllarmeAllagamento(bool value);
       void setAllarmeIntrusione(bool value);

   Home arrival state:
       void setArrivoACasa(String message, int value);
         → Appends textual state (Fuori Casa / In Casa / In Arrivo)

   External lights:
       void setLuciEsterne(String message, bool value);
         → Appends "Accese" / "Spente"

   Proximity:
       void setProximity(String message, bool value);
         → Appends "Arrivando" / "Partendo"

   Internal temperature:
       void setTemperaturaMediaInterna(String message, float value);

   GetStatus()
       Returns reference to SystemCmdInfo structure.

   ============================================================
   4. UPDATE LOOP — RECEIVE AND PROCESS UDP PACKETS
   ============================================================

       bool Update();

   Behavior:
       • Reads incoming UDP packet
       • Extracts command and value using MARKER_ID
       • Maps command string to enum via getCommandIndex()
       • Executes corresponding handler:
            CMD_onArrivoACasaChange  → setArrivoACasa()
            CMD_onLuciEsterneChange  → setLuciEsterne()
            CMD_updateProximity      → setProximity()
            CMD_Check                → replies with "Check::Ok"
       • Returns true if a valid command was processed

   Error handling:
       • Prints "Unknown Command" for unmapped commands
       • Prints "UDP Receive error" if packet has no marker

   ============================================================
   5. COMMAND MAPPING
   ============================================================

       CommandIndex getCommandIndex(String input);

   Maps command strings to enum values using the commands[] array.

   ============================================================
   6. UDP SEND HELPERS
   ============================================================

   Overloaded UDPSend() functions:

       void UDPSend(..., String command, int value);
       void UDPSend(..., String command, float value);
       void UDPSend(..., String command, String value);

   Behavior:
       • Builds packet as: command + "::" + value
       • Prints debug log: "IOT Send: <packet>"
       • Sends via UDP to configured IP and port

   ============================================================
   7. PROTOCOL FORMAT
   ============================================================

   Outgoing packet:
       <command>::<value>

   Examples:
       statoSistema::In Casa
       allarmeAllagamento::1
       temperaturaMediaInterna::21.5
       Check::Ok

   Incoming packet examples:
       onArrivoACasaChange::2
       onLuciEsterneChange::1
       updateProximity::0

   ============================================================
   8. DESIGN NOTES
   ============================================================

   • Lightweight and MCU-friendly
   • No dynamic memory allocation
   • All state changes are edge-triggered
   • Simple, human-readable UDP protocol
   • Easily extendable with new commands
   • Debug-friendly with Serial logging

   ============================================================ */

  class SystemCmdInfo {
    public:
        Cell<int>  onArrivoACasaChange;
        Cell<bool> onLuciEsterneChange;
        Cell<bool> onProximity;

        inline bool hasChanged() const {
            return onArrivoACasaChange.hasChanged() ||
                  onLuciEsterneChange.hasChanged() ||
                  onProximity.hasChanged();
        }
  };

  class IOT
  {
    public:
      IOT(EthernetUDP &udp, IPAddress ip, unsigned int localPort, unsigned int remotePort);
      void begin(String message);
      void setStatoSistema(String message);
      void setLuciEsterne(String message, bool value);
      void setProximity(String message, bool value);
      void setArrivoACasa(String message, int value);
      void setAllarmeAllagamento(bool value);
      void setAllarmeIntrusione(bool value);
      void setTemperaturaMediaInterna(String message, float value);
      bool Update();
      SystemCmdInfo& GetStatus();
    private:
      typedef struct {
        bool allarmeIntrusione;
        bool allarmeAllagamento;
        bool allarmeFumo;
        bool finestreAperte;
        bool porteAperte;
        float temperaturaMediaInterna;
        String lastMessage;
      }SystemInfo;
      
      SystemInfo System;
      SystemCmdInfo SystemCmd;
      EthernetUDP *_udp;
      IPAddress _ip;
      unsigned int _localPort;
      unsigned int _remotePort;
  };


#endif
