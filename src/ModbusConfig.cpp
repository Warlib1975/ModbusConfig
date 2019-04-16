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
  for (int i = 0; i < modbus.size(); i++) {
    //JsonObject connectionJSON = arr[i]["Connection"];
    ModbusConnection* connection  = new ModbusConnection; 
    JsonObject connectionJSON     = modbus[i];//doc["Modbus"][i];
    connection->Connection 	      = connectionJSON["Connection"].as<String>();
    connection->Sensor            = SensorType :: Modbus;
    String type 		              = connectionJSON["Type"].as<String>();
    connection->HardwareSerial    = connectionJSON["HardwareSerial"] | -1;  //.as<int>();  //
    connection->Type		          = (type == "TCP") ? ModbusType::TCP : ModbusType::RTU;
    connection->PollingInterval 	= connectionJSON["PollingInterval"] | 60000;  //.as<int>(); //
    connection->HwId 		          = connectionJSON["HwId"].as<String>();
    connection->BaudRate 		      = connectionJSON["BaudRate"] | 9600; //.as<int>(); //
    connection->Config 		        = connectionJSON["Config"].as<String>(); //
    connection->Config 		        = (connection->Config == "") ? "SERIAL_8N1" : connection->Config;
    connection->RxPin 		        = connectionJSON["RxPin"] | -1; //.as<int>(); //  | -1;
    connection->TxPin 		        = connectionJSON["TxPin"] | -1; //.as<int>(); //  | -1;
    connection->RetryCount 	      = connectionJSON["RetryCount"] | 10; //.as<int>(); //
    connection->RetryInterval 	  = connectionJSON["RetryInterval"] | 1000; //.as<int>(); //
    connection->TcpPort 		      = connectionJSON["TcpPort"] | -1; //.as<int>(); //
    connection->lastPolling	      = 0;

    //JsonArray ops = arr[i]["Connection"]["Ops"];
    JsonArray ops = connectionJSON["Ops"];
    for (int j = 0; j < ops.size(); j++) {
      JsonObject operationJSON = ops[j];
      ModbusOperation* operation = new ModbusOperation;
      operation->HwId            = "Empty";
      operation->PollingInterval = operationJSON["PollingInterval"] | 60000; //.as<int>(); //
      operation->SlaveId 	      = operationJSON["SlaveId"].as<int>(); //| 1; //
      operation->Function 	    = operationJSON["Function"].as<int>(); // StrToHex(function);
      operation->Address 	      = operationJSON["Address"].as<int>(); // StrToHex(address);
      operation->Len 		        = operationJSON["Len"].as<int>(); //
      operation->DisplayName 	  = operationJSON["DisplayName"].as<String>(); //
      operation->Transform 	    = operationJSON["Transform"].as<String>(); //
      operation->Location 	    = operationJSON["Location"].as<String>(); //
      operation->lastPolling	  = 0;
      connection->Operations.push_back(operation);
    }
    this->connections.push_back(connection);
  }

  JsonArray iWare = obj["iWare"];
  for (int i = 0; i < iWare.size(); i++) {
    JsonObject connectionJSON = iWare[i];
    iWareConnection* connection = new iWareConnection;
    connection->Sensor          = SensorType :: iWare; 
    connection->Connection 	    = connectionJSON["Connection"].as<String>();
    connection->GPIO            = connectionJSON["GPIO"].as<int>() | -1;  //.as<int>();  //
    connection->PollingInterval 	= connectionJSON["PollingInterval"] | 600000;  //.as<int>(); //
    connection->lastPolling	    = 0;

    JsonArray Sensors = connectionJSON["Sensors"];
    for (int j = 0; j < Sensors.size(); j++) {
      iWareSensor* sensor = new iWareSensor;
      JsonObject sensorJSON   = Sensors[j];
      sensor->HwId              = sensorJSON["HwId"].as<String>();
      sensor->PollingInterval   = sensorJSON["PollingInterval"] | 60000; //.as<int>(); //
      sensor->DisplayName 	    = sensorJSON["DisplayName"].as<String>(); //
      sensor->Location 	        = sensorJSON["Location"].as<String>(); //
      sensor->Transform 	      = sensorJSON["Transform"].as<String>(); //
      //connection->Sensors.push_back(sensor);
      connection->Operations.push_back(sensor);
    }
    this->connections.push_back(connection);
  }

  JsonArray Analog = obj["Analog"];
  for (int i = 0; i < Analog.size(); i++) {
    JsonObject connectionJSON 	= Analog[i];
    AnalogConnection* connection = new AnalogConnection; 
    connection->Connection 	    = connectionJSON["Connection"].as<String>();
    connection->PollingInterval = connectionJSON["PollingInterval"] | 600000;  //.as<int>(); //
    connection->Sensor          = SensorType :: Analog;
    connection->lastPolling	    = 0;

    JsonArray Sensors = connectionJSON["Sensors"];
    for (int j = 0; j < Sensors.size(); j++) {
      JsonObject sensorJSON 	= Sensors[j];
      AnalogSensor* sensor 		= new AnalogSensor;
      sensor->HwId              = sensorJSON["HwId"].as<String>(); //.as<int>(); //
      sensor->PollingInterval   = sensorJSON["PollingInterval"] | 5000; //.as<int>(); //
      sensor->DisplayName 	    = sensorJSON["DisplayName"].as<String>(); //
      sensor->Location 	        = sensorJSON["Location"].as<String>(); //
      sensor->Transform 	    = sensorJSON["Transform"].as<String>(); //
      sensor->Channel 	        = sensorJSON["Channel"] | -1; //
      sensor->GPIO 	            = sensorJSON["GPIO"] | -1; //
      //connection->Sensors.push_back(sensor);
      connection->Operations.push_back(sensor);
    }
    this->connections.push_back(connection);
  }
  
  JsonArray Relay = obj["RelayOutput"];
  for (int i = 0; i < Relay.size(); i++) 
  {
    JsonObject connectionJSON 	= Relay[i];
    RelayConnection* connection = new RelayConnection; 
    connection->Connection 	    = connectionJSON["Connection"].as<String>();
    connection->PollingInterval = connectionJSON["PollingInterval"] | 600000;  //.as<int>(); //
    connection->Sensor          = SensorType :: Relay;
    connection->lastPolling	    = 0;

    JsonArray Relays = connectionJSON["Relays"];
    for (int j = 0; j < Relays.size(); j++) 
	{
      JsonObject sensorJSON 		= Relays[j];
	  RelayOutput* sensor 		= new RelayOutput;
      sensor->HwId              = sensorJSON["HwId"].as<String>(); //.as<int>(); //
      sensor->PollingInterval   = sensorJSON["PollingInterval"] | 600000; //.as<int>(); //
      sensor->DisplayName 	    = sensorJSON["DisplayName"].as<String>(); //
      sensor->Location 	        = sensorJSON["Location"].as<String>(); //
      sensor->GPIO 	            = sensorJSON["GPIO"] | -1; //
      //connection->Sensors.push_back(sensor);
      connection->Operations.push_back(sensor);
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
  for (BaseConnection* connectionJSON : this->connections)
  {
    if ((connectionJSON->PollingInterval > 0) &&
        (currentMillis - connectionJSON->lastPolling >= connectionJSON->PollingInterval))
    {
      if (this->pollingIntervalCallback)
      {
        pollingIntervalCallback(connectionJSON, NULL); //Execute callback function to process connectionJSON polling interval
      }
      connectionJSON->lastPolling = currentMillis;
    }

    for (BaseOperation* operation : connectionJSON->Operations)	
    {
      if ((operation->PollingInterval) &&
          (currentMillis - operation->lastPolling >= operation->PollingInterval))
      {
       if (this->pollingIntervalCallback) 
       {
          pollingIntervalCallback(connectionJSON, operation); //Execute callback function to process operation polling interval
       }
       operation->lastPolling = currentMillis;
      }
    }
    //this->connections.shrink_to_fit(); //free unused memory	
  }
  //lastPolling = currentMillis;
  //}
}

void ModbusConfig::printOperations(SensorType sensorType, OperationsType operations)
{
  Serial.println("---Operations:-------------------------"); 
  for (BaseOperation* op : operations)	
  {
    printValue("\tHwId", op->HwId);
    printValue("\tPollingInterval", String(op->PollingInterval));
    printValue("\tDisplayName", op->DisplayName);
    printValue("\tTransform", op->Transform);
    printValue("\tLocation", op->Location);
    switch (sensorType)
    {
      case SensorType::Modbus:
      {
        ModbusOperation* operation = static_cast<ModbusOperation*>(op);
        printValue("\tSlaveId"	, String(operation->SlaveId));
        printValue("\tFunction"	, String(operation->Function, HEX), true);
        printValue("\tAddress"	, String(operation->Address, HEX), true);
        printValue("\tLen"	, String(operation->Len));
        break; 
      }
      case SensorType::iWare:
        break;
      case SensorType::Analog:
      {
        AnalogSensor* sensor = static_cast<AnalogSensor*>(op);
        printValue("\tChannel"	, String(sensor->Channel));
        printValue("\tGPIO"	, String(sensor->GPIO));
        break;
      }
	  case SensorType::Relay:
      {
        RelayOutput* sensor = static_cast<RelayOutput*>(op);
        printValue("\tGPIO"	, String(sensor->GPIO));
        break;
      }
      default:
        printValue("\nUnknown sensor type", "");
    }
    Serial.println("--------------------------------------"); 
  }
}

void ModbusConfig::printConfig()
{
  for (BaseConnection* conn : connections)
  {
    printValue("Connection"	, conn->Connection);
    printValue("Sensor"	, String(conn->Sensor));
    printValue("PollingInterval", String(conn->PollingInterval));
    switch (conn->Sensor)
    {
      case SensorType::Modbus:
      {
        ModbusConnection* connection = static_cast<ModbusConnection*>(conn); 
        printValue("PollingInterval", String(connection->PollingInterval));
        String type = (connection->Type == ModbusType::TCP) ? "TCP" : "RTU";
        printValue("Type"		, type);
        printValue("HwId"		, connection->HwId);
        printValue("HardwareSerial"	, String(connection->HardwareSerial));
        printValue("RxPin"		, String(connection->RxPin));
        printValue("TxPin"		, String(connection->TxPin));
        printValue("BaudRate"	, String(connection->BaudRate));
        printValue("Config"		, connection->Config);
        printValue("RetryCount"	, String(connection->RetryCount));
        printValue("RetryInterval"	, String(connection->RetryInterval));
        printValue("TcpPort"	, String(connection->TcpPort));
        printOperations(connection->Sensor, connection->Operations);
        break;
      }
      case SensorType::iWare:
      {
        iWareConnection* connection = static_cast<iWareConnection*>(conn); 
        printValue("GPIO", String(connection->GPIO));
        printOperations(connection->Sensor, connection->Operations);
        break;
      }
      case SensorType::Analog:
      {
        AnalogConnection* connection = static_cast<AnalogConnection*>(conn); 
        printOperations(connection->Sensor, connection->Operations);
        break;
      }
      case SensorType::Relay:
      {
        RelayConnection* connection = static_cast<RelayConnection*>(conn); 
        printOperations(connection->Sensor, connection->Operations);
        break;
      }
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