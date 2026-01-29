#include "Fncs.h"

#define DEBUG_ERROR
//#define DEBUG_TEST_PANEL
#define DEBUG_TEST
//#define DEBUG_VISUAL

const int ANALOG_TRESHOLD=25;
const int DELAY_VISUAL=800;

bool ManageMdbCli(pin_size_t ledR, pin_size_t ledW, ModbusTCPClient &modbusTCPCli, List<structIP> *IPList, short ipIndex,
  ModbusBuffer &buffer, std::vector<GenericPrgDevice> &prgDevices, ToggleManager &toggles, SomethingChangedFn somethingChanged, RouteFn route) {
  
  bool _connected=false;
  bool ioError=false;
     
  //Modbus CLIENT for Waveshare
  if (modbusTCPCli.connected()==0) {
    // client not connected, start the Modbus TCP client
    
    //Serial.print("Attempting to connect to Modbus TCP server ");
    //Serial.println(ip);
    
    if (modbusTCPCli.begin(IPList->get(ipIndex).IP, 502)==0) {
      Serial.print("Modbus TCP Client failed to connect on ");
      Serial.print(IPList->get(ipIndex).IP);
      Serial.print(", Last error:");
      Serial.println(modbusTCPCli.lastError());
      
      structIP tmp=IPList->get(ipIndex);
      tmp.Errors+=1;
      tmp.InError=true;
      IPList->remove(ipIndex);
      IPList->addAtIndex(ipIndex,tmp);
    }
    else {
      _connected=true; // client connected
    }
  } 
  else {
    _connected=true; // client connected
  }

  if(_connected) {
    if(IPList->get(ipIndex).InError) {
      //Se mi connetto ed ero in errore, resetto la struttura
      structIP _tmp=IPList->get(ipIndex);
      _tmp.Errors=0;
      _tmp.InError=false;
      IPList->remove(ipIndex);
      IPList->addAtIndex(ipIndex,_tmp);
    }
        
    // Lettura devices Modbus in ingresso
    if(!DeviceManagement_Read(ledR, modbusTCPCli, IPList, ipIndex, buffer, prgDevices, toggles))
      ioError=true;

    //Riverso poi gli I/O
    bool _anyChange=false;
    for(int area=0; area<buffer.size(); area++) {
      //Serial.print(" >>READ ");
      //  Serial.println(area);
      BufferSourceInfo _sourceInfo;
      if(buffer.GetData(area, Field, _sourceInfo )) {
        if(_sourceInfo.changed) {
          int areatoWrite=buffer.GetAreaToWrite(area);
          if(areatoWrite!=0) {
            buffer.WriteElement(areatoWrite, Field, _sourceInfo.value);
            buffer.ResetElement(area, Field);
                     
            //Vengono saltati quelli con AreaToWrite=0
            /*
            Serial.print(" Changed ");
            Serial.print(area);
            Serial.print(" write area ");
            Serial.print(areatoWrite,DEC);
            Serial.print(" value ");
            Serial.println(_sourceInfo.value,DEC); */
          route(_sourceInfo, area, buffer);
          _anyChange=true;
          }
        }
      }
    }

    if(_anyChange) {
      somethingChanged(buffer);
    }

    // Scrittura devices Modbus in uscita
    if(!DeviceManagement_Write(ledW, modbusTCPCli, IPList->get(ipIndex).IP, buffer, prgDevices))
      ioError=true;
  }
  //modbusTCPCli.end();
  modbusTCPCli.stop(); //Chiudere sempre, non mettere in parentesi prima
 
  return ioError;
}

bool DeviceManagement_Write(pin_size_t led, ModbusTCPClient &modbusTCPCli, arduino::IPAddress ip, ModbusBuffer &buffer, std::vector<GenericPrgDevice> prgDevices)
{
  //Get all devices under same IP address (device under Waveshare)
  std::vector<GenericPrgDevice> _devicesUnderSameIP;
  int _items=GetDevicesByIp(ip, prgDevices, _devicesUnderSameIP);
  
  bool inError=false;

  if(digitalRead(led))
    digitalWrite(led, !digitalRead(led));
  
  for (auto& _deviceUnderSameIP : _devicesUnderSameIP) {
    if(!_deviceUnderSameIP.IsInError()) { 
      //Se il device è in errore per le precedenti letture, lo salto (ATTENZIONE che non va per i device solo in USCITA, dato che NON ne testo la connessione)

      //For each Device, poll its channels
      for(int channel=0; channel< _deviceUnderSameIP.GetChannelsSize(); channel++) {
        if(_deviceUnderSameIP.GetChannelInfo(channel).type==GenericPrgDevice::DO || _deviceUnderSameIP.GetChannelInfo(channel).type==GenericPrgDevice::AO) {
          //Type WRITE
          for(int j=0; j< _deviceUnderSameIP.GetChannelInfo(channel).items; j++) {
            int _area=_deviceUnderSameIP.GetArea(channel, j);
            if(buffer.HasChanged(_area, Field)) {
              BufferSourceInfo _sourceInfo;
              if(buffer.GetData(_area, Field, _sourceInfo )) {
                /*
                Serial.print("BUFFER CHANGED (Field): ");
                Serial.print(" area ");
                Serial.print(_area);
                Serial.print(" value ");
                Serial.print(_sourceInfo.value);
                Serial.print(" ( ");
                Serial.print(buffer.GetName(_area));
                Serial.println(" ) ");*/

                if(_deviceUnderSameIP.Write(modbusTCPCli, channel, j, _sourceInfo.value)==0) {
                  inError=true;
                  
                  #ifdef DEBUG_ERROR
                  Serial.print("DeviceManagement_Write - Cannot WRITE: ");
                  Serial.print(_deviceUnderSameIP.GetName());

                  Serial.print(" IP: ");
                  Serial.print(_deviceUnderSameIP.GetIp());

                  Serial.print(" Address: ");
                  Serial.print(_deviceUnderSameIP.GetDeviceAddress());

                  Serial.print(" channel: ");
                  Serial.print(channel);
                  Serial.print(" item: ");
                  Serial.print(j);

                  Serial.print(" area ");
                  Serial.print(_area);

                  Serial.print(" ( ");
                  Serial.print(buffer.GetName(_area));
                  Serial.println(" ) ");
                  #endif
                  break;
                }
                                
                //Scritto con successo o fallimento, resetto solo Field
                buffer.ResetElement(_area, Field);
              }
            }
          }
        }
      } 
    }  
  }

  return !inError;
}

int GetToggleFwdValue(int area, ToggleManager &toggles, ModbusBuffer &buffer) {
  int _result=0;

  for (auto& toggle : toggles.getAll()) {              
    //Verifico se ci sono forwards
    if(toggle.areaRead==area) {
      if(!toggle.forwardsFromAreas.empty()) {
        for (int i=0; i< toggle.forwardsFromAreas.size(); i++) { 
          BufferSourceInfo _sourceInfo;
          if(buffer.GetData(toggle.forwardsFromAreas[i], Field, _sourceInfo )) {
            /*
            Serial.println();
            Serial.print("Toggle found id: "+String(i)+" area: "+String(area)+" fwd area: "+String(toggle.forwardsFromAreas[i]));
            Serial.println(", value: "+String(_sourceInfo.value));   */
            if(_sourceInfo.value>0) {
              _result=1;
              break;
            }
          }
        }  
      }
    }
    if(_result==1) break;
  }

  return _result;
}

void DeviceManagement_Read_SetOut(ModbusBuffer &buffer, int area, int value)
{
  buffer.WriteElement(area, Field, value);
  if(buffer.CanWriteToPanel(area))
    buffer.WriteElement(area, ToPanel, value);
}

bool DeviceManagement_Read(pin_size_t led, ModbusTCPClient &modbusTCPCli, List<structIP> *iPList, short ipIndex, ModbusBuffer &buffer, 
  std::vector<GenericPrgDevice> &prgDevices, ToggleManager &toggles)
{
  bool _error=false;
  
  PriorityMgmt _actualPriority=iPList->get(ipIndex).Priority.Priorities[iPList->get(ipIndex).Priority.Index];
  
  std::vector<int> _devices;
  int _items=GetDevicesByPriority(_actualPriority.Priority, iPList->get(ipIndex).IP, prgDevices, _devices);  //Get all devices under same IP address (device under Waveshare)
  if(_items>0) {
    /* #ifdef DEBUG_TEST
    Serial.println();
    Serial.print("Checking ");
    Serial.print(iPList->get(ipIndex).IP);
    Serial.print(" - ");
    Serial.print(iPList->get(ipIndex).Priority.Index);
    Serial.print(" - ");
    Serial.println(iPList->get(ipIndex).Priority.PrioritySize);
    Serial.print("Priority: ");
    Serial.println(_actualPriority.Priority);
    #endif */
    
    int _start=0;
    int _end=_items;
    digitalWrite(led, !digitalRead(led));

    if(_actualPriority.DeviceIndex!=-1) {
      //Passa la prima inizializzazione nella quale faccio polling di tutte le periferiche
      
      int _jump=GetJump(_actualPriority.Priority);
      switch (_actualPriority.Priority)   {
        case Low:
          _start=_actualPriority.DeviceIndex;
          _end=_actualPriority.DeviceIndex+_jump>=_items?_items:_actualPriority.DeviceIndex+_jump;
          break;

        case Medium:
          _start=_actualPriority.DeviceIndex;
          _end=_actualPriority.DeviceIndex+_jump>=_items?_items:_actualPriority.DeviceIndex+_jump;
        break;
        
        case Normal:
          _start=_actualPriority.DeviceIndex;
          _end=_actualPriority.DeviceIndex+_jump>=_items?_items:_actualPriority.DeviceIndex+_jump;
        break;
      }
    }

    for (int _deviceIndex=_start; _deviceIndex<_end; _deviceIndex++)
    {
      //For each Device, poll its channels
      for(int channel=0; channel<prgDevices[_devices[_deviceIndex]].GetChannelsSize(); channel++)
      {
        if(prgDevices[_devices[_deviceIndex]].GetChannelInfo(channel).type==GenericPrgDevice::DI || prgDevices[_devices[_deviceIndex]].GetChannelInfo(channel).type==GenericPrgDevice::AI)
        {     
          List<uint16_t> _mbRead;
          GenericPrgDevice::structRead _read=prgDevices[_devices[_deviceIndex]].Read(modbusTCPCli, channel, &_mbRead);
          if(_read.ok) {
            for(int j=0; j< _read.items; j++) {
              int _index=_read.startIndex +j;
              int _area=prgDevices[_devices[_deviceIndex]].GetArea(channel, _index);
               
              bool _process=false; //verifica le variazioni
              BufferSourceInfo _buffer;
              buffer.GetData(_area, Field, _buffer);  

              unsigned short _value=_mbRead.get(j);
              ToggleSignalItem* _toggle=nullptr;
             
              if(prgDevices[_devices[_deviceIndex]].GetChannelInfo(channel).type==GenericPrgDevice::AI) {
                //Analog check trashold
                _process=abs(_value-_buffer.value)>ANALOG_TRESHOLD;
              }
              else {
                _toggle=toggles.getToggle(_area);
                
                //Digital
                if(buffer.IsReverse(_area))
                  _value= !_value;     

                if(_toggle==nullptr) 
                  //Senza toggle
                  _process= _value!=_buffer.value;
                else {
                  if(_toggle->forwardsFromAreas.empty() )
                    //Toggle senza forwards
                    _process= _value!=_buffer.value || _value!=_toggle->Toggle.getOldValue();
                  else {
                    /*
                    Serial.println();
                    Serial.print("Test area -- ");
                    Serial.print(_area);
                    Serial.println();*/
                    //Devo verificare se il toggle ha forward verso di lui, nel caso ne devo testare il valore
                    long _tmp=GetToggleFwdValue(_area, toggles, buffer );
                    if(_tmp==0)
                      _tmp=_value;
                    _process= _tmp!=_buffer.value || _tmp!=_toggle->Toggle.getOldValue();   //Entra se ho toggle normale     
                  }
                }
              }
              
              if(_process ) {
                //#ifdef DEBUG_TEST
                if(_value>0 && _area>=11){ //Prende sia digitali a 1 che analogiche
                  Serial.println();
                  Serial.print("DeviceManagement_Read -- ");
                  Serial.print(prgDevices[_devices[_deviceIndex]].GetName());
                  Serial.print(" channel ");
                  Serial.print(channel);
                  Serial.print(" item ");
                  Serial.print(_index);
                  Serial.print(" value ");
                  Serial.print(_value,DEC);
                  Serial.print(", Buffer value ");
                  Serial.print(_buffer.value,DEC);
                  Serial.print(", Buffer prev value ");
                  Serial.print(_buffer.prevValue,DEC);
                  Serial.print(", AREA ");
                  Serial.print(_area,DEC); 
                  Serial.println(String(" - ")+buffer.GetName(_area));
                }
                //#endif
                buffer.ResetElement(_area, Field);
                              
                if(_toggle!=nullptr) {
                  if(_toggle->forwardsFromAreas.empty()) {
                    // Lo gestisco come nessun toggle associato, scrivo direttamente l'uscita
                    long _toggleOut=_buffer.value;
                    #ifdef DEBUG_TEST
                    Serial.print(" Write buffer std TOGGLE NO FWD ");
                    Serial.print(" Area: "+String(_area));
                    Serial.println(", Value: "+String(_value)); 
                    #endif 

                    if(_toggle->Toggle.change(_value, _toggleOut)) {
                      #ifdef DEBUG_TEST
                      Serial.println(", Toggle Value:"+String(_toggleOut));
                      #endif
                      DeviceManagement_Read_SetOut(buffer, _area, _toggleOut);
                    }
                  }
                  else {
                    long _toggleOut=_buffer.value;
                    //Devo verificare se il toggle ha forward verso di lui, nel caso ne devo testare il valore
                    int _signalIn=GetToggleFwdValue(_area, toggles, buffer );
                    if(_signalIn==0)
                      _signalIn=_value;
                    
                    #ifdef DEBUG_TEST
                    Serial.println();
                    Serial.print(" Write buffer toggle ");
                    Serial.print(" Area: "+String(_area));
                    Serial.println(", Value: "+String(_signalIn));
                    #endif
                    if(_toggle->Toggle.change(_signalIn, _toggleOut)) {
                      #ifdef DEBUG_TEST
                      Serial.println(", Toggle Value:"+String(_toggleOut));
                      #endif
                      DeviceManagement_Read_SetOut(buffer, _area, _toggleOut);
                    }
                  }
                }
                else {
                  // Se non ho un toggle associato, scrivo direttamente l'uscita
                  /*
                  #ifdef DEBUG_TEST
                  Serial.print(" Write buffer std ");
                  Serial.print(" Area: "+String(_area));
                  Serial.println(", Value: "+String(_value));
                  #endif */
                  DeviceManagement_Read_SetOut(buffer, _area, _value);
                }
              } 
            }
            
            #ifdef DEBUG_VISUAL
              delay(DELAY_VISUAL);
            #endif 
          }
          else {
            /*
            #ifdef DEBUG_ERROR
              Serial.print("DeviceManagement_Read - Cannot READ: ");
              Serial.print(prgDevices[_devices[_deviceIndex]].GetName());

              Serial.print(", IP: ");
              Serial.print(prgDevices[_devices[_deviceIndex]].GetIp());

              Serial.print(", Address: ");
              Serial.println(prgDevices[_devices[_deviceIndex]].GetDeviceAddress());
            #endif  */
            _error=true;
            break;
          }
        }
      } 
    }

    structIP tmp=iPList->get(ipIndex);
    int _jump=GetJump(tmp.Priority.Priorities[tmp.Priority.Index].Priority );
    if(tmp.Priority.Priorities[tmp.Priority.Index].DeviceIndex+_jump>=_items || tmp.Priority.Priorities[tmp.Priority.Index].DeviceIndex==-1)
      tmp.Priority.Priorities[tmp.Priority.Index].DeviceIndex=0;
    else
      tmp.Priority.Priorities[tmp.Priority.Index].DeviceIndex+=_jump;
  
    if(tmp.Priority.Index>=tmp.Priority.PrioritySize-1)
      tmp.Priority.Index=0;
    else
      tmp.Priority.Index++;

    iPList->remove(ipIndex);
    iPList->addAtIndex(ipIndex,tmp);
  } 
  else {
    #ifdef DEBUG_ERROR
      Serial.print("Nothing to read with priority: ");
      Serial.print(_actualPriority.Priority);

      Serial.print(", IP: ");
      Serial.print(iPList->get(ipIndex).IP);
    #endif  
  }

  return !_error;
}

void ManageMdbSvr(pin_size_t led, EthernetClient &client, MgsModbus &modbusTCPSvr, ModbusBuffer &buffer, ToggleManager &toggles, char *itemName, bool mode) {
  // poll for Modbus TCP requests, while client connected
  digitalWrite(led, !digitalRead(led));

  if (mode) {
    //GET data from BUFFER if any changed and update panel 
    ModbusBufferItemInfo2 _items[buffer.size()];
    int _toRead=buffer.getChanged(_items, ToPanel, true); //(preserve changed flag, will be resetted if write NOT fail
    if(_toRead>0) {
      for(int i=0; i<_toRead; i++)  {
        #ifdef DEBUG_TEST_PANEL
        if(_items[i].modbusArea<210 && _items[i].modbusArea>9) {
          Serial.print(" BUFFER > PANEL value:");
          Serial.print(_items[i].Item.value,DEC);
          Serial.print(" area ");
          Serial.println(_items[i].modbusArea,DEC);  
        }
        #endif

        //Scrivo sui registri del pannello, azzero il change del buffer
        modbusTCPSvr.MbData[_items[i].modbusArea]= _items[i].Item.value;
        buffer.ResetElement(_items[i].modbusArea, ToPanel); //Resetto flag bit xche la lettura dello stato era PRESERVE
        buffer.WriteElement(_items[i].modbusArea, FromPanel, _items[i].Item.value, true); //Genero un evento fittizio silenzioso per il comando che torna indietro
      
      }
    }
  }
  else {
    //Read all data from panel and write into buffer
    ModbusBufferArrayInfo _toRead=buffer.GetToReadFromPanel();
    if(_toRead.size>0) {    
      int _items=_toRead.size>buffer.size()?buffer.size():_toRead.size;            
      for(int i=0; i<_items; i++)  {            
        long _valueRead=modbusTCPSvr.MbData[_toRead.itemsPtr[i]]; 

        if(buffer.Compare(_toRead.itemsPtr[i], FromPanel, _valueRead)!=2) {
          #ifdef DEBUG_TEST_PANEL
          Serial.println();
          Serial.print(" PANEL > BUFFER value:");
          Serial.print(_valueRead,DEC); 
          Serial.print(" area ");
          Serial.println(_toRead.itemsPtr[i],DEC);
          #endif

          buffer.WriteElement(_toRead.itemsPtr[i], FromPanel, _valueRead);
          
          //Il pannello è variato, riverso il valore come se fosse arrivato da campo
          if(buffer.WriteElement(_toRead.itemsPtr[i], Field, _valueRead))
            buffer.ResetElement (_toRead.itemsPtr[i], FromPanel); //Abbasso il flag di modifica xche è stato processato dal Field
          
          if(buffer.CanWriteToPanel(_toRead.itemsPtr[i]))
              buffer.WriteElement(_toRead.itemsPtr[i], ToPanel, _valueRead, true); //Aggiorno anche il suo eventuale omonimo comando, in maniera silente
          
          #ifdef DEBUG_TEST_PANEL
          Serial.println(" PANEL > BUFFER - END -");
          #endif
        }
      }
    } 
  } 

  modbusTCPSvr.MbsRun(client);   
}



