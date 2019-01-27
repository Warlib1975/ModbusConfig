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

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <ArduinoJson.h>

/**
    @example ArduinoModbusConfig.ino
    Simple example of the ModbusConfig library.   
*/

bool processJsonError(int error);

typedef struct{
  int PollingInterval;  //default 4000
  int UnitId;
  int Function;
  int Address;
  int Len;        	//default 1
  String DisplayName;
} Operation;

typedef std::vector<Operation> OperationsType;

typedef struct{
  String Connection;
  int RxPin;           	//default -1
  int TxPin;            //default -1
  int RetryCount;    	//default 10
  int RetryInterval; 	//default 50 ms
  int PollingInterval; 	//default 4000
  String HwId;
  int BaudRate; 	//default 9600
  String Config; 	//default SERIAL_8N1
  int TcpPort; 		//default 502
  OperationsType Operations;
} Slave;

typedef std::vector<Slave> Slaves;

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

/*
    @brief   Convert char* string to int. 
    @param   str
             String containing HEX or DEC digit to convert to int.
 
    @return  int with result of conversion.
*/
    int StrToHex(const char* str);
 
    DynamicJsonDocument *doc;	
    char* filename;  
    String json;	
    Slaves slaves;
 
 protected:

 private:
    void printValue(String name, String value, bool isHex = false);
};

#endif
