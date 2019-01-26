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
    Simple example of the ModbusConfig library. Load Modbus config from SPIFFS storage.  
*/

void processError(int error);

typedef struct{
  int PollingInterval;
  int UnitId;
  int Function;
  int Address;
  int Len;
  String DisplayName;
} Operation;

typedef std::vector<Operation> OperationsType;

typedef struct{
  String Connection;
  String HwId;
  int BaudRate;
  int DataBits;
  int StopBits;
  String Parity;
  String FlowControl;
  int TcpPort;
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
    @brief  Attach to a pin and sets that pin's mode (INPUT, INPUT_PULLUP or OUTPUT).
            
    @param    filename
              JSON filename to load.
    @return True if the JSON script parsed successfully, otherwise false.
*/
    bool loadConfig(char* filename);
    bool loadConfig();


/*
    @brief   Initialize filesystem. 
*/
    bool initFS();

/*
    @brief   Format a filesystem. 
*/
    bool formatFS(); 

/*
    @brief   Show file list of the directory. 
*/
    void showDir();

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

 protected:

 private:
    void printValue(String name, String value);
    bool isSPIFFSInitialized; 
};

#endif
