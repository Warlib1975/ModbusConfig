// Please read ModbusConfig.h for information about the liscence and authors

#include "ModbusConfig.h"

bool processJsonError(int error)
{
    bool res = false;	
    switch (error) {
      case DeserializationError::Ok:
        Serial.println("Deserialization succeeded");
	res = true;
        break;
      case DeserializationError::InvalidInput:
        Serial.println("Invalid input!");
        break;
      case DeserializationError::NoMemory:
        Serial.println("Not enough memory");
        break;
      default:
        Serial.println("Deserialization failed");
        break;
    }
    return res;
}

ModbusConfig::ModbusConfig()
{
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
  return this->parseConfig(this->json);
}

bool ModbusConfig::parseConfig(String json)
{
  Slaves slaves;
    
  //DynamicJsonDocument doc;
  DeserializationError error = deserializeJson(*doc, json);
  if (error) {
    if (!processJsonError(error.code()))
    {
      Serial.println("JSON size is [" + String(json.length()) + "].");
    }
    return false;
  }

  JsonObject obj = doc->as<JsonObject>();
  JsonArray arr = doc->as<JsonArray>();

  //Serial.println("--------------------------------------"); 
  for (int i=0; i<arr.size(); i++) {
    JsonObject slave = arr[i]["Slave"];
    Slave slaveItem; 
    slaveItem.Connection 	= slave["Connection"].as<String>();
    slaveItem.PollingInterval 	= slave["PollingInterval"].as<int>();
    slaveItem.HwId 		= slave["HwId"].as<String>();
    slaveItem.BaudRate 		= slave["BaudRate"].as<int>();
    slaveItem.Config 		= slave["Config"].as<String>();
    slaveItem.Config 		= (slaveItem.Config == "") ? "SERIAL_8N1" : slaveItem.Config;
    slaveItem.RxPin 		= slave["RxPin"].as<int>();
    slaveItem.TxPin 		= slave["TxPin"].as<int>();
    slaveItem.RetryCount 	= slave["RetryCount"].as<int>();
    slaveItem.RetryInterval 	= slave["RetryInterval"].as<int>();
    slaveItem.TcpPort 		= slave["TcpPort"].as<int>();

    printValue("Connection"	, slaveItem.Connection);
    printValue("HwId"		, slaveItem.HwId);
    printValue("PollingInterval", String(slaveItem.PollingInterval));
    printValue("RxPin"		, String(slaveItem.RxPin));
    printValue("TxPin"		, String(slaveItem.TxPin));
    printValue("BaudRate"	, String(slaveItem.BaudRate));
    printValue("Config"		, slaveItem.Config);
    printValue("RetryCount"	, String(slaveItem.RetryCount));
    printValue("RetryInterval"	, String(slaveItem.RetryInterval));
    printValue("TcpPort"	, String(slaveItem.TcpPort));    

    JsonArray ops = arr[i]["Slave"]["Ops"];
    Operation operation;
    Serial.println("---Operations:-------------------------"); 
    for (int i=0; i<ops.size(); i++) {
      JsonObject op = ops[i];
      operation.PollingInterval = op["PollingInterval"].as<int>();
      operation.UnitId 		= op["UnitId"].as<int>();
      const char* function 	= op["Function"].as<char*>();
      operation.Function 	= StrToHex(function);
      const char* address 	= op["Address"].as<char*>();
      operation.Address 	= StrToHex(address);
      operation.Len 		= op["Len"].as<int>();
      operation.DisplayName 	= op["DisplayName"].as<String>();
      slaveItem.Operations.push_back(operation);

      printValue("\tPollingInterval", String(operation.PollingInterval));
      printValue("\tUnitId"	, String(operation.UnitId));
      printValue("\tFunction"	, String(operation.Function));
      printValue("\tAddress"	, String(operation.Address));
      printValue("\tLen"	, String(operation.Len));
      printValue("\tDisplayName", operation.DisplayName);
      Serial.println("--------------------------------------"); 
    }
    slaves.push_back(slaveItem);
  }
  return true;
}

void ModbusConfig::printValue(String name, String value)
{
  if ((value != "null") && (value != "") && (value != "0")) //0 - default value for int 
  {
    Serial.println(name + ": " + value);
  }
}