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
  //DynamicJsonDocument doc;
  DeserializationError error = deserializeJson(*doc, json);
  if (error) {
    if (!processJsonError(error.code()))
    {
      Serial.println("JSON size is [" + String(json.length()) + "].");
    }
    return false;
  }

  //JsonObject obj = doc->as<JsonObject>();
  JsonArray arr = doc->as<JsonArray>();

  //Serial.println("--------------------------------------"); 
  for (int i=0; i<arr.size(); i++) {
    JsonObject slave = arr[i]["Slave"];
    Slave slaveItem; 
    slaveItem.Connection 	= slave["Connection"].as<String>();
    String type 		= slave["Type"].as<String>();
    slaveItem.Type		= (type == "TCP") ? ModbusType::TCP : ModbusType::RTU;
    slaveItem.PollingInterval 	= slave["PollingInterval"].as<int>();
    slaveItem.HwId 		= slave["HwId"].as<String>();
    slaveItem.BaudRate 		= slave["BaudRate"].as<int>();
    slaveItem.Config 		= slave["Config"].as<String>();
    slaveItem.Config 		= (slaveItem.Config == "") ? "SERIAL_8N1" : slaveItem.Config;
    slaveItem.RxPin 		= slave["RxPin"].as<int>(); // default value doesnt't work in v.6 beta | -1;
    slaveItem.TxPin 		= slave["TxPin"].as<int>(); // default value doesnt't work in v.6 beta | -1;
    slaveItem.RetryCount 	= slave["RetryCount"].as<int>();
    slaveItem.RetryInterval 	= slave["RetryInterval"].as<int>();
    slaveItem.TcpPort 		= slave["TcpPort"].as<int>();
    slaveItem.lastPolling	= 0;

    /*if (minSlavePollingInterval > slaveItem.PollingInterval)
    { 
      minSlavePollingInterval = slaveItem.PollingInterval; 
    }*/	

    JsonArray ops = arr[i]["Slave"]["Ops"];
    Operation operation;
    for (int i=0; i<ops.size(); i++) {
      JsonObject op = ops[i];
      operation.PollingInterval = op["PollingInterval"].as<int>();
      operation.UnitId 		= op["UnitId"].as<int>();
      //const char* function 	= op["Function"].as<char*>();
      operation.Function 	= op["Function"].as<int>(); //StrToHex(function);
      //const char* address 	= op["Address"].as<char*>();
      operation.Address 	= op["Address"].as<int>(); //StrToHex(address);
      operation.Len 		= op["Len"].as<int>();
      operation.DisplayName 	= op["DisplayName"].as<String>();
      operation.lastPolling	= 0;
      slaveItem.Operations.push_back(operation);
    }
    this->slaves.push_back(slaveItem);
  }
  return true;
}

void ModbusConfig::loopModbusConfig()
{
  unsigned long currentMillis = millis();
  //if (currentMillis - lastPolling > minSlavePollingInterval)
  //{
  for (Slave& slave : this->slaves)
  {
    if (currentMillis - slave.lastPolling >= slave.PollingInterval)
    {
      if (this->pollingIntervalCallback)
      {
        pollingIntervalCallback(&slave, NULL); //Execute callback function to process slave polling interval
      }
      slave.lastPolling = currentMillis;
    }

    for (Operation& operation : slave.Operations)	
    {
      if (currentMillis - operation.lastPolling >= operation.PollingInterval)
      {
       if (this->pollingIntervalCallback) 
       {
          pollingIntervalCallback(&slave, &operation); //Execute callback function to process operation polling interval
       }
       operation.lastPolling = currentMillis;
      }
    }	
  }
  //lastPolling = currentMillis;
  //}
}

void ModbusConfig::printConfig()
{
  for (const Slave& slave : slaves)
  {
    printValue("Connection"	, slave.Connection);
    String type = (slave.Type == ModbusType::TCP) ? "TCP" : "RTU";
    printValue("Type"		, type);
    printValue("HwId"		, slave.HwId);
    printValue("PollingInterval", String(slave.PollingInterval));
    printValue("RxPin"		, String(slave.RxPin));
    printValue("TxPin"		, String(slave.TxPin));
    printValue("BaudRate"	, String(slave.BaudRate));
    printValue("Config"		, slave.Config);
    printValue("RetryCount"	, String(slave.RetryCount));
    printValue("RetryInterval"	, String(slave.RetryInterval));
    printValue("TcpPort"	, String(slave.TcpPort));
    Serial.println("---Operations:-------------------------"); 
    for (const Operation& operation : slave.Operations)	
    {
      printValue("\tPollingInterval", String(operation.PollingInterval));
      printValue("\tUnitId"	, String(operation.UnitId));
      printValue("\tFunction"	, String(operation.Function, HEX), true);
      printValue("\tAddress"	, String(operation.Address, HEX), true);
      printValue("\tLen"	, String(operation.Len));
      printValue("\tDisplayName", operation.DisplayName);
      Serial.println("--------------------------------------"); 
    }	
  }
}

//isHex default value is false
void ModbusConfig::printValue(String name, String value, bool isHex)
{
  if ((value != "null") && (value != "") && (value != "0")) //0 - default value for int 
  {
    String prefix = "";
    if (isHex)
    {
      prefix = (value.length() == 1) ? "0x0" : "0x";
    }
    Serial.println(name + ": " + prefix + value);
  }
}