#ifndef IOT_H
#define IOT_H

  #include <Arduino.h>
  #include <EthernetUdp.h>
  #include "PLC.h"

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
