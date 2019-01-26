// Please read ModbusConfig.h for information about the liscence and authors

#include "ModbusConfig.h"
#include <FS.h>   //http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html

void processError(int error)
{
    switch (error) {
      case DeserializationError::Ok:
        Serial.print(F("Deserialization succeeded"));
        break;
      case DeserializationError::InvalidInput:
        Serial.print(F("Invalid input!"));
        break;
      case DeserializationError::NoMemory:
        Serial.print(F("Not enough memory"));
        break;
      default:
        Serial.print(F("Deserialization failed"));
        break;
    }
    return;
}

ModbusConfig::ModbusConfig()
{
  this->isSPIFFSInitialized = false;	
}

int ModbusConfig::StrToHex(const char* str)
{
  if (strchr(str, 'x') != NULL )
  { 
    return (int) strtol(str, 0, 16);
  }
  return (int) strtol(str, 0, 10);
}

bool ModbusConfig::parseConfig()
{
  return this->parseConfig(json);
}

bool ModbusConfig::parseConfig(String json)
{
  Slaves slaves;
    
  //DynamicJsonDocument doc;
  DeserializationError error = deserializeJson(*doc, json);
  if (error) {
    processError(error.code());
    return false;
  }

  JsonObject obj = doc->as<JsonObject>();
  JsonArray arr = doc->as<JsonArray>();

  //Serial.println("--------------------------------------"); 
  for (int i=0; i<arr.size(); i++) {
    JsonObject slave = arr[i]["Slave"];
    Slave slaveItem; 
    slaveItem.Connection = slave["Connection"].as<String>();
    slaveItem.HwId = slave["HwId"].as<String>();
    slaveItem.BaudRate = slave["BaudRate"].as<int>();
    slaveItem.DataBits = slave["DataBits"].as<int>();
    slaveItem.StopBits = slave["StopBits"].as<int>();
    slaveItem.Parity = slave["Parity"].as<String>();
    slaveItem.FlowControl = slave["FlowControl"].as<String>();
    slaveItem.TcpPort = slave["TcpPort"].as<int>();

    printValue("Connection", slaveItem.Connection);
    printValue("HwId", slaveItem.HwId);
    printValue("BaudRate", String(slaveItem.BaudRate));
    printValue("DataBits", String(slaveItem.DataBits));
    printValue("StopBits", String(slaveItem.StopBits));
    printValue("Parity", slaveItem.Parity);
    printValue("FlowControl", slaveItem.FlowControl);
    printValue("TcpPort", String(slaveItem.TcpPort));    

    JsonArray ops = arr[i]["Slave"]["Ops"];
    Operation operation;
    Serial.println("---Operations:-------------------------"); 
    for (int i=0; i<ops.size(); i++) {
      JsonObject op = ops[i];
      operation.PollingInterval = op["PollingInterval"].as<int>();
      operation.UnitId = op["UnitId"].as<int>();
      const char* function = op["Function"].as<char*>();
      operation.Function = StrToHex(function);//op["Function"].as<int>();
      const char* address = op["Address"].as<char*>();
      operation.Address = StrToHex(address);//op["Address"].as<int>();
      operation.Len = op["Len"].as<int>();
      operation.DisplayName = op["DisplayName"].as<String>();
      slaveItem.Operations.push_back(operation);

      printValue("\tPollingInterval", String(operation.PollingInterval));
      printValue("\tUnitId", String(operation.UnitId));
      printValue("\tFunction", String(operation.Function));
      printValue("\tAddress", String(operation.Address));
      printValue("\tLen", String(operation.Len));
      printValue("\tDisplayName", operation.DisplayName);
      Serial.println("--------------------------------------"); 
    }
    slaves.push_back(slaveItem);
  }
  return true;
}

bool ModbusConfig::loadConfig()
{
  return this->loadConfig(filename);
}

bool ModbusConfig::loadConfig(char* filename)
{
  int i;
  this->json = "";

  if (!isSPIFFSInitialized)
  {
    isSPIFFSInitialized = this->initFS();
  }

  if (!isSPIFFSInitialized)
  {
    return false;
  }

  Serial.println("Try to read file [" + String(filename) + "].");

  //Read File data
  File file = SPIFFS.open(filename, "r");
  
  if (!file) 
  {
    Serial.println("File [" + String(file.name()) + "] open failed.");
    return false;	
  }
  else
  {
    Serial.println("Reading Data from File: [" + String(file.name()) + "] with size [" + file.size() + "].");

    //Data from file
    for(i=0;i<file.size();i++) //Read upto complete file size
    {
      this->json += (char)file.read();
    }
    Serial.println(this->json);
    file.close();  //Close file
    Serial.println();
    Serial.println("File Closed.");
  }
  return true;
}

bool ModbusConfig::initFS() 
{
  //Initialize File System
  if(SPIFFS.begin())
  {
    Serial.println("SPIFFS Initialize....ok");
    isSPIFFSInitialized = true;
    return true;
  }
  else
  {
    Serial.println("SPIFFS Initialization...failed");
    return false;
  }
}

bool ModbusConfig::formatFS()
{
  //Format File System
  if(SPIFFS.format())
  {
    Serial.println("File System Formated");
    return true;
  }
  else
  {
    Serial.println("File System Formatting Error");
    return false;	
  }
}

void ModbusConfig::showDir()
{
  String str = "";
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    str += dir.fileName();
    str += " / ";
    str += dir.fileSize();
    str += "\r\n";
  }
  Serial.print(str);
}

void ModbusConfig::printValue(String name, String value)
{
  if ((value != "null") && (value != "") && (value != "0")) //exclude 0 - it's not so good idea. :-) 
  {
    Serial.println(name + ": " + value);
  }
}