#ifndef PLC_H
#define PLC_H


#include "Signal.h"
#include <ModbusClient.h>
#include <List.hpp>
#include <vector>

// number of items in an array
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

//Cell, classe che implementa un valore con controllo sullo stato variato
template<typename T>
class Cell {
  private:
    T value;
    bool changed;

  public:
    Cell() : value{}, changed(false) {}
    Cell(const T& initial) : value(initial), changed(false) {}

    inline void setIfDiff(const T& newValue) {
      // Set change flag only if old and new values are differents
      if (value != newValue) {
          value = newValue;
          changed = true;
      }
    }

    inline void set(const T& newValue) {
      value = newValue;
      changed = true;
    }

    inline T get() {
        changed = false;
        return value;
    }

    // Lettura che NON resetta il flag 
    inline T preserveGet() const { 
      return value; 
    }

    inline bool hasChanged() const {
        return changed;
    }
};

///// Usi:
//// toggleCucina0.Change(_bit, ModbusTCP.MbData[MBTCP_LAMP_CUCINA01]);  Per fare toggle del bit di ingresso direttamente su una variabile
//// - oppure
//// int tmp= ModbusBuffer.ReadElement(Toggles[i].WriteArea).Data; 
//// if(Toggles[i].Toggle.Change(_dataRead[j], tmp))
////     ModbusBuffer.WriteElement(_device.GetArea(channel, j), tmp);

typedef struct {
    ToggleSignal Toggle;
    int areaRead;
    std::vector<int> forwardsFromAreas; //THis is the index (if not -1) of other toggles to forward to
  }ToggleSignalItem;
  
class ToggleManager {
  private:
    std::vector<ToggleSignalItem> toggles;

  public:
    ToggleManager() {}

    const std::vector<ToggleSignalItem>& getAll() const { 
      return toggles; 
    }

    // Add a new toggle item
    void addToggle(int areaRead, std::vector<int> forwardsFromAreas= std::vector<int>()) {
      ToggleSignalItem item;
      item.areaRead = areaRead;
      item.forwardsFromAreas = forwardsFromAreas;
      toggles.push_back(item);
    }

    // Access a toggle by index
    ToggleSignalItem& operator[](size_t index) {
      return toggles[index];
    }

    size_t size() const {
        return toggles.size();
    }

    // Safe getter: returns nullptr if index is out of range
    ToggleSignalItem* getToggle(size_t index) {
        if (index < toggles.size()) {
            return &toggles[index];
        }
        return nullptr;
    }

    // Get toggle by areaRead (non-const)
    ToggleSignalItem* getToggle(int areaRead) {
      for (auto& t : toggles) {
          if (t.areaRead == areaRead) {
              /* Serial.println();
              Serial.println("Found toggle for area -- "+String(areaRead)); */
              return &t;
          }
      }
      return nullptr;
    }
};

class Errors
{
  public:            
    Errors(short count, unsigned long time=60000);   
    bool Loop(bool inError);
    bool IsInError();   
    void IncrementError();
  
  protected:
   short _errorCnt;

  private:     
    void Check(bool inError);                 
    void Reset(); 
    void Retry();
    
    unsigned long _lastErrorTime;
    bool _error;
    short _count;
   
    unsigned long _time=60000; //60 seconds
};


enum GenericPrgDevicePriority {
      Low=0, 
      Normal=1,
      Medium=2, 
      High=3 
    };

  typedef struct {
    int DeviceIndex;
    GenericPrgDevicePriority Priority;
  }PriorityMgmt;

  typedef struct {
    PriorityMgmt Priorities[4];
    size_t PrioritySize;
    int Index;
  }structPriority; 

  typedef struct {
    arduino::IPAddress IP;
    int Errors; 
    bool InError;
    structPriority Priority;
  }structIP; 


class GenericPrgDevice
{
  public:  
    enum GenericPrgDeviceEnum {
      NOTSET=-1,
      AI=0,
      AO=1,
      DI=2,
      DO=3
    };

    enum GenericPrgDeviceHwEnum {
      Coil=0, //Coil
      Input=1,
      Hold=2, //Holding register
      Discrete=3 //DiscreteInput
    };

  typedef struct {
    GenericPrgDeviceEnum type;
    GenericPrgDeviceHwEnum hwType;
    int startingAddr;
    int items;
    int ItemsPerCall; //If I need more bytes to build 1 call
  }GenericPrgDeviceChannel;  

  typedef struct {
    short items;
    int startIndex;
    bool ok;
  }structRead; 

    GenericPrgDevice(const char* name, arduino::IPAddress ip, unsigned int deviceAddress, GenericPrgDeviceChannel channels[], size_t channelSize, std::vector<int> ioAreas, short ErrorCnt, GenericPrgDevicePriority priority);     
    bool Run();
    structRead Read(ModbusClient &mb, int channel, List<uint16_t> *value);
    bool Read(ModbusClient &mb, int channel, float *value);
    bool Write(ModbusClient &mb, int channel, int address, int value);
    int GetArea(int channel, int address);
    
    GenericPrgDeviceChannel GetChannelInfo(int channel);
    arduino::IPAddress GetIp();
    GenericPrgDevicePriority GetPriority();
    size_t GetChannelsSize();
    const char* GetName();
    unsigned int GetDeviceAddress();
    bool IsInError();
  private:  
    short bank;
    std::vector<int> _ioAreas;
    GenericPrgDevicePriority _priority;
    const char* _name;
    unsigned int _deviceAddress;
    arduino::IPAddress _ip;
    size_t _channelSize;
    GenericPrgDeviceChannel *_channels;
    Errors Error;
    const short MAX_CALLS=8;
};

int GetJump(GenericPrgDevicePriority priority);
int GetDevicesByIp(arduino::IPAddress ip, std::vector<GenericPrgDevice> prgDevices, std::vector<GenericPrgDevice> &items);
int GetDevicesByPriority(GenericPrgDevicePriority priority, arduino::IPAddress ip, std::vector<GenericPrgDevice> prgDevices, std::vector<int> &items);
bool ExistDevicesByPriority(GenericPrgDevicePriority priority, arduino::IPAddress ip, std::vector<GenericPrgDevice> prgDevices);
int GetUsedPriorities(arduino::IPAddress ip, std::vector<GenericPrgDevice> prgDevices, List<PriorityMgmt> *items);
bool ExistDevicesByIp(arduino::IPAddress ip, std::vector<GenericPrgDevice> prgDevices);
int BuildIps(std::vector<GenericPrgDevice> prgDevices, List<structIP> *items);

//Calcolatore Medie
#define NUM_VARIAZIONI 5

class Gruppo {
public:
    String nome;

    // Buffer circolare misure
    std::vector<float> buffer;
    int maxSize;
    int head = 0;
    int count = 0;

    // Buffer circolare variazioni
    std::vector<float> variazioni;
    int headVar = 0;
    int countVar = 0;

    float soglia;

    enum Trend {
        COSTANTE,
        CRESCENTE,
        DECRESCENTE
    };

    Gruppo(String n, int size = 5, float sens = 0.1)
        : nome(n), maxSize(size), soglia(sens)
    {
        buffer.resize(maxSize, 0);
        variazioni.resize(NUM_VARIAZIONI, 0);
    }

    void aggiornaMisura(float valore) {
        // Calcolo variazione rispetto all'ultima misura
        if (count > 0) {
            float ultima = buffer[(head - 1 + maxSize) % maxSize];
            float diff = valore - ultima;
            aggiornaVariazione(diff);
        }

        // Scrittura nel buffer circolare
        buffer[head] = valore;
        head = (head + 1) % maxSize;

        if (count < maxSize) count++;
    }

    void aggiornaVariazione(float diff) {
        variazioni[headVar] = diff;
        headVar = (headVar + 1) % NUM_VARIAZIONI;

        if (countVar < NUM_VARIAZIONI) countVar++;
    }

    float media() const {
        if (count == 0) return 0;

        float somma = 0;
        for (int i = 0; i < count; i++) {
            somma += buffer[i];
        }
        return somma / count;
    }

    float mediaVariazioni() const {
        if (countVar == 0) return 0;

        float somma = 0;
        for (int i = 0; i < countVar; i++) {
            somma += variazioni[i];
        }
        return somma / countVar;
    }

    Trend trend() const {
        if (countVar == 0) return COSTANTE;

        float m = mediaVariazioni();

        if (fabs(m) < soglia) return COSTANTE;
        if (m > 0) return CRESCENTE;
        return DECRESCENTE;
    }
};


class CalcolatoreMedie {
public:
    std::vector<Gruppo> gruppi;

    void creaGruppo(const String& nome, int size = 5, float soglia = 0.1) {
        gruppi.emplace_back(nome, size, soglia);
    }

    Gruppo* trovaGruppo(const String& nome) {
        for (auto& g : gruppi) {
            if (g.nome == nome) return &g;
        }
        return nullptr;
    }

    void aggiungiMisura(const String& nomeGruppo, float valore, float soglia = 0.1) {
        Gruppo* g = trovaGruppo(nomeGruppo);
        if (!g) {
            creaGruppo(nomeGruppo, 5, soglia);
            g = trovaGruppo(nomeGruppo);
        }
        g->aggiornaMisura(valore);
    }

    float mediaGruppo(const String& nomeGruppo) {
        Gruppo* g = trovaGruppo(nomeGruppo);
        return g ? g->media() : 0;
    }

    Gruppo::Trend trendGruppo(const String& nomeGruppo) {
        Gruppo* g = trovaGruppo(nomeGruppo);
        return g ? g->trend() : Gruppo::COSTANTE;
    }
};

#endif
