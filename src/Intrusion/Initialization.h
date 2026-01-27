#ifndef Initialization_H
#define Initialization_H

#pragma once

#include "Domo.h"
#include <vector>

// ************ Scheduler *******************************

bool coolDown(DomoManager* dm) {
  Serial.println("Cooling...");
  return true;
}

bool heatUp(DomoManager* dm) {
  static unsigned long start = 0;

  if (start == 0) {
      start = millis();
      Serial.println("Heating started");
  }

  if (millis() - start >= 50000) {
      Serial.println("Heating finished");
      start = 0;   // reset for next time
      return true; // step complete
  }

  return false; // still running
}

bool function3(DomoManager* dm) { 
  static unsigned long start = 0;

  if (start == 0) {
      start = millis();
      Serial.println("Long task started");
  }

  if (millis() - start >= 5000) {
      Serial.println("Long task finished");
      start = 0;   // reset for next time
      return true; // step complete
  }

  return false; // still running
}

void jobDone() { 
  Serial.println("Job completed!"); 
}

bool isHot(DomoManager* dm) { 
  BufferSourceInfo _sourceInfo;
  if(dm->GetBuffer().GetData(33, Field, _sourceInfo )) 
    return _sourceInfo.value>35;
  else
    return analogRead(A0) > 600; // example threshold 
}

bool myExternalSkipCondition(DomoManager* domo) { 
  // logica tua 
  return true; // oppure false
}

void DefineSchduleJobs(AsyncScheduler &scheduler) {
  
  //Job 0
  AsyncScheduler::Job _obArrivoACasa; 
  _obArrivoACasa.priority = 10; 
  _obArrivoACasa.onComplete = jobDone;

  // Step 0: Branch
  AsyncScheduler::Step branch;
  branch.type = AsyncScheduler::BRANCH_STEP;
  branch.description = "Verifica temperatura casa...";
  branch.condition = isHot;
  branch.thenStep = 1;  // go to cooling
  branch.elseStep = 2;  // go to heating
  _obArrivoACasa.steps.push_back(branch);

  // Step 1: Cooling
  AsyncScheduler::Step cool;
  cool.type = AsyncScheduler::NORMAL_STEP;
  cool.fnc = coolDown;
  cool.description = "Esecuzione ciclo raffreddamento";
  cool.delayAfterMs = 1000;
  _obArrivoACasa.steps.push_back(cool);

  // Step 2: Heating
  AsyncScheduler::Step heat;
  heat.type = AsyncScheduler::NORMAL_STEP;
  heat.fnc = heatUp;
  heat.description = "Esecuzione ciclo riscaldamento";
  heat.delayAfterMs = 1000;
  _obArrivoACasa.steps.push_back(heat);

  scheduler.addJob(_obArrivoACasa); 

  //Job 1
  AsyncScheduler::Job _jobTest; 
  _jobTest.priority = 10; 
  _jobTest.onComplete = jobDone;

  AsyncScheduler::Step dummy;
  dummy.type = AsyncScheduler::NORMAL_STEP;
  dummy.fnc = heatUp;
  dummy.description = "Heating step";
  dummy.delayAfterMs = 1000;
  _jobTest.steps.push_back(dummy);

  AsyncScheduler::Step longR;
  longR.type = AsyncScheduler::NORMAL_STEP;
  longR.fnc = function3;
  longR.description = "Prepara acqua calda";
  longR.delayAfterMs = 1000;
  longR.skipIf = myExternalSkipCondition;
  _jobTest.steps.push_back(longR);
 
  scheduler.addJob(_jobTest); 
}


// ************ DEVICE DEFINITION *******************************
GenericPrgDevice::GenericPrgDeviceChannel N4DIH32[]={ {GenericPrgDevice::DI, GenericPrgDevice::Hold, 128, 32, 1} }; //Scheda N4DIH32, 32 digital input
GenericPrgDevice::GenericPrgDeviceChannel MA01_XACX0440[]={ {GenericPrgDevice::DI, GenericPrgDevice::Discrete, 0, 4, 1}, {GenericPrgDevice::DO, GenericPrgDevice::Coil, 0, 4, 1} }; //Scheda MA01-AXCX4040, 4DI e 4DO Ebyte
GenericPrgDevice::GenericPrgDeviceChannel MA01_AXCX4020[]={ {GenericPrgDevice::DI, GenericPrgDevice::Discrete, 0, 4, 1}, {GenericPrgDevice::DO, GenericPrgDevice::Coil, 0, 2, 1} }; //Scheda MA01-AXCX4020, 4DI e 2DO Ebyte
GenericPrgDevice::GenericPrgDeviceChannel MA01_AXCX0080[]={ {GenericPrgDevice::DO, GenericPrgDevice::Coil, 0, 8, 1} }; //Scheda MA01-AXCX4020, 8DO Ebyte
GenericPrgDevice::GenericPrgDeviceChannel LE_01MQ[]={ {GenericPrgDevice::AI, GenericPrgDevice::Input, 0, 1, 2}, {GenericPrgDevice::AI, GenericPrgDevice::Input, 6, 1, 2}, {GenericPrgDevice::AI, GenericPrgDevice::Input, 12, 1, 2}, {GenericPrgDevice::AI, GenericPrgDevice::Input, 24, 1, 2}, {GenericPrgDevice::AI, GenericPrgDevice::Input, 70, 1, 2} }; //Scheda lettura consumi F&F LE-Q01MQ: Voltage, Current, Actoive Pw, Reactive pw, Frequency
GenericPrgDevice::GenericPrgDeviceChannel R421A08[]={ {GenericPrgDevice::DO, GenericPrgDevice::Hold, 1, 8, 1} }; //Scheda 8DO R421A08 Eletechsup, Close=256, Open=512
GenericPrgDevice::GenericPrgDeviceChannel GENERIC_4DI_4DO[]={ {GenericPrgDevice::DI, GenericPrgDevice::Discrete, 0, 4, 1}, {GenericPrgDevice::DO, GenericPrgDevice::Coil, 0, 4, 1} }; //Scheda generica, 4DI e 4DO
GenericPrgDevice::GenericPrgDeviceChannel CWT_SLTH_6W_S[]={ {GenericPrgDevice::AI, GenericPrgDevice::Hold, 0, 3, 1} }; //Scheda lettura Light, temp, Hum ComWinTop
GenericPrgDevice::GenericPrgDeviceChannel CWT_SLTH_6W_S_C[]={ {GenericPrgDevice::AI, GenericPrgDevice::Hold, 0, 3, 1} }; //Scheda lettura Hum, Temp, Light ComWinTop
GenericPrgDevice::GenericPrgDeviceChannel CWT_THCO_2K[]={ {GenericPrgDevice::AI, GenericPrgDevice::Hold, 0, 3, 1} }; //Scheda lettura Hum, Temp, CO ComWinTop
GenericPrgDevice::GenericPrgDeviceChannel R414A01[]={ {GenericPrgDevice::AI, GenericPrgDevice::Hold, 0, 2, 1} }; //Scheda lettura temp, Hum R414A01
GenericPrgDevice::GenericPrgDeviceChannel IR_210[]={ {GenericPrgDevice::AO, GenericPrgDevice::Hold, 109, 12, 1} }; //Scheda Emitter IR 6 canali, command (1-224), channel (1,2,4,8,16,32), ICPDas
GenericPrgDevice::GenericPrgDeviceChannel CTR4A01[]={ {GenericPrgDevice::AI, GenericPrgDevice::Input, 0, 1, 1} }; //Scheda lettura corrente CTR4A01
GenericPrgDevice::GenericPrgDeviceChannel PTA8C04[]={ {GenericPrgDevice::AI, GenericPrgDevice::Hold, 0, 4, 1} }; //Scheda lettura PT100 Eletechsup PTA8C04
GenericPrgDevice::GenericPrgDeviceChannel RESI_LED[]={ {GenericPrgDevice::DO, GenericPrgDevice::Hold, 0, 12, 1} }; //Scheda comando LED RESI 1-12 Luminosita 0-4095, 13-16 Led modes 0/1
GenericPrgDevice::GenericPrgDeviceChannel LSA_H4P40YBM[]={ {GenericPrgDevice::AO, GenericPrgDevice::Hold, 0, 4, 1}, {GenericPrgDevice::AO, GenericPrgDevice::Input, 0, 4, 1} }; //Scheda controllo potenza Loncont LSA-H4P40YBM, 0=Voltageout (R\W), 1=not used, 2=start\stop (R\W), 3=input frequency, 4=Voltage out, 5=Input signal, 6=output status, 7=Abnormal state indicator
GenericPrgDevice::GenericPrgDeviceChannel PROBE01[]={ {GenericPrgDevice::AI, GenericPrgDevice::Hold, 0, 2, 1} }; //Sensore generico temperatura/umidita


arduino::IPAddress WaveShareP1_Addr=IPAddress(192, 168, 12, 203);
arduino::IPAddress WaveSharePT_Addr=IPAddress(192, 168, 12, 204);
arduino::IPAddress WaveShareCantina_Addr=IPAddress(192, 168, 12, 205);

// ************ IO BUFFER *******************************
// 0..9 RESERVED

const int AREA_CUCINA_DOOR_ALM=0;
const int AREA_INGRESSO_DOOR_ALM=0;
const int AREA_CAMERA_WIN1_ALM=0;
const int AREA_CAMERA_WIN2_ALM=0;
const int AREA_CAMERA_WIN3_ALM=0;



const int AREA_PMETER_CONSUMPTION=10;

const int AREA_CAMERA_PIR_ALM=23;
const int AREA_CAMERA_PIR_TAMPER=24;
const int AREA_CAMERA_SMOKE_ALM=29;

const int AREA_PLAFONIERA_EXT=58;
const int AREA_CUCINA_PIR_ALM=98;
const int AREA_CUCINA_PIR_TAMPER=99;

const int SENSORE_CAMERA_CO=118;
const int SENSORE_CAMERA_TEMP=119;
const int SENSORE_CAMERA_HUM=120;

const int SENSORE_CUCINA_CORRENTE=122;
const int AREA_CUCINA_FLOOD_ALM=125;

const int AREA_ESTRATTORE_CUCINA=135;

const int RELAY_LED_BAGNO=151;


const int AREA_BAGNO_FLOOD_ALM=167;
const int BAGNO_IN_P1=168;
const int BAGNO_IN_P2=169;
const int BAGNO_IN_P3=170;

const int SENSORE_CUCINA_HUM=188;
const int SENSORE_CUCINA_TEMP=189;
const int SENSORE_CUCINA_LUX=190;

const int BAGNO_OUT_LED01R=191;
const int BAGNO_OUT_LED01G=192;
const int BAGNO_OUT_LED01W=193;
const int BAGNO_OUT_LED01B=194;

const int BAGNO_OUT_LED02=197;

// VIRTUAL, end phisical IO
const int AREA_CUCINA_PIR_CMD=220;
const int AREA_CUCINA_DOOR_CMD=221;

const int AREA_INGRESSO_DOOR_CMD=222;
const int AREA_CAMERA_SMOKE_CMD=223;
const int AREA_CAMERA_PIR_CMD=224;
const int AREA_CAMERA_WIN1_CMD=225;
const int AREA_CAMERA_WIN2_CMD=226;
const int AREA_CAMERA_WIN3_CMD=227;
const int AREA_CUCINA_SMOKE_CMD=228;
const int AREA_CUCINA_FLOOD_CMD=229;
const int AREA_BAGNO_FLOOD_CMD=230;

const int AREA_MEAN_TEMPS=231;

const int MAX_MDB_AREA=AREA_MEAN_TEMPS+1;

void initDevices(DomoManager &manager) { 
  const int RETRY_CNT=3;

  manager.addDevice("Lettore consumi - Quadro P1", WaveShareP1_Addr, 1, LE_01MQ, ARRAY_SIZE(MA01_AXCX4020), std::vector<int>{10, 11, 12, 13, 14}, RETRY_CNT, Low);
  manager.addDevice("Scheda 4DI+2DO - Fondo P1", WaveShareP1_Addr, 8, MA01_AXCX4020, ARRAY_SIZE(MA01_AXCX4020), std::vector<int>{15, 16, 17, 18, 19, 20}, RETRY_CNT, High);
  manager.addDevice("Scheda 32DI - Parete P1", WaveShareP1_Addr, 4, N4DIH32, ARRAY_SIZE(N4DIH32), std::vector<int>{21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52}, RETRY_CNT, High);
  manager.addDevice("Scheda 4DI+4DO - Scala", WaveShareP1_Addr, 2, MA01_XACX0440, ARRAY_SIZE(MA01_XACX0440), std::vector<int>{53,54,55,56,57,AREA_PLAFONIERA_EXT,59,60}, RETRY_CNT, High);
  //1 spazio
  manager.addDevice("Scheda 4DI+4DO - Quadro P1", WaveShareP1_Addr, 3, MA01_XACX0440, ARRAY_SIZE(MA01_XACX0440), std::vector<int>{62,63,64,65,66,67,68,69}, RETRY_CNT, High); 
  manager.addDevice("Scheda 4DI+4DO - Parete P1", WaveShareP1_Addr, 5, MA01_XACX0440, ARRAY_SIZE(MA01_XACX0440), std::vector<int>{70,71,72,73,74,75,76,77}, RETRY_CNT, Medium);
  manager.addDevice("Scheda 32DI - Fondo P1", WaveShareP1_Addr, 6, N4DIH32, ARRAY_SIZE(N4DIH32), std::vector<int>{78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,AREA_CUCINA_PIR_ALM,AREA_CUCINA_PIR_TAMPER,100,101,102,103,104,105,106,107,108,109}, RETRY_CNT, High);
  manager.addDevice("Scheda 8DO - Fondo P1", WaveShareP1_Addr, 7, MA01_AXCX0080, ARRAY_SIZE(MA01_AXCX0080), std::vector<int>{110,111,112,113,114,115,116,117}, RETRY_CNT, Low);
  manager.addDevice("Sensore CO/Temp/Hum - P1", WaveShareP1_Addr, 10, CWT_THCO_2K, ARRAY_SIZE(CWT_THCO_2K), std::vector<int>{SENSORE_CAMERA_CO,SENSORE_CAMERA_TEMP,SENSORE_CAMERA_HUM}, RETRY_CNT, Low);
  //1 spazio
  
  /*
  manager.addDevice("Sensore Corrente - Quadro cucina PT", WaveSharePT_Addr, 1, CTR4A01, ARRAY_SIZE(CTR4A01), std::vector<int>{SENSORE_CUCINA_CORRENTE}, RETRY_CNT, Low);
  manager.addDevice("Scheda 4DI+4DO - Quadro cucina PT", WaveSharePT_Addr, 3, MA01_XACX0440, ARRAY_SIZE(MA01_XACX0440), std::vector<int>{123,124,AREA_CUCINA_FLOOD_ALM,126,127,128,129,130},RETRY_CNT, High);
  manager.addDevice("Scheda 4DI+4DO - Parete cucina PT", WaveSharePT_Addr, 4, MA01_XACX0440, ARRAY_SIZE(MA01_XACX0440), std::vector<int>{131,132,133,134,AREA_ESTRATTORE_CUCINA,136,137,138}, RETRY_CNT, Low);
  manager.addDevice("Scheda 4DI+4DO - Corridoio PT", WaveSharePT_Addr, 5, MA01_XACX0440, ARRAY_SIZE(MA01_XACX0440), std::vector<int>{139,140,141,142,143,144,145,146}, RETRY_CNT, Low);
  manager.addDevice("Scheda 4AI+4DO - Corridoio PT", WaveSharePT_Addr, 6, MA01_XACX0440, ARRAY_SIZE(MA01_XACX0440), std::vector<int>{147,148,149,150,RELAY_LED_BAGNO,152,153,154}, RETRY_CNT, Low); //O: 0=Relay Led Bagno
  manager.addDevice("Scheda 32DI - Corridoio PT", WaveSharePT_Addr, 7, N4DIH32, ARRAY_SIZE(N4DIH32), std::vector<int>{155,156,157,158,159,160,161,162,163,164,165,166,AREA_BAGNO_FLOOD_ALM,BAGNO_IN_P1, BAGNO_IN_P2, BAGNO_IN_P3,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187}, RETRY_CNT, High); // 158-160=Pulsante 1, 2, 3; 157=Allagamento Bagno; 156= Pulsante Bagno
  manager.addDevice("Sensore Light/Temp/Hum - cucina PT", WaveSharePT_Addr, 10, CWT_THCO_2K, ARRAY_SIZE(CWT_THCO_2K), std::vector<int>{SENSORE_CUCINA_HUM,SENSORE_CUCINA_TEMP,SENSORE_CUCINA_LUX}, RETRY_CNT, Low);
  manager.addDevice("Gestore LED - Bagno", WaveSharePT_Addr, 8, RESI_LED, ARRAY_SIZE(RESI_LED), std::vector<int>{BAGNO_OUT_LED01R, BAGNO_OUT_LED01G, BAGNO_OUT_LED01B, BAGNO_OUT_LED01W, 195, 196, BAGNO_OUT_LED02, 198, 199, 200, 201, 202}, RETRY_CNT, Low); //33-44 Luminosita 0-4095, 13-16 Led modes 0/1
  */
  
  //manager.addDevice("Emettitore IR - Parete Bagno", WaveSharePT_Addr, 9, IR_210, ARRAY_SIZE(IR_210), std::vector<int>{203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214}, RETRY_CNT, Low); //1109-1119 Comando IR 1/224 per ogni uscita 1/6
  //manager.addDevice("Lettore temperature puffer - Quadro Cantina", WaveShareCantina_Addr, 1, PTA8C04, ARRAY_SIZE(MA01_AXCX4020), std::vector<int>{215, 216, 217, 218}, RETRY_CNT, Low); 
}

// ************ ALARMS *******************************
Sensor securitySensorCucina({
    SensorChannel(-1, INITIAL_DELAY, SensorChannelType::H24),
    SensorChannel(-1, RT_DELAY, SensorChannelType::RT)
});
Sensor doorSensorCucina({
    SensorChannel(-1, RT_DELAY, SensorChannelType::RT)
});
Sensor floodingSensorCucina({
    SensorChannel(-1, RT_DELAY, SensorChannelType::RT)
});
Sensor smokeSensorCucina({
    SensorChannel(-1, RT_DELAY, SensorChannelType::RT)
});

Sensor smokeSensorCamera({
    SensorChannel(-1, RT_DELAY, SensorChannelType::RT)
});
Sensor securitySensorCamera({
    SensorChannel(-1, INITIAL_DELAY, SensorChannelType::H24),
    SensorChannel(-1, RT_DELAY, SensorChannelType::RT)
});
Sensor windowSensorCamera1({
    SensorChannel(-1, RT_DELAY, SensorChannelType::RT)
});
Sensor windowSensorCamera2({
    SensorChannel(-1, RT_DELAY, SensorChannelType::RT)
});
Sensor windowSensorCamera3({
    SensorChannel(-1, RT_DELAY, SensorChannelType::RT)
});

Sensor floodingSensorBagno({
    SensorChannel(-1, RT_DELAY, SensorChannelType::RT)
});
Sensor doorSensorIngresso({
    SensorChannel(-1, RT_DELAY, SensorChannelType::RT)
});



void InitBuffer(DomoManager &manager) {

  // First 9 Areas are reserved, starts from 10.
  
  //Buffer
  // Lettura consumi ID=1
  manager.DefineBufferElement(AREA_PMETER_CONSUMPTION, 0, false, true, false, "Lettura consumo W");
  manager.DefineBufferElement(11, 0, false, true, false, "Lettura consumo 2");
  manager.DefineBufferElement(12, 0, false, true, false, "Lettura consumo 3");
  manager.DefineBufferElement(13, 0, false, true, false, "Lettura consumo 4");
  manager.DefineBufferElement(14, 0, false, true, false, "Lettura consumo 5");
  
  //**** 4 DI EbYTE ID=8 
  manager.DefineBufferElement(15, 0, false, false, false, "");
  manager.DefineBufferElement(16, 0, false, true, false, "");
  manager.DefineBufferElement(17, 0, false, false, false, "");
  manager.DefineBufferElement(18, 0, true, false, false, "Vasistas sx. chiuso");
   //2 DO EbYTE ID=8 
  manager.DefineBufferElement(19, 0, false, false, false, "Luce letto 1");
  manager.DefineBufferElement(20, 0, false, false, false, "Luce letto 2");

  //**** 32 DI EbYTE ID=4 (da 21 a 52)
  manager.DefineBufferElement(AREA_CAMERA_PIR_ALM, 0, true, false, true, "PIR Camera Allarme, NEGATO"); 
  manager.DefineBufferElement(AREA_CAMERA_PIR_TAMPER, 0, true, false, true, "Tamper PIR Camera, NEGATO"); 
  manager.DefineBufferElement(AREA_CAMERA_SMOKE_ALM, 0, true, false, true, "Sensore fumo Camera, NEGATO");
  manager.DefineBufferElement(30, 0, false, false, false, "");
  // Fino 52

  //SCALE
  //**** 4 DI EbYTE ID=2
  manager.DefineBufferElement(53, 0, false, false, false, "");
  manager.DefineBufferElement(54, 57, true, true, false, "Pulsante scala 3");
  manager.addToggle(54);
  manager.DefineBufferElement(55, DUMMY_AREA, true, true, false, "Pulsante scala 1");
  manager.DefineBufferElement(56, 59, true, true, false, "Pulsante scala 2 (Luce PT)");
  manager.addToggle(56);
  //4 DO EbYTE
  manager.DefineBufferElement(57, 0, false, false, false, "Luce scale P1");
  manager.DefineBufferElement(AREA_PLAFONIERA_EXT, 0, false, true, false, "Luce Ext.");
  manager.DefineBufferElement(59, 0, false, false, false, "Luce scale PT");
  manager.DefineBufferElement(60, 0, false, false, false, "");

  //QUADRO P1
  //**** 4 DI EbYTE ID=3
  manager.DefineBufferElement(62, 0, true, true, false, "Pulsante chiusura scuri");
  manager.DefineBufferElement(63, 75, true, true, false, "Pulsante trave 2");
  manager.addToggle(63);
  manager.DefineBufferElement(64, 76, true, true, false, "Pulsante trave 1");//66 - 102
  manager.addToggle(64);
  manager.DefineBufferElement(65, 0, true, true, false, "Pulsante apertura scuri");
   //4 DO EbYTE
  manager.DefineBufferElement(66, 0, false, false, false, "Buzzer");
  manager.DefineBufferElement(67, 0, false, false, false, "Open/close scuro");
  manager.DefineBufferElement(68, 0, false, false, false, "On/off scuro");
  manager.DefineBufferElement(69, 0, false, false, false, "");

  //PARETE P1
  //**** 4 DI EbYTE ID=5
  //60-63 INPUT nc
  manager.DefineBufferElement(70, 0, false, false, false, "");
  manager.DefineBufferElement(71, 0, false, false, false, "");
  manager.DefineBufferElement(72, 0, false, false, false, "");
  manager.DefineBufferElement(73, 0, false, false, false, "");
  //4 DO EbYTE ID=5  
  manager.DefineBufferElement(74, 0, false, false, false, "Led binario"); 
  manager.DefineBufferElement(75, 0, false, false, false, "Led trave 2"); 
  manager.DefineBufferElement(76, 0, false, false, false, "Led trave 1"); 
  manager.DefineBufferElement(77, 0, false, false, false, ""); 
  
    //**** 32 DI EbYTE ID=6 - Pulsanti Letto e sensori 
  //0, 1, 2, 3= Sensore tetto antenna
  manager.DefineBufferElement(78, 0, true, true, false, "Pulsante Apertura serranda vasistas sx.");
  manager.DefineBufferElement(79, 0, true, true, false, "Pulsante Chiusura serranda vasistas sx.");
  manager.DefineBufferElement(80, 0, true, true, false, "Pulsante Apertura vasistas sx.");
  manager.DefineBufferElement(81, 0, true, true, false, "Pulsante Chiusura vasistas sx.");

  manager.DefineBufferElement(82, DUMMY_AREA, false, false, false, "Pulsante Letto sx. 1"); //-------A
  manager.DefineBufferElement(83, 77, true, true, false, "Pulsante Letto sx. 2"); //19
  manager.addToggle(83); 

  manager.DefineBufferElement(86, 0, true, true, false, "Pulsante Apertura serranda vasistas dx.");
  manager.DefineBufferElement(87, 0, true, true, false, "Pulsante Chiusura serranda vasistas dx.");
  manager.DefineBufferElement(88, 0, true, true, false, "Pulsante Apertura vasistas dx.");
  manager.DefineBufferElement(89, 0, true, true, false, "Pulsante Chiusura vasistas dx.");

  manager.DefineBufferElement(90, 74, true, true, false, "Pulsante Letto dx. 1"); //-------A
  manager.addToggle(90, {82, 55} ); //Dipende anche da Area 82 e 55

  manager.DefineBufferElement(91, 20, true, true, false, "Pulsante Letto dx. 2");
  manager.addToggle(91); 
   
  manager.DefineBufferElement(AREA_CUCINA_PIR_ALM, 0, true, false, true, "PIR Cucina Allarme, NEGATO"); 
  manager.DefineBufferElement(AREA_CUCINA_PIR_TAMPER, 0, true, false, true, "Tamper PIR Cucina, NEGATO"); 
  manager.DefineBufferElement(100, 0, false, false, false, "");
  manager.DefineBufferElement(101, 0, false, false, false, "");

  manager.DefineBufferElement(106, 0, true, true, false, "Pulsante cucina fondo 1");
  manager.DefineBufferElement(107, 0, true, true, false, "Pulsante cucina fondo 2");
  manager.DefineBufferElement(109, 0, true, true, false, "Pulsante cucina fondo 3");

  //FONDO
  //**** 8 DO EbYTE ID=7 - Vasistas e scuri
  manager.DefineBufferElement(110, 0, false, false, false, "");
  manager.DefineBufferElement(111, 0, false, false, false, "");
  manager.DefineBufferElement(112, 0, false, false, false, "");
  manager.DefineBufferElement(113, 0, false, false, false, "");
  manager.DefineBufferElement(114, 0, false, false, false, "");
  manager.DefineBufferElement(115, 0, false, false, false, "");
  manager.DefineBufferElement(116, 0, false, false, false, "");
  manager.DefineBufferElement(117, 0, false, false, false, "");

  //Sensore ID=10
  manager.DefineBufferElement(SENSORE_CAMERA_CO, 0, true, false, false, "Co"); 
  manager.DefineBufferElement(SENSORE_CAMERA_TEMP, 0, true, false, false, "Temperatura"); 
  manager.DefineBufferElement(SENSORE_CAMERA_HUM, 0, true, false, false, "Umidita"); 

  
 
  //*****************************************************************************************
  //PIANO TERRA
  manager.DefineBufferElement(SENSORE_CUCINA_CORRENTE, 0, false, false, false, "Sensore corrente Induzione"); 

  //**** 4 DI EbYTE
  manager.DefineBufferElement(123, 127, true, true, false, "Pulsante Cucina 1"); 
  manager.addToggle(123);

  manager.DefineBufferElement(124, 129, true, true, false, "Pulsante Cucina 2"); 
  manager.addToggle(124);

  manager.DefineBufferElement(AREA_CUCINA_FLOOD_ALM, 0, false, false, false, "Allagamento Cucina");
  manager.DefineBufferElement(126, 0, false, false, false, ""); 
  
   //4 DO EbYTE 
  manager.DefineBufferElement(127, 0, false, false, false, "Luce cucina 1");  
  manager.DefineBufferElement(128, 0, true, true, false, "Bluetooth");  
  manager.DefineBufferElement(129, 0, false, false, false, "Luce cucina 2");  
  manager.DefineBufferElement(130, 0, false, false, false, "");

//**** 4 DI EbYTE ID=4 Parete Cucina
  manager.DefineBufferElement(131, 0, true, false, false, "Valvola calorifero bagno NC");  
  manager.DefineBufferElement(132, 0, true, false, false, "Valvola calorifero bagno NO");  
  manager.DefineBufferElement(133, 0, false, false, false, "");
  manager.DefineBufferElement(134, 0, false, false, false, "");
   //4 DO EbYTE ID=4 - 
  manager.DefineBufferElement(AREA_ESTRATTORE_CUCINA, 0, true, true, false, "Estrattore");  
  manager.DefineBufferElement(136, 0, true, false, false, "Valvola calorifero bagno ON");  
  manager.DefineBufferElement(137, 0, true, false, false, "Valvola calorifero bagno OFF");  
  manager.DefineBufferElement(138, 0, true, true, false, "Led Fondo"); 

//**** 4 DI EbYTE ID=5 Corridoio PT
  manager.DefineBufferElement(139, 0, true, true, false, "Valvola acqua casa APERTA");  
  manager.DefineBufferElement(140, 0, false, false, false, "");  
  manager.DefineBufferElement(141, 0, false, false, false, "");
  manager.DefineBufferElement(142, 0, false, false, false, "");
   //4 DO
  manager.DefineBufferElement(143, 0, false, false, false, "Alimentatore LED bagno");  
  manager.DefineBufferElement(144, 0, false, true, false, "Valvola acqua casa Alim (ON)");  
  manager.DefineBufferElement(145, 0, false, true, false, "Valvola acqua casa OFF");  
  manager.DefineBufferElement(146, 0, true, true, false, "Estrattore bagno"); 

  //**** 4 AI EbYTE ID=6
  manager.DefineBufferElement(147, 0, false, false, false, "");  
  manager.DefineBufferElement(148, 0, false, false, false, "");  
  manager.DefineBufferElement(149, 0, false, false, false, "");
  manager.DefineBufferElement(150, 0, false, false, false, "");
   //4 DO
  manager.DefineBufferElement(RELAY_LED_BAGNO, 0, false, false, false, "");  
  manager.DefineBufferElement(152, 0, false, false, false, "");  
  manager.DefineBufferElement(153, 0, false, false, false, "");  
  manager.DefineBufferElement(154, 0, false, false, false, ""); 
 
   //**** 32 DI EbYTE ID=7 - Parete bagno su scala cantina, 155-187
  manager.DefineBufferElement(166, 0, true, true, false, "Pulsante bagno"); 
  manager.DefineBufferElement(AREA_BAGNO_FLOOD_ALM, 0, true, false, false, "Sensore allagamento bagno"); 
  manager.DefineBufferElement(BAGNO_IN_P1, DUMMY_AREA, true, true, false, "Int. Luce bagno"); //Essendo riversato mettere in out valore fittizio x renderlo comandabile
  manager.addToggle(BAGNO_IN_P1);

  manager.DefineBufferElement(BAGNO_IN_P2, DUMMY_AREA, true, true, false, "Int. Luce corridoio"); //Essendo riversato mettere in out valore fittizio x renderlo comandabile
  manager.addToggle(BAGNO_IN_P2, {57}); //Riversa verso interruttore piano 1
  
  manager.DefineBufferElement(BAGNO_IN_P3, 0, true, true, false, "Int. Luce ingresso");
  manager.addToggle(BAGNO_IN_P3);

  //Sensore ID=10
  manager.DefineBufferElement(SENSORE_CUCINA_HUM, 0, true, false, false, "Umidita"); 
  manager.DefineBufferElement(SENSORE_CUCINA_TEMP, 0, true, false, false, "Temperatura"); 
  manager.DefineBufferElement(SENSORE_CUCINA_LUX, 0, true, false, false, "Luminosita"); 

  //To call at definition ends
  manager.initBuffer();
}

DomoManager Manager(MAX_MDB_AREA, initDevices, InitBuffer,LED_D0, LED_D1, LED_D2, LED_D3);
#endif
