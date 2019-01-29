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
  SoftwareSerial *serial;  //default 
  ModbusMaster *modbus;          //default 
  String str;
} ModbusConnector;

typedef std::vector<ModbusConnector> ModbusConnectors;

#define Slave_ID    1 
#define RX_PIN      15  //D8
#define TX_PIN      13  //D7

// instantiate ModbusMaster object
ModbusMaster node;

ModbusConfig modbusCfg;
EspFS fileSystem;

ModbusConnectors connectors;

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
     //Serial.printf("Connector: %p\n", slave->Connector);
     ModbusConnector* p = static_cast<ModbusConnector*>(slave->Connector);
     Serial.println("Slave str: [" + (p->str) + "].");
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

    int i = 0;
    for (Slave& slave : modbusCfg.slaves)
    {
      if (slave.Type == ModbusType::RTU)
      {
        ModbusConnector modbusRTU;
        modbusRTU.serial = new SoftwareSerial(slave.RxPin, slave.TxPin, false, 128);
        modbusRTU.serial->begin(slave.BaudRate);
        modbusRTU.modbus = new ModbusMaster();
        modbusRTU.modbus->begin(Slave_ID, *modbusRTU.serial);
        modbusRTU.str = "Test" + String(i);
        connectors.push_back(modbusRTU);
        slave.Connector = &connectors.front();
        void* p = &connectors.front();
        Serial.print("String: " + modbusRTU.str);
        Serial.printf(" with back() = %p and front() = %p\n", slave.Connector, p);
        i++;
      }
     }

     /*int j = 0;
     for (ModbusConnector& connector : connectors)
      {
        Serial.print("String: " + connector.str);
        Serial.printf(" with address %p\n", &connector);
        modbusCfg.slaves[j].Connector = &connector;
        j++;
      }*/    

    connectors.shrink_to_fit();
    
    for (ModbusConnector& modbusRTU : connectors)
    {
      Serial.print("String: " + modbusRTU.str);
      Serial.printf(" with address %p\n", &modbusRTU);
    }
    for (Slave& slave : modbusCfg.slaves)
    {
     ModbusConnector* p = static_cast<ModbusConnector*>(slave.Connector);
     Serial.print("Slave str: [" + (p->str) + "].");
     Serial.printf(" with address %p\n", slave.Connector);
    }
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
  modbusCfg.pollingIntervalCallback = *pollingIntervalProcessor;

  fileSystem.showDir();
  readModbusConfig();

  
  //modbusRTU.serial = &swSer;
}

void loop()
{
  modbusCfg.loopModbusConfig();
}
