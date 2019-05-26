// Please read ModbusConfig.h for information about the liscence and authors

#include "ModbusConfig.h"
//#include "Evaluator.h"

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
    connection->Transform 	    = connectionJSON["Transform"] | ""; //.as<String>()
    connection->lastPolling	      = 0;
    connection->lastInstantMillis = 0;

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
      operation->Transform 	    = operationJSON["Transform"] | ""; //.as<String>()
      operation->Location 	    = operationJSON["Location"].as<String>(); //
      operation->lastPolling	  = 0;
      operation->lastInstantMillis	  = 0;
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
    connection->Transform 	    = connectionJSON["Transform"] | "0.1"; //.as<String>()
    connection->GPIO            = connectionJSON["GPIO"].as<int>() | -1;  //.as<int>();  //
    connection->PollingInterval 	= connectionJSON["PollingInterval"] | 600000;  //.as<int>(); //
    connection->lastPolling	    = 0;
    connection->lastInstantMillis	    = 0;

    JsonArray Sensors = connectionJSON["Sensors"];
    for (int j = 0; j < Sensors.size(); j++) {
      iWareSensor* sensor = new iWareSensor;
      JsonObject sensorJSON   = Sensors[j];
      sensor->HwId              = sensorJSON["HwId"].as<String>();
      sensor->PollingInterval   = sensorJSON["PollingInterval"] | 60000; //.as<int>(); //
      sensor->DisplayName 	    = sensorJSON["DisplayName"].as<String>(); //
      sensor->Location 	        = sensorJSON["Location"].as<String>(); //
      sensor->Transform 	      = sensorJSON["Transform"] | ""; //.as<String>()
      sensor->lastPolling	  = 0;
      sensor->lastInstantMillis	  = 0;
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
    connection->Transform 	    = connectionJSON["Transform"] | ""; //.as<String>()
    connection->lastPolling	    = 0;
    connection->lastInstantMillis	    = 0;

    JsonArray Sensors = connectionJSON["Sensors"];
    for (int j = 0; j < Sensors.size(); j++) {
      JsonObject sensorJSON 	= Sensors[j];
      AnalogSensor* sensor 		= new AnalogSensor;
      sensor->HwId              = sensorJSON["HwId"].as<String>(); //.as<int>(); //
      sensor->PollingInterval   = sensorJSON["PollingInterval"] | 5000; //.as<int>(); //
      sensor->DisplayName 	    = sensorJSON["DisplayName"].as<String>(); //
      sensor->Location 	        = sensorJSON["Location"].as<String>(); //
      sensor->Transform 	    = sensorJSON["Transform"] | ""; //.as<String>()
      sensor->Channel 	        = sensorJSON["Channel"] | -1; //
      sensor->GPIO 	            = sensorJSON["GPIO"] | -1; //
      sensor->lastPolling	  = 0;
      sensor->lastInstantMillis	  = 0;
      //connection->Sensors.push_back(sensor);
      connection->Operations.push_back(sensor);
    }
    this->connections.push_back(connection);
  }
  
  JsonArray relays = obj["RelayOutput"];
  for (int i = 0; i < relays.size(); i++) 
  {
    JsonObject connectionJSON 	= relays[i];
    //RelayConnection* connection = new RelayConnection; 
	relayOutput 				= new RelayConnection; 
    relayOutput->Connection 	    = connectionJSON["Connection"].as<String>();
    relayOutput->PollingInterval = connectionJSON["PollingInterval"] | 600000;  //.as<int>(); //
    relayOutput->Transform 	    = connectionJSON["Transform"] | ""; //.as<String>()
    relayOutput->Sensor          = SensorType :: Relay;
    relayOutput->lastPolling	    = 0;
    relayOutput->lastInstantMillis	    = 0;

    JsonArray Relays = connectionJSON["Relays"];
    for (int j = 0; j < Relays.size(); j++) 
	{
      JsonObject sensorJSON 		= Relays[j];
	  RelayOut* sensor 			= new RelayOut;
      sensor->HwId              = sensorJSON["HwId"].as<String>(); //.as<int>(); //
      sensor->PollingInterval   = sensorJSON["PollingInterval"] | 600000; //.as<int>(); //
      sensor->DisplayName 	    = sensorJSON["DisplayName"].as<String>(); //
      sensor->Location 	        = sensorJSON["Location"].as<String>(); //
      sensor->GPIO 	            = sensorJSON["GPIO"] | -1; //
      sensor->lastPolling	  = 0;
      sensor->lastInstantMillis	  = 0;      //connection->Sensors.push_back(sensor);
      relayOutput->Operations.push_back(sensor);
    }
    this->connections.push_back(relayOutput);
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
    if (currentMillis - connectionJSON->lastInstantMillis >= 10) //Call the function to calculate an average value
	{
	  if (this->instantCallback)
	  {
		instantCallback(connectionJSON, NULL); 
	  }
	  connectionJSON->lastInstantMillis = currentMillis;
	}
  
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
	  
	  if (currentMillis - operation->lastInstantMillis >= 10) //Call the function to calculate an average value
	  {
		  if (this->instantCallback)
		  {
			  instantCallback(connectionJSON, operation); 
		  }
		  operation->lastInstantMillis = currentMillis;
	  }
    }
    //this->connections.shrink_to_fit(); //free unused memory	
  }
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
        RelayOut* sensor = static_cast<RelayOut*>(op);
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

void ModbusConfig::math_operation(math_op op, float &prev_val, String &current_val, float &value)
{
  if (prev_val != -16101975) //Check if the default value has changed
  {
    if (current_val.length() > 0)
    {
      float second = current_val.toFloat();
      switch (op)
      {
      case math_op::Mul:
        value = prev_val * second;
        break;
      case math_op::Add:
        value = prev_val + second;
        break;
      case math_op::Div:
        value = prev_val / second;
        break;
      case math_op::Sub:
        value = prev_val - second;
        break;
      case math_op::None:
        printf("There is no operation.");
      }
	  prev_val = value;
      current_val = "";
    }
  }
  else
  {
    if (current_val.length() > 0)
    {
      prev_val = current_val.toFloat();
      current_val = "";
    }
  }
}

float ModbusConfig::Eval(String expr, float value)
{
  if (expr == "")
  {
	  return value;
  }	  
  
  //String pattern = "%V%";
  expr.toUpperCase();
  int pos = expr.indexOf("%V%"); //pattern
  if (pos == -1)
  {
    expr = "%V%*" + expr; //default operation is mul 
  }
  expr.replace("%V%", String(value));
  String val = "";
  math_op last_op = math_op::None;
  float prev_val = -16101975;//Set the some default value to check if the value has changed 
  value = 0;
  for (int i = 0; i < expr.length(); i++)
  {
    switch (expr[i])
    {
    case '*':
      math_operation(last_op, prev_val, val, value);
      last_op = math_op::Mul;
      break;
    case '+':
      math_operation(last_op, prev_val, val, value);
      last_op = math_op::Add;
      break;
    case '/':
      math_operation(last_op, prev_val, val, value);
      last_op = math_op::Div;
      break;
    case '-':
      math_operation(last_op, prev_val, val, value);
      last_op = math_op::Sub;
      break;
    default:
      if (isdigit(expr[i]) || expr[i] == '.' || expr[i] == ',')
      {
        val += (expr[i] == ',') ? '.' : expr[i];
      }
      break;
    }
  }
  math_operation(last_op, prev_val, val, value);
  return value;
}
