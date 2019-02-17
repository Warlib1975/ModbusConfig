#include <ArduinoJson.h>
/*
 * ModbusConfig test example for ESP8266 boards. For Arduino just remove #include <ESP8266WiFi.h> 
*/

#include <ESP8266WiFi.h>
#include "ModbusConfig.h";
#include "EspFS.h";

#define Num_of_Slaves 2 //Number of serial ports on the board
#define Num_of_Ops    5 //Number of operations per serial port 

//30 - is the maximum number of possible operations https://arduinojson.org/v6/assistant/ 
const size_t capacity = 2*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + Num_of_Ops*JSON_OBJECT_SIZE(6) + Num_of_Slaves*JSON_OBJECT_SIZE(11) + 830;
StaticJsonDocument<capacity> doc;

char* filename = "/modbus.cfg"; //You should upload the file to SPDIFF using the tool: https://github.com/esp8266/arduino-esp8266fs-plugin

ModbusConfig modbusCfg;
EspFS fileSystem;

//Callback function to process polling interval
void pollingIntervalProcessor(Connection* connection, Operation* operation)
{
  if (connection)
  {
     if (operation == NULL)
     {
       Serial.println("Publish to the cloud. Slave connection is [" + String(connection->Connection) + "]. HwId: [" + String(connection->HwId) + "].");
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
  Serial.begin(9600, SERIAL_8N1);
  
  modbusCfg.doc = new DynamicJsonDocument(capacity);
  modbusCfg.pollingIntervalCallback = *pollingIntervalProcessor;

  fileSystem.showDir();
  readModbusConfig();
}

void loop()
{
  modbusCfg.loopModbusConfig();
}
