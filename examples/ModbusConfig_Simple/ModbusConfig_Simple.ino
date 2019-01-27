#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "ModbusConfig.h";
#include "EspFS.h";

#define Num_of_Slaves 2 //Number of serial ports on the board
#define Num_of_Ops     5 //Number of operations per serial port 

//30 - is the maximum number of possible operations  
const size_t capacity = 2*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + Num_of_Ops*JSON_OBJECT_SIZE(6) + Num_of_Slaves*JSON_OBJECT_SIZE(8) + 830;
StaticJsonDocument<capacity> doc;

//DynamicJsonBuffer jsonBuffer(capacity);

char* filename = "/modbus.cfg";

ModbusConfig modbusCfg;
EspFS fileSystem;

void readRS485Config()
{
  //DynamicJsonDocument *doc = 


  bool res = fileSystem.loadTextFile(filename);
  if (res)
  {
    processRS485Config(fileSystem.text);
  }
  else
  {
    Serial.println("File reading error.");
  }
}

void processRS485Config(String json)
{
  Serial.println("---Auto processing--------------------"); 
  autoProcess(json);
  Serial.println(); 
  Serial.println("---Manual processing-----------------"); 
  modbusCfg.parseConfig(json);
}

void autoProcess(String json)
{
  //DynamicJsonDocument doc;
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

  Serial.println("--------------------------------------"); 
  for (const JsonObject& item : arr) {
    const JsonObject& slaves = item["Slave"];
    //printValue("Connection", slave["Connection"].as<String>());
    for (const JsonPair& slave : slaves) {
      String key = slave.key().c_str();
      if (slave.value().is<JsonArray>())
      {
        JsonArray operations = slave.value().as<JsonArray>();
        Serial.println("---Operations:-------------------------"); 
        for (const JsonObject& operation : operations) {
          for (const JsonPair& op : operation) {
            key = op.key().c_str();
            Serial.println("\t" + key + ": " + op.value().as<String>());
          }
          Serial.println("--------------------------------------"); 
        }
      }
      else
      {
        Serial.println(key + ": " + slave.value().as<String>());
      }
    }
  }
}

void printResult()
{
  JsonObject res;
}

void setup()
{
  Serial.begin(9600, SERIAL_8N1);
  
  modbusCfg.doc = new DynamicJsonDocument(capacity);
  fileSystem.showDir();
  
  //showDir();
  //showDir1();
  
  readRS485Config();
}

void loop()
{
}
