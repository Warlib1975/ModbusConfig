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

  JsonObject obj = doc->as<JsonObject>();
  JsonArray modbus = obj["Modbus"];
  //JsonArray arr = doc->as<JsonArray>();

  //Serial.println("--------------------------------------"); 
  for (int i=0; i<modbus.size(); i++) {
    //JsonObject connectionJSON = arr[i]["Connection"];
    JsonObject connectionJSON = modbus[i];//doc["Modbus"][i];
    Connection connection; 
    connection.Connection 	  = connectionJSON["Connection"].as<String>();
    String type 		          = connectionJSON["Type"].as<String>();
    connection.HardwareSerial = connectionJSON["HardwareSerial"] | -1;  //.as<int>();  //
    connection.Type		        = (type == "TCP") ? ModbusType::TCP : ModbusType::RTU;
    connection.PollingInterval 	= connectionJSON["PollingInterval"] | 5000;  //.as<int>(); //
    connection.HwId 		      = connectionJSON["HwId"].as<String>();
    connection.BaudRate 		  = connectionJSON["BaudRate"] | 9600; //.as<int>(); //
    connection.Config 		    = connectionJSON["Config"].as<String>(); //
    connection.Config 		    = (connection.Config == "") ? "SERIAL_8N1" : connection.Config;
    connection.RxPin 		      = connectionJSON["RxPin"] | -1; //.as<int>(); //  | -1;
    connection.TxPin 		      = connectionJSON["TxPin"] | -1; //.as<int>(); //  | -1;
    connection.RetryCount 	  = connectionJSON["RetryCount"] | 10; //.as<int>(); //
    connection.RetryInterval 	= connectionJSON["RetryInterval"] | 1000; //.as<int>(); //
    connection.TcpPort 		    = connectionJSON["TcpPort"] | -1; //.as<int>(); //
    connection.lastPolling	= 0;

    /*if (minconnectionPollingInterval > connection.PollingInterval)
    { 
      minconnectionPollingInterval = connection.PollingInterval; 
    }*/	

    //JsonArray ops = arr[i]["Connection"]["Ops"];
    JsonArray ops = connectionJSON["Ops"];
    Operation operation;
    for (int i=0; i<ops.size(); i++) {
      JsonObject operationJSON = ops[i];
      operation.PollingInterval = operationJSON["PollingInterval"] | 5000; //.as<int>(); //
      operation.SlaveId 	      = operationJSON["SlaveId"] | 1; //.as<int>(); //
      operation.Function 	      = operationJSON["Function"]; //.as<int>(); // StrToHex(function);
      operation.Address 	      = operationJSON["Address"]; //.as<int>(); // StrToHex(address);
      operation.Len 		        = operationJSON["Len"]; //.as<int>(); //
      operation.DisplayName 	  = operationJSON["DisplayName"].as<String>(); //
      operation.lastPolling	    = 0;
      connection.Operations.push_back(operation);
    }
    this->connections.push_back(connection);
  }
  return true;
}

void ModbusConfig::loopModbusConfig()
{
  unsigned long currentMillis = millis();
  //if (currentMillis - lastPolling > minconnectionPollingInterval)
  //{
  for (Connection& connectionJSON : this->connections)
  {
    if (currentMillis - connectionJSON.lastPolling >= connectionJSON.PollingInterval)
    {
      if (this->pollingIntervalCallback)
      {
        pollingIntervalCallback(&connectionJSON, NULL); //Execute callback function to process connectionJSON polling interval
      }
      connectionJSON.lastPolling = currentMillis;
    }

    for (Operation& operation : connectionJSON.Operations)	
    {
      if (currentMillis - operation.lastPolling >= operation.PollingInterval)
      {
       if (this->pollingIntervalCallback) 
       {
          pollingIntervalCallback(&connectionJSON, &operation); //Execute callback function to process operation polling interval
       }
       operation.lastPolling = currentMillis;
      }
    }
    this->connections.shrink_to_fit(); //free unused memory	
  }
  //lastPolling = currentMillis;
  //}
}

void ModbusConfig::printConfig()
{
  for (const Connection& connection : connections)
  {
    printValue("Connection"	, connection.Connection);
    String type = (connection.Type == ModbusType::TCP) ? "TCP" : "RTU";
    printValue("Type"		, type);
    printValue("HwId"		, connection.HwId);
    printValue("HardwareSerial"	, String(connection.HardwareSerial));
    printValue("PollingInterval", String(connection.PollingInterval));
    printValue("RxPin"		, String(connection.RxPin));
    printValue("TxPin"		, String(connection.TxPin));
    printValue("BaudRate"	, String(connection.BaudRate));
    printValue("Config"		, connection.Config);
    printValue("RetryCount"	, String(connection.RetryCount));
    printValue("RetryInterval"	, String(connection.RetryInterval));
    printValue("TcpPort"	, String(connection.TcpPort));
    
    Serial.println("---Operations:-------------------------"); 
    for (const Operation& operation : connection.Operations)	
    {
      printValue("\tPollingInterval", String(operation.PollingInterval));
      printValue("\tSlaveId"	, String(operation.SlaveId));
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
  if ((value != "null") && (value != "") && (value != "-1")) //-1 - default value for int 
  {
    String prefix = "";
    if (isHex)
    {
      prefix = (value.length() == 1) ? "0x0" : "0x";
    }
    Serial.println(name + ": " + prefix + value);
  }
}