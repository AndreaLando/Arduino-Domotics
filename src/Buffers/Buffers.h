#ifndef Buffers_H
#define Buffers_H

#include <List.hpp>
#include "Arduino.h"
#include <vector>

const int DUMMY_AREA=999;
// number of items in an array
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

class AreaTracker {
private:
    std::vector<int> initCount;

public:
    AreaTracker(int totalAreas) {
        initCount.resize(totalAreas, 0);
    }

    void registerInit(int modbusArea) {
        if (modbusArea >= 0 && modbusArea < initCount.size()) {
            initCount[modbusArea]++;
        }
    }

    std::vector<int> getNeverInitialized() const {
        std::vector<int> result;
        for (int i = 0; i < initCount.size(); i++) {
            if (initCount[i] == 0)
                result.push_back(i);
        }
        return result;
    }

    std::vector<int> getInitializedMultipleTimes() const {
        std::vector<int> result;
        for (int i = 0; i < initCount.size(); i++) {
            if (initCount[i] > 1)
                result.push_back(i);
        }
        return result;
    }
};

enum ModbusBufferFlagType 
{
  Field=0,
  FromPanel=1,
  ToPanel=2
};

typedef struct {
    ModbusBufferFlagType bufferType;
    unsigned long time; //in millis, that take 50 days to go back to zero. -1 means never write or no change
    long value;
    long prevValue;
    bool changed;
  }BufferSourceInfo;

typedef struct {
  List<BufferSourceInfo> Data; // Piu BufferSourceInfo in base al tipo
  bool Reverse;
  bool ReadFromPanel;
  bool WriteToPanel;
  bool FromPanelToField;
  int modbusAreaToWrite;
  char* name;
}ModbusBufferInfo;

typedef struct {
    ModbusBufferInfo Item;
    int modbusArea;
  }ModbusBufferItemInfo;

  typedef struct {
    BufferSourceInfo Item;
    int modbusArea;
  }ModbusBufferItemInfo2;

typedef struct {
    ModbusBufferInfo Item;
    bool done;
  }ModbusBufferReadElementInfo;

typedef struct {
    int *itemsPtr;
    int size;
  }ModbusBufferArrayInfo;

class ModbusBuffer
{
  public:             
    ModbusBuffer(unsigned int items);       
    int getChanged(ModbusBufferItemInfo* items);
    ModbusBufferArrayInfo GetToReadFromPanel();
    void Init();
    void SetElement(int modbusArea, int modbusAreaToWrite, bool WriteToPanel, bool ReadFromPanel, bool Reverse, char* name);
    void AddType(int modbusArea, long initialValue, ModbusBufferFlagType type);
    bool CanReadFromPanel(int modbusArea);
    bool CanWriteToPanel(int modbusArea);
    bool WriteElement(int modbusArea, ModbusBufferFlagType type, long value, bool silent);
    bool WriteElement(int modbusArea, ModbusBufferFlagType type, long value);
    bool IsReverse(int modbusArea);
    bool HasChanged(int modbusArea, ModbusBufferFlagType type);
    int GetAreaToWrite(int modbusArea);
    bool GetData(int modbusArea, ModbusBufferFlagType type, BufferSourceInfo &data);
    ModbusBufferReadElementInfo ReadElement(int modbusArea, bool preserve, ModbusBufferFlagType type);
    void ResetElement(int modbusArea, ModbusBufferFlagType type);
    int getChanged(ModbusBufferItemInfo2* items, ModbusBufferFlagType type, bool preserveChanges);
    char* GetName(int modbusArea);
    int Compare(int modbusArea, ModbusBufferFlagType type, long value);
    std::vector<int> getNeverInitialized();
    std::vector<int> getInitializedMultipleTimes();

    size_t size() const {
        return _items;
    }
  private: 
    void SetChangeFlag(int modbusArea, ModbusBufferFlagType type, bool value); 
    bool GetChangeFlag(int modbusArea, ModbusBufferFlagType type); 
    AreaTracker tracker;
    int _items;
    ModbusBufferInfo *_buffer;
    ModbusBufferArrayInfo _toPanelRead;
};

#endif
