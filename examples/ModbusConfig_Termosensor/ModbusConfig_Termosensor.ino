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

char* filename = "/modbus.cfg"; //You should upload the file to SPIFFS using the tool: https://github.com/esp8266/arduino-esp8266fs-plugin

/*typedef struct{
  ModbusMaster modbus;
  int SlaveId;
} ModbusMasterInfo;*/


class ModbusMasterEx : public ModbusMaster 
{
public:
  int SlaveId = -1;
  ModbusMasterEx() : ModbusMaster(){}
};

class SoftwareSerialEx : public SoftwareSerial
{
public:
  int RxPin = -1;
  int TxPin = -1;
  int BaudRate = -1;
  SoftwareSerialEx(int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize) : SoftwareSerial(receivePin, transmitPin, inverse_logic, buffSize){}
};

//typedef std::vector<ModbusMasterTest> ModbusConnectors;
typedef std::vector<ModbusMasterEx*> ModbusConnectors;

class ModbusRTUConnector 
{
public:
  SoftwareSerialEx *serial;  //default 
  ModbusConnectors Connectors; //default 
  String tag;
  ModbusRTUConnector() {}
};

/*typedef struct{
  SoftwareSerialEx *serial;  //default 
  ModbusConnectors Connectors; //default 
  String tag;
} ModbusRTUConnector;*/

typedef std::vector<ModbusRTUConnector*> Connections;

//#define Slave_ID    1 
#define RX_PIN      5 //D1 15  //D8
#define TX_PIN      13  //D7

// instantiate ModbusMaster object
//ModbusMaster node;

ModbusConfig modbusCfg;
EspFS fileSystem;

Connections connectors;

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
       getInputRegistersResult(slave->Connector, *operation);
     }
     //Serial.printf("Connector: %p\n", slave->Connector);
     //ModbusRTUConnector* p = static_cast<ModbusRTUConnector*>(slave->Connector);
     //Serial.println("Slave str: [" + (p->tag) + "].");
     //SoftwareSerialEx* d = static_cast<SoftwareSerialEx*>(p->serial);
     //Serial.println("Serial speed: " + String(d->BaudRate) + ", Rx pin: " + String(d->RxPin) + ", Tx pin: " + String(d->TxPin));
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
    
    connectors.clear(); 
    
    int i = 0;
    for (Slave& slave : modbusCfg.slaves)
    {
      if (slave.Type == ModbusType::RTU)
      {
        ModbusRTUConnector* modbusRTU = new ModbusRTUConnector();
        modbusRTU->serial = new SoftwareSerialEx(slave.RxPin, slave.TxPin, false, 128);
        modbusRTU->serial->begin(slave.BaudRate);
        modbusRTU->serial->BaudRate = slave.BaudRate;
        modbusRTU->serial->RxPin = slave.RxPin;
        modbusRTU->serial->TxPin = slave.TxPin;
        modbusRTU->tag = "Slave_" + String(i);
        int j = 0;
        for (Operation& operation : slave.Operations)
        {
          ModbusMasterEx* modbus = isSlaveIdInQueue(*modbusRTU, operation.SlaveId);
          if (modbus->SlaveId == -1)
          {
            //modbus = new ModbusMasterEx();
            modbus->SlaveId = operation.SlaveId;
            modbus->begin(operation.SlaveId, *(modbusRTU->serial));
           //modbus.str = "Operation_" + String(j);
          }
          Serial.println("Operation with SlaveID: " + String(modbus->SlaveId) + " Function: " + String(operation.Function) + " Address: " + String(operation.Address));
          Serial.printf("Pointer to modbus object: %x\r\n", &modbus);
          modbusRTU->Connectors.push_back(modbus);
          j++;   
        }
        connectors.push_back(modbusRTU);

        ModbusRTUConnector* k =  connectors.back();
        Serial.println("Back " + k->tag);
//        slave.Connector = &connectors.front();
//        void* p = &connectors.front();
//        Serial.print("String: " + modbusRTU.str);
//        Serial.printf(" with back() = %p and front() = %p\n", slave.Connector, p);
        i++;
      }
     }

    int j = 0; //vector can reallocate elements during the filling, so, addresses can be assign only after finishing vector filling
    for (ModbusRTUConnector* connector : connectors)
    {
      Serial.print("String: " + connector->tag);
      Serial.printf(" with address %p\n", &connector);
      modbusCfg.slaves[j].Connector = connector;  
      int k = 0;
      for (ModbusMasterEx* modbus : connector->Connectors)
      {
        Serial.print("Modbus with SlaveId: " + String(modbus->SlaveId));
        Serial.printf(" with address %p\n", modbus);
        modbusCfg.slaves[j].Operations[k].Modbus = modbus;
        k++;
      }
      j++;
    }    

    //connectors.shrink_to_fit();
    
    /*for (Slave& slave : modbusCfg.slaves)
    {
     ModbusRTUConnector* p = static_cast<ModbusRTUConnector*>(slave.Connector);
     Serial.print("Slave str: [" + (p->tag) + "].");
     Serial.printf(" with address %p\n", slave.Connector);
     for (Operation& operation : slave.Operations)
     {
         //String res = operation.DisplayName + " C";
         //Serial.println(res);
     }
    }*/
  }
}

void setup()
{
  Serial.begin(9600, SERIAL_8N1);

  //Serial.setRxBufferSize(128); //Change default 256 to 128
  
  //swSer.begin(9600);
  //modbus1.begin(1, swSer);


  // Modbus slave ID 1
  //node.begin(Slave_ID, Serial);
  //node.begin(Slave_ID, swSer);

  
  modbusCfg.doc = new DynamicJsonDocument(capacity);
  modbusCfg.pollingIntervalCallback = *pollingIntervalProcessor;

  //fileSystem.showDir();
  readModbusConfig();

  
  //modbusRTU.serial = &swSer;
}

void loop()
{
  modbusCfg.loopModbusConfig();
  //getModbus();
  //delay(1000);
}

ModbusRTUConnector modbusRTU;
void* modbusConnector;
//ModbusMasterEx* node;

bool isInitiated= false;
void getModbus()
{
  if (!isInitiated)
  {
  modbusRTU.serial = new SoftwareSerialEx(RX_PIN, TX_PIN, false, 128);
  modbusRTU.serial->begin(9600);
  modbusRTU.serial->BaudRate = 9600;
  modbusRTU.serial->RxPin = RX_PIN;
  modbusRTU.serial->TxPin = TX_PIN;
  modbusRTU.tag = "Slave_0";
  ModbusMasterEx* m = new ModbusMasterEx();
  m->SlaveId = 1;
  Serial.printf("Address1 %p, SlaveId %u\r\n", m, m->SlaveId);
  m->begin(1, *(modbusRTU.serial));
  modbusRTU.Connectors.push_back(m);

  //ModbusMasterEx* arr[1] = {m};

  ModbusMasterEx* modbus = isSlaveIdInQueue(modbusRTU, 1);
  //modbus = modbusRTU.Connectors.back(); //arr[0];//m; //&

  Serial.printf("Address2 %p, SlaveId %u\r\n", modbus, modbus->SlaveId);
  //modbus = new ModbusMasterEx();
  //modbus->SlaveId = 1;
  //modbus->begin(1, *(modbusRTU.serial));
  modbusConnector = modbus;
  isInitiated = true;
  }  
  
  Serial.println("Operation with SlaveID: " + String(1) + " Address: 1");
  ModbusMasterEx* node = static_cast<ModbusMasterEx*>(modbusConnector);


  if (node)
  {
  ESP.wdtDisable();
  uint8_t result = node->readInputRegisters(1, 1);
  ESP.wdtEnable(1);
  
  if (getResultMsg(*node, result))
  {
    Serial.println();

    String res = "Temp: ";
    int j = 0;
    double res_dbl = node->getResponseBuffer(j);
    res += " " + String(res_dbl) + "\r\n";
    Serial.println(res);
  }
  }
}

ModbusMasterEx* isSlaveIdInQueue(ModbusRTUConnector connector, int SlaveId)
{
  for (ModbusMasterEx* modbus : connector.Connectors)
  {
    if (modbus->SlaveId == SlaveId)
    {
      Serial.println("The Slave ID is found. Set the pointer to the found one.");
      return modbus;
    }
  }  
  return (new ModbusMasterEx());
}

void getInputRegistersResult(void* modbusConnector, Operation operation)
{
  if (modbusConnector)
  {
    ModbusMasterEx* node = static_cast<ModbusMasterEx*>(modbusConnector);

    Serial.println("Read Input Registers from Slave ID: " + String(node->SlaveId) + " with address: " + String(operation.Address) + " and len: " + String(operation.Len));
    ESP.wdtDisable();
    uint8_t result = node->readInputRegisters(operation.Address, operation.Len);
    ESP.wdtEnable(1);
  
    if (getResultMsg(*node, result))
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

bool getResultMsg(ModbusMaster node, uint8_t result)
{
  String tmpstr2;

  switch (result) {
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
