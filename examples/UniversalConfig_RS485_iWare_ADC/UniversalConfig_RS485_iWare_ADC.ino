#include <ArduinoJson.h>
/*
 * ModbusConfig test example for ESP8266 boards. For Arduino just remove #include <ESP8266WiFi.h> 
*/

//#include <ESP8266WiFi.h>
#include "ModbusConfig.h";
#include "EspFS.h";

#define Num_of_Slaves 2 //Number of serial ports on the board
#define Num_of_Ops    5 //Number of operations per serial port 

//30 - is the maximum number of possible operations https://arduinojson.org/v6/assistant/ 
//const size_t capacity = 2*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + Num_of_Ops*JSON_OBJECT_SIZE(6) + Num_of_Slaves*JSON_OBJECT_SIZE(11) + 830;
const size_t capacity = 2*JSON_ARRAY_SIZE(1) + 5*JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(3) + 4*JSON_OBJECT_SIZE(5) + 2*JSON_OBJECT_SIZE(7) + 2*JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(12) + 1240*3;

//StaticJsonDocument<capacity> doc;
DynamicJsonDocument doc(capacity);

char* filename = "/modbus.cfg"; //You should upload the file to SPDIFF using the tool: https://github.com/esp8266/arduino-esp8266fs-plugin

ModbusConfig modbusCfg;
EspFS fileSystem;

//Callback function to process polling interval
void pollingIntervalProcessor(BaseConnection* connection, BaseOperation* operation)
{
  if (connection)
  {
     switch (connection->Sensor)
     {
        case SensorType :: Modbus:
        {
          ModbusConnection* conn = static_cast<ModbusConnection*>(connection);
          if (operation == NULL)
          {
            Serial.println("Publish to the cloud. Slave connection is [" + String(connection->Connection) + "]. HwId: [" + String(conn->HwId) + "].");
          }
          else
          {
            ModbusOperation* op = static_cast<ModbusOperation*>(operation);
            Serial.println("Operation with name [" + String(operation->DisplayName) + "] has executed. Function: [0x0" + String(op->Function, HEX) + "]. Address: [0x0" + String(op->Address, HEX) + "].");
          }
        };
        break;
        case SensorType :: iWare:
        {
          iWareConnection* conn = static_cast<iWareConnection*>(connection);
          if (operation == NULL)
          {
            Serial.println("Connection is [" + String(connection->Connection) + "]. GPIO: [" + String(conn->GPIO) + "].");
          }
          else
          {
            iWareSensor* sensor = static_cast<iWareSensor*>(operation);
            Serial.println("Sensor with name [" + String(sensor->DisplayName) + "] has executed. HwId: [" + String(sensor->HwId) + "].");
          }          
        }
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
  jsonParsing(json);
  Serial.println(); 
  Serial.println("---ModbusConfig processing------------"); 
  if (modbusCfg.parseConfig(json))
  {
    modbusCfg.printConfig();
  }
}

void jsonParsing(String json)
{
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    processJsonError(error.code());
    return;
  }

  JsonObject obj = doc.as<JsonObject>();
  JsonArray arr = doc.as<JsonArray>();

  /*Serial.println("--------------------------------------"); 
  JsonArray::iterator it;
  for (it=arr.begin(); it!=arr.end(); ++it) {
    const JsonObject& elem = *it;
    JsonObject slave = elem["Slave"];
    printValue("Connection", slave["Connection"].as<String>());
  }*/

  Serial.println("---JSON parsing--------------------"); 
  for (const JsonObject& item : arr) 
  {
    const JsonObject& connections = item["Connection"];
    for (const JsonPair& connection : connections) {
      String key = connection.key().c_str();
      if (connection.value().is<JsonArray>()) {
        JsonArray operations = connection.value().as<JsonArray>();
        Serial.println("---Operations:----------------------------"); 
        for (const JsonObject& operation : operations) {
          for (const JsonPair& op : operation) {
            key = op.key().c_str();
            Serial.println("\t" + key + ": " + op.value().as<String>());
          }
          Serial.println("--------------------------------------"); 
        }
      }
      else {
        Serial.println(key + ": " + connection.value().as<String>());
      }
    }
  }
}

void setup()
{
  Serial.begin(9600);
  fileSystem.initFS();
  
  modbusCfg.doc = &doc; //new DynamicJsonDocument(capacity);
  modbusCfg.pollingIntervalCallback = *pollingIntervalProcessor;

  #ifdef ESP8266
  fileSystem.showDir();
  #endif
  
  readModbusConfig();
}

void loop()
{
  modbusCfg.loopModbusConfig();
}
