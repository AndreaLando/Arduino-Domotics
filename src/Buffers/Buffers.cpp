//#define DEBUG_TEST

#include "Buffers.h"


ModbusBuffer::ModbusBuffer(unsigned int items): tracker(items) {
  this->_items=items;
  this->_buffer=new ModbusBufferInfo [items];
}

void ModbusBuffer::SetElement(int modbusArea, int modbusAreaToWrite, bool WriteToPanel, bool ReadFromPanel, bool Reverse, char* name) {
  tracker.registerInit(modbusArea);

  this->_buffer[modbusArea].WriteToPanel=WriteToPanel;
  this->_buffer[modbusArea].name=name;
  this->_buffer[modbusArea].ReadFromPanel=ReadFromPanel;
  this->_buffer[modbusArea].Reverse=Reverse; //Negato
  this->_buffer[modbusArea].modbusAreaToWrite=modbusAreaToWrite;
}

void ModbusBuffer::AddType(int modbusArea, long initialValue, ModbusBufferFlagType type) {
  BufferSourceInfo data;
  data.value=initialValue;
  data.prevValue=0;
  data.time=millis();
  data.bufferType=type;
  data.changed=true;
   
  this->_buffer[modbusArea].Data.add(data);
}

void ModbusBuffer::Init() {
  int idxPnl=0;
  for(int i=0; i<this->_items; i++) {
    if(this->_buffer[i].ReadFromPanel)
      idxPnl++;
  }

  this->_toPanelRead.itemsPtr=new int [idxPnl];
  this->_toPanelRead.size=idxPnl;
  
  idxPnl=0;
  for(int i=0; i<this->_items; i++) {
    if(this->_buffer[i].ReadFromPanel) {
      this->_toPanelRead.itemsPtr[idxPnl]=i;
      idxPnl++;
    }
  }
}

int ModbusBuffer::Compare(int modbusArea, ModbusBufferFlagType type, long value) {
  if(modbusArea>this->_items) {
    Serial.print("Compare ERROR ");
    Serial.println(modbusArea);
    return -1; //Error
  }
  else {
    if(this->_buffer[modbusArea].Data.getSize()==0) {
      return 0; //Not Found
    }
    else {
      int _size=this->_buffer[modbusArea].Data.getSize()-1;
      for (int j=_size; j!=-1; j--) {
        BufferSourceInfo _data=this->_buffer[modbusArea].Data.get(j);
        if (_data.bufferType==type) {
          if( _data.value!=value) { 
            return 1; // Different
          }
          else return 2; //Equal
        }
      }

      return 0; //Not found
    }
  }
  
  return -1; //Error
}

bool ModbusBuffer::WriteElement(int modbusArea, ModbusBufferFlagType type, long value) {
  return WriteElement(modbusArea,type,value,false);
}

bool ModbusBuffer::WriteElement(int modbusArea, ModbusBufferFlagType type, long value, bool silent) {
  if(modbusArea!=DUMMY_AREA) {
    if(modbusArea>this->_items) {
      Serial.print("WriteElement ERROR ");
      Serial.println(modbusArea);
      return false;
    }
    else {
      if(this->_buffer[modbusArea].Data.getSize()==0) {
        BufferSourceInfo data;
        data.prevValue=0; //Essendo creato da zero, il valore precedente è zero
        data.value=value;
        data.time=millis();
        data.bufferType=type;
        data.changed=(silent==true?false:true);
        
        this->_buffer[modbusArea].Data.add(data);

        #ifdef DEBUG_TEST   
        Serial.print("Buffer write area (1st time) ");
        if(silent)
          Serial.print("-SILENT- ");
        Serial.print(modbusArea);
        Serial.print(" value ");
        Serial.print(value); 
        Serial.print(" copy ");
        Serial.print(data.prevValue); 
        Serial.print(" data ");
        Serial.print(data.value); 
        Serial.print(" type ");
        Serial.println(data.bufferType); 
        #endif
      }
      else {
        bool exist=false;
        int _size=this->_buffer[modbusArea].Data.getSize()-1;
        for (int j=_size; j!=-1; j--) {
          BufferSourceInfo _data=this->_buffer[modbusArea].Data.get(j);
          if (_data.bufferType==type) {
            exist=true;
            if( _data.value!=value) { 
              #ifdef DEBUG_TEST 
              if(modbusArea<210) { 
                Serial.print("Buffer write area ");
                if(silent)
                  Serial.print("-SILENT- ");
                Serial.print(modbusArea);
                Serial.print(" value ");
                Serial.print(value); 
                Serial.print(" copy ");
                Serial.print(_data.prevValue); 
                Serial.print(" data ");
                Serial.print(_data.value); 
                Serial.print(" type ");
                Serial.println(_data.bufferType); 
              }
              #endif

              this->_buffer[modbusArea].Data.remove(j);
              _data.prevValue=_data.value;
              _data.value=value;
              _data.time=millis();
              _data.changed=(silent==true?false:true);
              this->_buffer[modbusArea].Data.add(_data); 
            }
            else { 
              #ifdef DEBUG_TEST 
              if(modbusArea<210) { 
                Serial.print("Buffer write area NOT UPDATED ");
                if(silent)
                  Serial.print("-SILENT- ");
                Serial.print(modbusArea);
                Serial.print(" value ");
                Serial.print(value); 
                Serial.print(" copy ");
                Serial.print(_data.prevValue); 
                Serial.print(" data ");
                Serial.print(_data.value); 
                Serial.print(" type ");
                Serial.println(_data.bufferType); 
              }
              #endif
            }
            break; 
          }
        }

        if(!exist) {
          BufferSourceInfo _data;
          _data.prevValue=0;
          _data.value=value;
          _data.time=millis();
          _data.bufferType=type;
          _data.changed=(silent==true?false:true);
          
          this->_buffer[modbusArea].Data.add(_data);

          #ifdef DEBUG_TEST
          Serial.print("Buffer write area NOT EXIST ");
          if(silent)
            Serial.print("-SILENT- ");
          Serial.print(modbusArea);
          Serial.print(" value ");
          Serial.print(value); 
          Serial.print(" copy ");
          Serial.print(_data.prevValue); 
          Serial.print(" data ");
          Serial.print(_data.value); 
          Serial.print(" type ");
          Serial.println(_data.bufferType); 
          #endif
        }
      }
    }
  }
  return true;
}


bool ModbusBuffer::GetChangeFlag(int modbusArea, ModbusBufferFlagType type) {
  
  if(this->_buffer[modbusArea].Data.getSize()!=0) {
    for (int j=0; j<this->_buffer[modbusArea].Data.getSize(); j++) {
      if(this->_buffer[modbusArea].Data.get(j).bufferType==type) {
        if(this->_buffer[modbusArea].Data.get(j).changed ) { 
          return true;
        }
        break;
      }
    }
  }

  return false;
}

void ModbusBuffer::SetChangeFlag(int modbusArea, ModbusBufferFlagType type, bool value) {
    if(this->_buffer[modbusArea].Data.getSize()!=0) {
      for (int j=0; j<this->_buffer[modbusArea].Data.getSize(); j++) {
        if(this->_buffer[modbusArea].Data.get(j).bufferType==type) {
          BufferSourceInfo _data=this->_buffer[modbusArea].Data.get(j);
          this->_buffer[modbusArea].Data.remove(j);
          _data.changed=value;
          this->_buffer[modbusArea].Data.add(_data);
          break;
        }
      }
    }
}

char* ModbusBuffer::GetName(int modbusArea)
{ 
  if(modbusArea<=this->_items) {
    if(this->_buffer[modbusArea].name!=NULL) {
      return this->_buffer[modbusArea].name;
    }
  }

  return "Rocks";
}

bool ModbusBuffer::GetData(int modbusArea, ModbusBufferFlagType type, BufferSourceInfo &dataOut) {
  if(this->_buffer[modbusArea].Data.getSize()!=0) {
    for (int j=0; j<this->_buffer[modbusArea].Data.getSize(); j++) {
      if(this->_buffer[modbusArea].Data.get(j).bufferType==type) {
        dataOut= this->_buffer[modbusArea].Data.get(j);
        return true;
      }
    }
  }

  //Ritorno valori fittizi
  dataOut.value=0;
  dataOut.prevValue=0;
  dataOut.bufferType=type;
  dataOut.changed=false;
  dataOut.time=0;
  return false;
}

void ModbusBuffer::ResetElement(int modbusArea, ModbusBufferFlagType type) {
    SetChangeFlag(modbusArea, type, false);
}

int ModbusBuffer::GetAreaToWrite(int modbusArea) {
  if(modbusArea>this->_items)
    return 0;
  else return this->_buffer[modbusArea].modbusAreaToWrite;
}

bool ModbusBuffer::IsReverse(int modbusArea) {
  return this->_buffer[modbusArea].Reverse;
}

bool ModbusBuffer::CanReadFromPanel(int modbusArea) {
  return this->_buffer[modbusArea].ReadFromPanel;
}

bool ModbusBuffer::CanWriteToPanel(int modbusArea) {
  return this->_buffer[modbusArea].WriteToPanel;
}

bool ModbusBuffer::HasChanged(int modbusArea, ModbusBufferFlagType type) {
  return GetChangeFlag(modbusArea, type);
}

ModbusBufferArrayInfo ModbusBuffer::GetToReadFromPanel() {
  return this->_toPanelRead;
}

int ModbusBuffer::getChanged(ModbusBufferItemInfo2* items, ModbusBufferFlagType type, bool preserveChanges=false) {
  int _foundR=0;

  for (int i=0; i<this->_items; i++) {
    if(this->_buffer[i].Data.getSize()!=0) {
      for (int j = this->_buffer[i].Data.getSize() - 1; j >= 0; j--) {
          auto data = this->_buffer[i].Data.get(j);
          if (data.changed && data.bufferType == type) {
              items[_foundR].Item = data;
              items[_foundR].modbusArea = i;
              _foundR++;

              if (!preserveChanges) {
                  this->_buffer[i].Data.remove(j);
                  data.changed = false;
                  this->_buffer[i].Data.add(data);
              }
          }
      }

      /* old versionfor (int j=0; j<this->_buffer[i].Data.getSize(); j++) {
        if(this->_buffer[i].Data.get(j).changed && this->_buffer[i].Data.get(j).bufferType==type) {
          items[_foundR].Item=this->_buffer[i].Data.get(j);
          items[_foundR].modbusArea=i;
          
          _foundR++;

          if(!preserveChanges) {
            BufferSourceInfo data=this->_buffer[i].Data.get(j);
            if(data.changed) {
              this->_buffer[i].Data.remove(j);
              data.changed=false;
              this->_buffer[i].Data.add(data);
            }
          }
        }
      }*/
    }
  }

  return _foundR;
}

std::vector<int> ModbusBuffer::getNeverInitialized() {
  return tracker.getNeverInitialized();
}

std::vector<int> ModbusBuffer::getInitializedMultipleTimes() {
  return tracker.getInitializedMultipleTimes();
}
