#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "ModbusConfig.h";
#include "EspFS.h";
#include "ModbusMaster.h"
#include <SoftwareSerial.h>

#define Num_of_Slaves 2 //Number of serial ports on the board
#define Num_of_Ops    5 //Number of operations per serial port 

//30 - is the maximum number of possible operations  
const size_t capacity = 2*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + Num_of_Ops*JSON_OBJECT_SIZE(6) + Num_of_Slaves*JSON_OBJECT_SIZE(11) + 830;
StaticJsonDocument<capacity> doc;

char* filename = "/modbus.cfg"; //You should upload the file to SPDIFF using the tool: https://github.com/esp8266/arduino-esp8266fs-

typedef struct{
  SoftwareSerial serial;  //default 
  ModbusMaster modbus;          //default 
} ModbusRTUSerial;

typedef std::vector<ModbusRTUSerial> modbusRTUSerials;

ModbusConfig modbusCfg;
EspFS fileSystem;

//Callback function to process polling interval
void pollingIntervalProcessor(Slave* slave, Operation* operation)
{
  if (slave)
  {
     if (operation == NULL)
     {
       Serial.println("Publish to the cloud. Slave connection is [" + String(slave->Connection) + "]. HwId: [" + String(slave->HwId) + "].");
     }
     else
     {
       Serial.println("Operation with name [" + String(operation->DisplayName) + "] has executed. Function: [0x0" + String(operation->Function, HEX) + "]. Address: [0x0" + String(operation->Address, HEX) + "].");
     }
  }
}

void readModbusConfig()
{
  bool res = fileSystem.loadTextFile(filename);
  if (res)
  {
    processModbusConfig(fileSystem.text);
  }
  else
  {
    Serial.println("File reading error.");
  }
}

void processModbusConfig(String json)
{
  if (modbusCfg.parseConfig(json))
  {
    modbusCfg.printConfig();
  }
}

void setup()
{
  Serial.begin(9600, SERIAL_8N1);

  //Serial.setRxBufferSize(128); //Change default 256 to 128
  
  //swSer.begin(9600);

  // Modbus slave ID 1
  //node.begin(Slave_ID, Serial);
  //node.begin(Slave_ID, swSer);

  
  modbusCfg.doc = new DynamicJsonDocument(capacity);
  modbusCfg.pollingIntervalCallback = &pollingIntervalProcessor;

  fileSystem.showDir();
  readModbusConfig();

  
  ModbusRTUSerial modbusRTU;
  modbusRTU->serial = new swSer(RX_PIN, TX_PIN, false, 128);
}

void loop()
{
  modbusCfg.loopModbusConfig();
}
