/* 
 *  The example shows how to work with a single RS485 interface using Modbus RTU and configure by ModbusConfig.
    ESP8266 microcontroller 
*/ 

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "ModbusConfig.h";
#include "EspFS.h";
#include "ModbusMaster.h"
#include <SoftwareSerial.h>

#define Num_of_Connections 2 //Number of serial ports on the board
#define Num_of_Ops    5 //Number of operations per serial port 

//30 - is the maximum number of possible operations  
const size_t capacity = 2*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + Num_of_Ops*JSON_OBJECT_SIZE(6) + Num_of_Connections*JSON_OBJECT_SIZE(11) + 830;
StaticJsonDocument<capacity> doc;

char* filename = "/modbus.cfg"; //You should upload the file to SPIFFS using the tool: https://github.com/esp8266/arduino-esp8266fs-plugin


class ModbusMasterEx : public ModbusMaster 
{
public:
  int SlaveId = -1;
  ModbusMasterEx() : ModbusMaster(){}

  bool getResultMsg(ModbusMaster node, uint8_t result)
  {
    String tmpstr2;

    switch (result)
    {
    case node.ku8MBSuccess:
      return true;
      break;
    case node.ku8MBIllegalFunction:
      tmpstr2 = "Illegal Function";
      break;
    case node.ku8MBIllegalDataAddress:
      tmpstr2 = "Illegal Data Address";
      break;
    case node.ku8MBIllegalDataValue:
      tmpstr2 = "Illegal Data Value";
      break;
    case node.ku8MBSlaveDeviceFailure:
      tmpstr2 = "Slave Device Failure";
      break;
    case node.ku8MBInvalidSlaveID:
      tmpstr2 = "Invalid Slave ID";
      break;
    case node.ku8MBInvalidFunction:
      tmpstr2 = "Invalid Function";
      break;
    case node.ku8MBResponseTimedOut:
      tmpstr2 = "Response Timed Out";
      break;
    case node.ku8MBInvalidCRC:
      tmpstr2 = "Invalid CRC";
      break;
    default:
      tmpstr2 = "Unknown error: " + String(result);
      break;
    }
    Serial.println(tmpstr2);
    return false;
  }
};

class SoftwareSerialEx : public SoftwareSerial
{
public:
  int RxPin = -1;
  int TxPin = -1;
  int BaudRate = -1;
  SoftwareSerialEx(int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize) : SoftwareSerial(receivePin, transmitPin, inverse_logic, buffSize){}
};

typedef std::vector<ModbusMasterEx*> ModbusConnectors;

class ModbusRTUConnector 
{
public:
  SoftwareSerialEx *serial; 
  ModbusConnectors* Connectors; 
  String tag; //Field just to test
  ModbusRTUConnector() {}
};

//typedef std::vector<ModbusRTUConnector*> Connections;

ModbusConfig modbusCfg;
EspFS fileSystem;

//Connections connectors;
ModbusRTUConnector* Connector;

//Callback function to process polling interval
void pollingIntervalProcessor(Connection* connection, Operation* operation)
{
  if (connection)
  {
     if (operation == NULL)
     {
       Serial.println("Publish telemetry data to the cloud. Slave connection is [" + String(connection->Connection) + "]. HwId: [" + String(connection->HwId) + "].");
     }
     else
     {
       Serial.println("Operation with name [" + String(operation->DisplayName) + "] has executed. Function: [0x0" + String(operation->Function, HEX) + "]. Address: [0x0" + String(operation->Address, HEX) + "].");
       getInputRegistersResult(*operation);
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

    Connector = new ModbusRTUConnector();
    
    int i = 0;
    for (Connection& connection : modbusCfg.connections)  //Only one Slave settings supported in the example
    {
      if (connection.Type == ModbusType::RTU)
      {
        Connector->serial = new SoftwareSerialEx(connection.RxPin, connection.TxPin, false, 256); 
        Connector->serial->begin(connection.BaudRate);
        Connector->serial->BaudRate = connection.BaudRate;
        Connector->serial->RxPin = connection.RxPin;
        Connector->serial->TxPin = connection.TxPin;
        ModbusConnectors* connectors = new ModbusConnectors();  
        for (Operation& operation : connection.Operations)
        {
          ModbusMasterEx* modbus = new ModbusMasterEx();
          modbus->begin(operation.SlaveId, *Connector->serial);
          modbus->SlaveId = operation.SlaveId;
          operation.Modbus = modbus;
          connectors->push_back(modbus);
        }
        Connector->Connectors = connectors;
        connection.Connector = Connector;
      }
    }
  }
}

void setup() {
  Serial.begin(9600, SERIAL_8N1);

  modbusCfg.doc = new DynamicJsonDocument(capacity);
  modbusCfg.pollingIntervalCallback = *pollingIntervalProcessor;

  Serial.println("Try to read Modbus config");
  readModbusConfig();
}

void loop() {
  modbusCfg.loopModbusConfig();
}

void getInputRegistersResult(Operation operation)
{
  if (operation.Modbus)
  {
    ModbusMasterEx* node = static_cast<ModbusMasterEx*>(operation.Modbus);

    Serial.println("Read Input Registers from Slave ID: " + String(node->SlaveId) + " with address: " + String(operation.Address) + " and len: " + String(operation.Len));
    ESP.wdtDisable();
    uint8_t result = node->readInputRegisters(operation.Address, operation.Len);
    ESP.wdtEnable(1);
  
    if (node->getResultMsg(*node, result))
    {
      Serial.println();

      String res = operation.DisplayName;
      int j = 0;
      //for (j = 0; j < operation.Len; j++)
      {
        double res_dbl = node->getResponseBuffer(j);
        res += " " + String(res_dbl) + "\r\n";
      }
      Serial.println(res);
    }
  }
}