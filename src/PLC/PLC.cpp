//#define DEBUG_TEMPHUM
//#define DEBUG_READDEVICE
//#define DEBUG_ENERGY

#define DEBUG_ERRORS
//#define DEBUG_TEST
//#define DEBUG_ERRORS_ERR

#include "PLC.h"

ToggleSignal::ToggleSignal(){
  this->_oldStatus=0;
}

long ToggleSignal::GetOldValue() {
  return this->_oldStatus;
}

bool ToggleSignal::Change(int statusIn, long &value) {
  if(this->_oldStatus!=statusIn) { 
    #ifdef DEBUG_TEST
    Serial.print( "CHANGE Old: ");
    Serial.print( this->_oldStatus);
    Serial.print( ", In: ");
    Serial.print( statusIn);
    Serial.print( ", Value: ");
    Serial.println( value);
    #endif 

    this->_oldStatus=statusIn;  
    if(statusIn==1) {  
      value=(value==0?1:0);   
      return true;
    }
  }  

  return false;
}

void ToggleSignal::Force(long value) {
  this->_oldStatus=value;
}

AntiBump::AntiBump(bool initialState) {
  this->_initialState=initialState;
  this->OS(this->_initialState);  
}

bool AntiBump::Change(bool state) {
  if(state) {
     if(! this->OS(this->_initialState).GetState() ) {  
      this->OS(this->_initialState).GetState();
      
      return true;
    }
  } else this->OS(this->_initialState).Reset(); 

  return false;
}
  
//////////////////////////////////////////////////////////
OneShot::OneShot(bool initialState){
  this->_toControl=initialState;
  this->_done=false;  
}

bool OneShot::Loop(bool state){
  if(state) {
    if(!GetState() && !this->_done) {
      Set();
      return true;
    }
  } else Reset();

  return false;
}

void OneShot::Set(){
  this->_toControl=true;
  this->_done=false;  
}

void OneShot::Reset(){
  this->_toControl=false;
  this->_done=false;  
}

void OneShot::Done(){  
  this->_done=true;  
}

bool OneShot::GetState(){
  return this->_toControl;
}

bool OneShot::IsDone(){
  return this->_done;
}

///////////////// Errors
Errors::Errors(short count, unsigned long time){  
  this->_count=count; 
  this->_time=time; 

  this->_error=false;
  this->_errorCnt=0; 
  this->_lastErrorTime =0;
}

bool Errors::Loop(bool inError){  
  if(!this->_error) {
    Check(inError);
    return !this->_error;
  }
  else {
    Retry();  //Wait some time
  }  

  return false;
}

bool Errors::IsInError(){  
  return this->_error;
}

///// Private
void Errors::Reset(){  
  this->_error=false;
  this->_errorCnt=0; 
  this->_lastErrorTime =0;

  #ifdef DEBUG_ERRORS_ERR
  Serial.println( "ERROR RESET");
  #endif
}

void Errors::Retry(){  
  unsigned long _value=this->_lastErrorTime+(this->_time*this->_count);
  #ifdef DEBUG_ERRORS_ERR
  Serial.print( "ERROR RETRY ");
  Serial.println( this->_error);
  #endif

  if(this->_error && _value  <= millis()) {       
    Reset();
  }    
}

void Errors::IncrementError(){
  this->_errorCnt++;

  #ifdef DEBUG_ERRORS_ERR
  Serial.print( "ERROR INCREMENT Counter ");
  Serial.println( this->_errorCnt);
  #endif
 }

void Errors::Check(bool inError){  
  if(inError) {
    #ifdef DEBUG_ERRORS_ERR
    Serial.print( "ERROR CHECK, cnt: ");
    Serial.print( this->_errorCnt);
    Serial.println( this->_error);
    #endif

    if(this->_errorCnt>=this->_count) {  
      #ifdef DEBUG_ERRORS_ERR
      Serial.print( "ERROR CHECK - ERROR SET ");
      #endif

      this->_error=true;
      this->_lastErrorTime = millis();  
    }
    else
      IncrementError();
  }
}

////////////////////////////////////////////////////////// GenericDevice
GenericPrgDevice::GenericPrgDevice(const char* name, arduino::IPAddress ip, unsigned int deviceAddress, GenericPrgDeviceChannel channels[], size_t channelSize, std::vector<int> ioAreas, short ErrorCnt, GenericPrgDevicePriority priority): Error(ErrorCnt, 30000)
{ 
 this->_channels=channels;
 this->_channelSize=channelSize;

  this->_ioAreas=ioAreas;

  this->_deviceAddress=deviceAddress;
  this->_name=name;
  
  this->_ip=ip;
  this->_priority=priority;
  this->bank=0; // nel caso di calls ripetute
}

int GenericPrgDevice::GetArea(int channel, int address)
{ 
  if(this->_ioAreas.empty())
    return -1;
  else { 
    int size=this->_channels[0].items * channel; //VERIFICARE GET 0 non deve essere sempre fisso
    return this->_ioAreas[size + address];
  }
}

GenericPrgDevice::GenericPrgDeviceChannel GenericPrgDevice::GetChannelInfo(int channel)
{ 
  if(_channelSize-1>=channel)
    return this->_channels[channel];
  else { 
    GenericPrgDeviceChannel tmp;
    tmp.type=NOTSET;
    return tmp;
  }
}

bool GenericPrgDevice::IsInError()
{ 
  return this->Error.IsInError(); //this->_inError;
}

size_t GenericPrgDevice::GetChannelsSize()
{ 
  return this->_channelSize;
}

GenericPrgDevicePriority GenericPrgDevice::GetPriority()
{ 
  return this->_priority;
}

arduino::IPAddress GenericPrgDevice::GetIp()
{ 
  return this->_ip;
}

unsigned int GenericPrgDevice::GetDeviceAddress()
{ 
  return this->_deviceAddress;
}

const char* GenericPrgDevice::GetName()
{ 
  return this->_name;
}

float get_float(const uint16_t *src) 
{ 
  float f; 
  uint32_t i = (((uint32_t)src[1]) << 16) + src[0]; 
  
  memcpy(&f, &i, sizeof(float)); 

  return f; 
} 

bool GenericPrgDevice::Read(ModbusClient &mb, int channel, float *value)
{
  switch(this->_channels[channel].hwType) {
    case Hold: //Hold
      if(!this->Error.IsInError()) {
        int tmpRead;
        tmpRead=mb.requestFrom(this->_deviceAddress, HOLDING_REGISTERS, this->_channels[channel].startingAddr, this->_channels[channel].items*this->_channels[channel].ItemsPerCall );
        if(tmpRead!=0) { 
          for (int i=0; i<(tmpRead/this->_channels[channel].ItemsPerCall); i++) {
            uint16_t tmp[this->_channels[channel].ItemsPerCall];
            tmp[1]=mb.read();
            tmp[0]=mb.read();
            
            value[i]=get_float(tmp);
            //Serial.print("GenericPrgDevice read hold reg. (float): ");
            //Serial.println(value[i]);
          }

          this->Error.Loop(false);
          return true;
        }
        else if(!this->Error.Loop(true)) {
          #ifdef DEBUG_ERRORS
          Serial.print("Generic Device ERROR: ");
          Serial.print(this->_name);

          Serial.print(" IP: ");
          Serial.print(this->_ip);

          Serial.print(" Address: ");
          Serial.println(this->GetDeviceAddress());   
          #endif     
        }
      }
      else this->Error.Loop(true);
      
      return false;
    break;

    case Input: //Input Register
      if(!this->Error.IsInError()) {
        int tmpRead;
        tmpRead=mb.requestFrom(this->_deviceAddress, INPUT_REGISTERS, this->_channels[channel].startingAddr, this->_channels[channel].items*this->_channels[channel].ItemsPerCall);
        if(tmpRead!=0) { 
          for (int i=0; i<(tmpRead/this->_channels[channel].ItemsPerCall); i++) {
            uint16_t tmp[this->_channels[channel].ItemsPerCall];
            tmp[1]=mb.read();
            tmp[0]=mb.read();
            
            value[i]=get_float(tmp);
            //Serial.print("GenericPrgDevice read input reg. (float): ");
            //Serial.println(value[i]);
          }

          this->Error.Loop(false);
          return true;
        }
        else if(!this->Error.Loop(true)) {
          #ifdef DEBUG_ERRORS
          Serial.print("Generic Device ERROR read (Float): ");
          Serial.print(this->_name);

          Serial.print(" IP: ");
          Serial.print(this->_ip);

          Serial.print(" Address: ");
          Serial.println(this->GetDeviceAddress());
          #endif
        }
      }
      else this->Error.Loop(true);

      return false;
    break;
    
    default:
      #ifdef DEBUG_ERRORS
      Serial.print("GenericPrgDevice ERROR (Unknown read function), channel: ");
      Serial.print(channel);
      Serial.print(", HWtype: ");
      Serial.print(this->_channels[channel].hwType);
      Serial.print(", Dev.Name: ");
      Serial.println(this->_name);

      Serial.print(" IP: ");
      Serial.print(this->_ip);

      Serial.print(" Address: ");
      Serial.println(this->GetDeviceAddress());
      #endif

      this->Error.Loop(true);
      return false;
    break;
  }
}

GenericPrgDevice::structRead GenericPrgDevice::Read(ModbusClient &mb, int channel, List<uint16_t> *items)
{
  structRead retVal;
  retVal.ok=false;

  if(channel>=this->_channelSize) {
    Serial.print("Generic Device READ ERROR, size<channel ");
    Serial.print(this->_name);

    Serial.print(" IP: ");
    Serial.print(this->_ip);

    Serial.print(" Address: ");
    Serial.println(this->GetDeviceAddress());

    Serial.print(" Size: ");
    Serial.print(this->_channelSize);

    Serial.print(" Channel: ");
    Serial.print(channel);

    this->Error.Loop(true);
    return retVal;
  }

  int startingAddr=this->_channels[channel].startingAddr;         //Inizializza default come indirizzo di partenza
  retVal.startIndex=0;

  if(this->_channels[channel].items< this->MAX_CALLS) 
    retVal.items= this->_channels[channel].items;   //Inizializza default come numero items
  else {
    //Se il numero di items è maggiore di quanto ne posso leggere ogni volta
    if(this->bank==0) {
      retVal.items=this->MAX_CALLS;
      this->bank+=1;
    } 
    else {
      int jump=this->bank * this->MAX_CALLS;
      retVal.startIndex=jump;

      startingAddr+= jump;
      retVal.items= this->MAX_CALLS;

      if(jump + retVal.items > this->_channels[channel].items) {
        int _items=abs(jump -this->_channels[channel].items);
        //Serial.print("items: "+String(_items)+", jump: "+String(jump));
        //Serial.println();

        if(_items==0) {
          retVal.startIndex=0;
          startingAddr=this->_channels[channel].startingAddr;  
        }
        else
          retVal.items=_items;

        this->bank=0;
      }
      else
        this->bank+=1;
    }
  }

  /*
  Serial.println();
  Serial.print("Generic Device READ::  ");
  Serial.print(this->_name);

  Serial.print(", Bank:");
  Serial.print(this->bank);

  Serial.print(", StartIndex:");
  Serial.print(retVal.startIndex);

  Serial.print(", startingAddr: ");
  Serial.print(startingAddr);

  Serial.print(", Items: ");
  Serial.print(retVal.items);

  Serial.print(", Channel Items: ");
  Serial.print(this->_channels[channel].items);
  Serial.println(); */

  switch(this->_channels[channel].hwType)
  {
    case Hold: //Hold
      if(!this->Error.IsInError()) {
        int tmpRead=mb.requestFrom(this->_deviceAddress, HOLDING_REGISTERS, startingAddr, retVal.items);
        if(tmpRead!=0) { 
          for (int i=0; i<tmpRead; i++)
            items->add(mb.read());
                      
          this->Error.Loop(false);
          retVal.ok=true;
        }
        else if(!this->Error.Loop(true)) {
          #ifdef DEBUG_ERRORS
          Serial.print("Generic Device ERROR (Hold): ");
          Serial.print(this->_name);

          Serial.print(" IP: ");
          Serial.print(this->_ip);

          Serial.print(" Address: ");
          Serial.println(this->GetDeviceAddress());
          #endif
        }
      }
      else this->Error.Loop(true);

    break;

  case Input: //Input Register
      if(!this->Error.IsInError()) {
        int tmpRead=mb.requestFrom(this->_deviceAddress, INPUT_REGISTERS, startingAddr, retVal.items);
        if(tmpRead!=0) { 
          for (int i=0; i<tmpRead; i++)
            items->add(mb.read());

          //Serial.print("GenericPrgDevice read input register: ");
          //Serial.println(value[0]);
          this->Error.Loop(false);
          retVal.ok=true;
        }
        else if(!this->Error.Loop(true)) {
          #ifdef DEBUG_ERRORS
          Serial.print("Generic Device ERROR (Input): ");
          Serial.print(this->_name);

          Serial.print(" IP: ");
          Serial.print(this->_ip);

          Serial.print(" Address: ");
          Serial.println(this->GetDeviceAddress());
          #endif
        }
      }
      else this->Error.Loop(true);
      
    break;

    case Discrete: //Discrete input
      if(!this->Error.IsInError()) {
        int tmpRead=mb.requestFrom(this->_deviceAddress, DISCRETE_INPUTS, startingAddr, retVal.items);
        if(tmpRead!=0){ 
          //Ok per una letturo di botto
          for (int i=0; i<tmpRead; i++)
            items->add(mb.read());
          
          this->Error.Loop(false);
          retVal.ok=true;
        }
        else if(!this->Error.Loop(true)) {
          #ifdef DEBUG_ERRORS
          Serial.print("Generic Device ERROR (Discrete): ");
          Serial.print(this->_name);

          Serial.print(" IP: ");
          Serial.print(this->_ip);

          Serial.print(" Address: ");
          Serial.println(this->GetDeviceAddress());
          #endif
        }
      }
      else this->Error.Loop(true);
      
    break;

    default:
      #ifdef DEBUG_ERRORS
      Serial.print("GenericPrgDevice ERROR (Unknown read function): ");
      Serial.println(this->_name);

      Serial.print(" IP: ");
      Serial.print(this->_ip);

      Serial.print(" Address: ");
      Serial.print(this->GetDeviceAddress());

      Serial.print(" Type: ");
      Serial.print(this->_channels[channel].hwType);

      Serial.print(" Channel: ");
      Serial.println(channel);
      #endif
      
      this->Error.Loop(true);
    break;
  }

  return retVal;
}

bool GenericPrgDevice::Write(ModbusClient &mb, int channel, int address, int value)
{
  if(this->_channels[channel].type==DO) {
    int tmpResult=0;

    switch(this->_channels[channel].hwType) {
      case Hold: //Hold
        tmpResult=mb.holdingRegisterWrite(this->_deviceAddress, this->_channels[channel].startingAddr, value);
        
        if(!this->Error.Loop(tmpResult==0)) {
          Serial.print("Generic Device ERROR (Write Holding register): ");
          Serial.println(this->_name);
          return false;
        }
        /*
        if(tmpResult!=0){
          Serial.print("OK (Write Holding register): ");
          Serial.println(this->_name);
        }*/

        return tmpResult!=0;
      break;

      case Coil:
        tmpResult=mb.coilWrite(this->_deviceAddress, address, value);
        
        if(!this->Error.Loop(tmpResult==0)) {
          Serial.print("Generic Device ERROR (Write coil): ");
          Serial.println(this->_name);
          return false;
        }
        /*
        if(tmpResult!=0){
          Serial.print("OK (Write coil): ");
          Serial.println(this->_name);
        }*/

        return tmpResult!=0;
      break;

      default:
        Serial.print("GenericPrgDevice ERROR (Unknown Write function): ");
        Serial.println(this->_name);

        Serial.print(" IP: ");
        Serial.print(this->_ip);

        Serial.print(" Address: ");
        Serial.println(this->GetDeviceAddress());
        return false;
      break;
    }
  }
  else {
    Serial.print("GenericPrgDevice write definition error: ");
    Serial.println(this->_deviceAddress);
    Serial.println(this->_channels[channel].startingAddr);
    Serial.println(value);

    return false;
  }
}

int GetDevicesByIp(arduino::IPAddress ip, std::vector<GenericPrgDevice> prgDevices, std::vector<GenericPrgDevice> &items)
{ 
  int foundId=0;
  for (auto& prgDevice : prgDevices) {
    if(prgDevice.GetIp()==ip) { 
      items.push_back(prgDevice);
      foundId++;
    }
  }
  return foundId;
}

int GetJump(GenericPrgDevicePriority priority) {
  switch (priority)   {
      case Low:
        return 1;
        break;

      case Medium:
        return 2;
      break;
      
      case Normal:
        return 3;
      break;

      default:
        return 0;
    }

    return 0;
}

int BuildIps(std::vector<GenericPrgDevice> prgDevices, List<structIP> *items)
{ 
  int foundId=0;
  for (auto& prgDevice : prgDevices) {
    if(items->getSize()==0) {
      structIP NewItem;
      NewItem.IP=prgDevice.GetIp();
      NewItem.Errors=0;
      NewItem.InError=false;

      List<PriorityMgmt> tmpPriorities;
      GetUsedPriorities(NewItem.IP, prgDevices, &tmpPriorities);
      for (int j=0; j<tmpPriorities.getSize(); j++)
          NewItem.Priority.Priorities[j]=tmpPriorities.get(j);

      NewItem.Priority.PrioritySize=tmpPriorities.getSize();
      NewItem.Priority.Index=0;
      items->add(NewItem);

      Serial.print("IP: " );
      Serial.print( NewItem.IP );
      Serial.print(", priorities " );
      Serial.println( NewItem.Priority.PrioritySize );
    }
    else {
      bool exist=false;
      for (int j=0; j<items->getSize(); j++) {
        if(items->get(j).IP== prgDevice.GetIp()) {
          exist=true;
          break;
        }
      }

      if(!exist) {
        structIP NewItem;
        NewItem.IP=prgDevice.GetIp();
        NewItem.Errors=0;
        NewItem.InError=false;

        List<PriorityMgmt> tmpPriorities;
        GetUsedPriorities(NewItem.IP, prgDevices, &tmpPriorities);
              
        for (int j=0; j<tmpPriorities.getSize(); j++)
          NewItem.Priority.Priorities[j]=tmpPriorities.get(j);

        NewItem.Priority.PrioritySize=tmpPriorities.getSize();
        NewItem.Priority.Index=0;
        items->add(NewItem);

        Serial.print("IP: " );
        Serial.print( NewItem.IP );
        Serial.print(", priorities " );
        Serial.println( NewItem.Priority.PrioritySize );
      }
    }
  }
  return items->getSize();
}

bool ExistDevicesByPriority(GenericPrgDevicePriority priority, arduino::IPAddress ip, std::vector<GenericPrgDevice> prgDevices)
{ 
  for (auto& prgDevice : prgDevices) {       
    if(prgDevice.GetPriority()==priority && prgDevice.GetIp()==ip) 
      return true;
  }

  return false;
}

int GetDevicesByPriority(GenericPrgDevicePriority priority, arduino::IPAddress ip, std::vector<GenericPrgDevice> prgDevices, std::vector<int> &items)
{ 
  int foundId=0;
  for (auto& prgDevice : prgDevices) {           
    if(prgDevice.GetPriority()==priority && prgDevice.GetIp()==ip) { 
      items.push_back(foundId);
    }
    foundId+=1;
  }

   return items.size();
}

int GetUsedPriorities(arduino::IPAddress ip, std::vector<GenericPrgDevice> prgDevices, List<PriorityMgmt> *items)
{ 
  int foundId=0;
  for (auto& prgDevice : prgDevices) {
    if(items->getSize()==0 && prgDevice.GetIp()==ip ) {
      PriorityMgmt tmp;
      tmp.Priority =prgDevice.GetPriority();
      tmp.DeviceIndex=-1; //Inizializza il ciclo
      items->add(tmp);
    }
    else {
      bool exist=false;
      for (int j=0; j<items->getSize(); j++) {
        if(items->get(j).Priority== prgDevice.GetPriority() && prgDevice.GetIp()==ip) {
          exist=true;
          break;
        }
      }

      if(!exist && prgDevice.GetIp()==ip){
        PriorityMgmt tmp;
        tmp.Priority=prgDevice.GetPriority();
        tmp.DeviceIndex=-1; //Inizializza il ciclo
        items->add(tmp);
      }
    }
  }
  return items->getSize();
}

////////////////////////////////////////////////////////// Valve
Valve::Valve(int address, unsigned long timeout=40000)
{ 
  this->_address=address;

  this->_timeOut=timeout;
  this->_status=Stopped;
}

void Valve::Open()
{
  this->_status=Opening;
  this->_startTime= millis();

   #ifdef DEBUG_ERROR
      Serial.println( "EV Open command");
    #endif    
}

void Valve::Close()
{
  this->_status=Closing;
  this->_startTime= millis();
   #ifdef DEBUG_ERROR
      Serial.println( "EV Close command");
    #endif 
}

void Valve::Stop()
{
  this->_status=Stopped;
  #ifdef DEBUG_ERROR
      Serial.println( "EV Stop command");
    #endif 
}

bool Valve::IsInError()
{
  return this->_status==Error;     
}

bool Valve::IsMoving()
{
  return this->_status==Opening || this->_status==Closing;     
}

bool Valve::IsInEOC()
{
  return this->_status==Opened || this->_status==Closed;     
}

int Valve::GetAddress()
{
  return this->_address;     
}

void Valve::Run(int switchStatus)
{
  //switchStatus==1 if open
  
  if(this->_status==Opening || this->_status==Closing) { 
    #ifdef DEBUG_ERROR
      Serial.print( "EV Switch ");
      Serial.println( switchStatus);
    #endif 

    if (millis() > this->_startTime + this->_timeOut) {   
      if((this->_status==Opening && switchStatus==0) || (this->_status==Closing && switchStatus==1))
        this->_status=Error;

         #ifdef DEBUG_ERROR
          Serial.print( "EV Status ");
          Serial.println( this->_status);
          Serial.println( "EV in ERROR");
        #endif 
    }
    else {
      if(this->_status==Opening && switchStatus==1){
        this->_status=Opened;

         #ifdef DEBUG_ERROR
          Serial.println( "EV Opened");
        #endif 
        
        }
      else if(this->_status==Closing && switchStatus==0){
        this->_status=Closed;

        #ifdef DEBUG_ERROR
          Serial.println( "EV Closed");
        #endif 
        }
    }
  }
}
