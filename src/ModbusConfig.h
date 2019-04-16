/*
  The MIT License (MIT)

  Copyright (c) 2019 Andrey Fedorov

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
  Main code by Andrey Fedorov (fedorov@bizkit.ru)
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef ModbusConfig_h
#define ModbusConfig_h

#include "Arduino.h"

#include <ArduinoJson.h>

/**
    @example ArduinoModbusConfig.ino
    Simple example of the ModbusConfig library.   
*/

bool processJsonError(int error);

enum SensorType { Modbus, iWare, Analog, Relay, DryContact, NONE };

struct BaseOperation
{
	String HwId;                  //Sensor HwId or inventory number
	int PollingInterval;      	  //default 5000
	String Transform;		      //Formulas to transform input data
	String DisplayName;           //Name to display 
	String Location;              //Location of the sensor 
  unsigned long lastPolling;      //Last polling in millis
};

struct AnalogSensor : BaseOperation
{
	int Channel; 			//ADC channel number
	int GPIO; 			//GPIO number
};

struct iWareSensor : BaseOperation
{
	
};

struct RelayOutput : BaseOperation
{
	int GPIO; 			//GPIO number
};

//typedef std::vector<iWareSensor*> iWareSensors;

typedef std::vector<BaseOperation*> OperationsType;

struct BaseConnection
{
  String Connection;
  SensorType Sensor;
  unsigned long lastPolling; //
  int PollingInterval; 	    //default 5000
  OperationsType Operations;
};

struct iWareConnection : BaseConnection
{
  int GPIO;
  //iWareSensors Sensors; 
};


//typedef std::vector<AnalogSensor*> AnalogSensors; //Array of the analog sensors

struct AnalogConnection : BaseConnection
{
	
};

struct RelayConnection : BaseConnection
{
	
};

//typedef std::vector<iWareConnection*> iWareConnections;


struct ModbusOperation : BaseOperation
{
  int SlaveId;              //default 1
  void* Modbus;
  int Function;
  int Address;
  int Len;
};

//typedef std::vector<ModbusOperation> OperationsType;

enum ModbusType { RTU, TCP };

struct ModbusConnection : BaseConnection
{
  void* Connector;
  ModbusType Type = ModbusType :: RTU;
  int RxPin;           	    //default -1
  int TxPin;                //default -1
  int HardwareSerial = -1;  //Hardware port number or -1 for SoftwareSerial  
  int RetryCount;    	    //default 10
  int RetryInterval; 	    //default 1000
  String HwId;
  int BaudRate; 	        //default 9600
  String Config; 	        //SERIAL_8N1
  int TcpPort; 		        //default -1 
//  OperationsType Operations;
};

typedef std::vector<BaseConnection*> Connections;

/*
    @brief  Callback function to process an operation polling interval.
*/
//typedef void (*PHandler)(Connection* connection, Operation* operation);
typedef void (*OnSensorPollingHandler)(BaseConnection* connection, BaseOperation* operation);

/**
     The ModbusConfig class.
*/
class ModbusConfig
{
 public:

/*!
    @brief  Create an instance of the ModbusConfig class.

    @code
    // Create an instance of the ModbusConfig class.
    ModbusConfig() modbus;

    @endcode
*/
    ModbusConfig();


/*!
    @brief  Parse Modbus Config JSON file.
            
    @param    json
              JSON file with modbus configuration.
    @return True if the JSON script parsed successfully, otherwise false.
*/
    bool parseConfig(String json);
    bool parseConfig();		

/*!
    @brief  Print loaded and parsed config to Serial.
*/
    void printConfig();

/*!
    @brief  Polling interval periodical checker and callback function executer if some operation should be processed.
*/
    void loopModbusConfig();


/*
    @brief   Convert char* string to int. 
    @param   str
             String containing HEX or DEC digit to convert to int.
 
    @return  int with result of conversion.
*/
    int StrToHex(const char* str);
 
    OnSensorPollingHandler pollingIntervalCallback;
    DynamicJsonDocument *doc;	
    char* filename;  
    String json;	
    Connections connections;
 
 protected:

 private:
    //unsigned int minSlavePollingInterval = 0;
    //unsigned int lastPolling = 0;
    void printValue(String name, String value, bool isHex = false);
    void printOperations(SensorType sensorType, OperationsType operations);
};

#endif
