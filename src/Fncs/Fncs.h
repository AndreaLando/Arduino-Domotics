#ifndef Fncs_H
#define Fncs_H

#include <Arduino.h>
#include "PLC.h"
#include "Buffers.h"
#include <vector>

/////////////////////////MODBUS TCP INIT
#include <SPI.h>
#include <Ethernet.h>

//Modbus Client, uso libreria ARDUINO
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

//Modbus Server Pannello, uso libreria MgsModbus
#include "MgsModbus.h"

typedef void (*SomethingChangedFn)(ModbusBuffer &); 
typedef void (*RouteFn)(BufferSourceInfo, int, ModbusBuffer &);

void ManageMdbSvr(pin_size_t led, EthernetClient &client, MgsModbus &modbusTCPSvr, ModbusBuffer &buffer, ToggleManager &toggles, char *itemName, bool mode);
bool ManageMdbCli(pin_size_t ledR, pin_size_t ledW, ModbusTCPClient &modbusTCPCli, List<structIP> *IPList, short ipIndex, ModbusBuffer &buffer, std::vector<GenericPrgDevice> &prgDevices, ToggleManager &toggles, SomethingChangedFn somethingChanged, RouteFn route);

bool DeviceManagement_Write(pin_size_t led, ModbusTCPClient &modbusTCPCli, arduino::IPAddress ip, ModbusBuffer &buffer, std::vector<GenericPrgDevice> prgDevices);
bool DeviceManagement_Read(pin_size_t led, ModbusTCPClient &modbusTCPCli, List<structIP> *iPList, short ipIndex, ModbusBuffer &buffer, std::vector<GenericPrgDevice> &prgDevices, ToggleManager &toggles);
#endif
