#include "IOT.h"


const String MARKER_ID="::"; //Separator between command and value
const int UDP_TIMEOUT=400;
enum CommandIndex { CMD_onArrivoACasaChange,  CMD_onLuciEsterneChange, CMD_updateProximity, CMD_Check, CMD_UNKNOWN };

// Define corresponding string array
const String commands[] = { "onArrivoACasaChange", "onLuciEsterneChange", "updateProximity", "Check" };

void UDPSend(EthernetUDP &Udp, IPAddress ip, unsigned int remotePort, String command, int value);
void UDPSend(EthernetUDP &Udp, IPAddress ip, unsigned int remotePort, String command, float value);
void UDPSend(EthernetUDP &Udp, IPAddress ip, unsigned int remotePort, String command, String value);
CommandIndex getCommandIndex(String input);
String StatoCasaDesc(int stato) {
  switch(stato) {
    case 0:
    return "Fuori Casa";
    break;

    case 1:
    return "In Casa";
    break;
    
    case 2:
    return "In Arrivo";
    break;
  }

  return "Unknown";
}

IOT::IOT(EthernetUDP &udp, IPAddress ip, unsigned int localPort, unsigned int remotePort) 
{
  this->_udp=&udp;
  this->_ip=ip;
  this->_localPort=localPort;
  this->_remotePort=remotePort;
}

void IOT::begin(String message) 
{
  this->_udp->setTimeout(UDP_TIMEOUT);
  this->_udp->begin(this->_localPort);

  if(message!="")
    this->setStatoSistema(message);
}
 
void IOT::setStatoSistema(String message) {
  if(System.lastMessage!=message) {
    UDPSend(*this->_udp, this->_ip, this->_remotePort, "statoSistema", message);
    System.lastMessage=message;
  }
};

void IOT::setAllarmeAllagamento(bool value) {
  if(System.allarmeAllagamento!=value) {
    UDPSend(*this->_udp, this->_ip, this->_remotePort, "allarmeAllagamento", value);
    System.allarmeAllagamento=value;
  }
};

void IOT::setAllarmeIntrusione(bool value) {
  if(System.allarmeIntrusione!=value) {
    UDPSend(*this->_udp, this->_ip, this->_remotePort, "allarmeIntrusione", value);
    System.allarmeIntrusione=value;
  }
};


void IOT::setArrivoACasa(String message, int value) {
  message+=" "+StatoCasaDesc(value);
  if(System.lastMessage!=message) {
    UDPSend(*this->_udp, this->_ip, this->_remotePort, "statoSistema", message);
  }
  SystemCmd.onArrivoACasaChange.set(value);
};

void IOT::setLuciEsterne(String message, bool value) {
  String _stato=(value?"Accese":"Spente");
  message+=" "+_stato;
  if(System.lastMessage!=message && SystemCmd.onLuciEsterneChange.preserveGet()!=value) {
    UDPSend(*this->_udp, this->_ip, this->_remotePort, "statoSistema", message);
  }
  SystemCmd.onLuciEsterneChange.set(value);
};

void IOT::setProximity(String message, bool value) {
  String _stato=(value?"Arrivando":"Partendo");
  message+=" "+_stato;
  if(System.lastMessage!=message && SystemCmd.onProximity.preserveGet()!=value) {      
    UDPSend(*this->_udp, this->_ip, this->_remotePort, "statoSistema", message);
  }
  SystemCmd.onProximity.set(value);
};

void IOT::setTemperaturaMediaInterna(String message, float value) {
  if(System.temperaturaMediaInterna!=value) {
    UDPSend(*this->_udp, this->_ip, this->_remotePort, "temperaturaMediaInterna", value);
  }
  System.temperaturaMediaInterna=value;
};

SystemCmdInfo& IOT::GetStatus() {
  return this->SystemCmd;
};

bool IOT::Update() {
 // Receive
  int packetSize = this->_udp->parsePacket();
  if (packetSize) {
    char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
    this->_udp->read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    int _marker=String(packetBuffer).indexOf(MARKER_ID);
    if(_marker!=-1) {
      String _cmd=String(packetBuffer).substring(0, _marker);
      Serial.print("IOT Receive: "+_cmd);
      String _value="";
      switch (getCommandIndex(_cmd)) {
        case CMD_onArrivoACasaChange:
          _value=String(packetBuffer).substring(_marker+2);
          Serial.println(", value: "+_value);
          
          this->setArrivoACasa("Ho variato lo stato di Casa: ", _value.toInt());
          return true;
          break;

        case CMD_onLuciEsterneChange:
          _value=String(packetBuffer).substring(_marker+2);
          Serial.println(", value: "+_value);
          
          this->setLuciEsterne("Ho variato lo stato delle luci Esterne: ", _value.toInt()==1?true:false);
          return true;
          break;

        case CMD_updateProximity:
          _value=String(packetBuffer).substring(_marker+2);
          Serial.println(", value: "+_value);
          
          this->setProximity("Vedo che stai: ", _value.toInt()==1?true:false);
          return true;
          break;

        case CMD_Check:
          Serial.println();
          UDPSend(*this->_udp, this->_ip, this->_remotePort, "Check", "Ok");
          return true;
          break;

        default:
          Serial.println("Unknown Command");
          break;
        
      }
    }
    else Serial.println("UDP Receive error: "+String(packetBuffer));
  }

  return false;
}


// **************************   UDP
const int numCommands = sizeof(commands) / sizeof(commands[0]);

// Function to map string to enum
CommandIndex getCommandIndex(String input) {
  for (int i = 0; i < numCommands; i++) {
    if (input == commands[i]) {
      return static_cast<CommandIndex>(i);
    }
  }
  
  return CMD_UNKNOWN;
}

void UDPSend(EthernetUDP &Udp, IPAddress ip, unsigned int remotePort, String command, float value) {
  String _toSend=command+MARKER_ID+String(value);
  Serial.println("IOT Send: "+_toSend);

  // Send
  Udp.beginPacket(ip, remotePort);
  Udp.print(_toSend);
  Udp.endPacket();
}

void UDPSend(EthernetUDP &Udp, IPAddress ip, unsigned int remotePort, String command, int value) {
  String _toSend=command+MARKER_ID+String(value);
  Serial.println("IOT Send: "+_toSend);

  // Send
  Udp.beginPacket(ip, remotePort);
  Udp.print(_toSend);
  Udp.endPacket();
}

void UDPSend(EthernetUDP &Udp, IPAddress ip, unsigned int remotePort, String command, String value) {
  String _toSend=command+MARKER_ID+value;
  Serial.println("IOT Send: "+_toSend);

  // Send
  Udp.beginPacket(ip, remotePort);
  Udp.print(_toSend);
  Udp.endPacket();
}


