# ModbusConfig
ModbusConfig library for ESP8266/ESP32/Arduino. 

The library process a sensors configuration file in JSON format and execute operations described in the file. 

# Description

Library supports not only Modbus equipment, but iWare and analog also. 

Each device described in a configuration file has set of the parameters to read data from equipment correspondng PollingInterval property. 

RS485/Modbus devices:

Each device connected to Rx/Tx pin (RS485 interface board) can have several operations block with different polling interval specified in a configuration file.
Any operation has several parameters: SlaveID, function, address, len, operation name etc to visually distinguish operations. 
When a device (slave) polling interval has reached, the libriary invoke the callback function to send telemetry data to a cloud server or poll a slave device.

JSON configuration file should be uploaded to SPIFFS file system on ESP8266/ESP32 boards using corresponding tool: http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html.  

# Config file example
There are several sections in the configuration file:
- Modbus devices
- iWare sensors (DS18B20, DS18S20 and etc.)
- Analog sensors (current 4..20 mA)
- Relays output

## Modbus configuration
```
  "Modbus": [
    {
      "Connection": "tty0",
      "Type": "RTU",
      "PollingInterval": 60000,
      "RetryCount": 20,
      "RetryInterval": 100,
      "HwId": "RS485TermoSensor",
      "RxPin": 5,
      "TxPin": 13,
      "BaudRate": 9600,
      "Config": "SERIAL_8N1",
      "HardwareSerial": 2,
      "Ops": [
        {
          "PollingInterval": 60000,
          "SlaveId": 1,
          "Function": 4,
          "Address": 1,
          "Len": 1,
          "Transform": "0.1",
          "Location": "Warehouse1_Shelf1",
          "DisplayName": "Temp"
        },
        {
          "PollingInterval": 60000,
          "SlaveId": 1,
          "Function": 4,
          "Address": 2,
          "Len": 1,
          "Transform": "%V%*0.1",
          "Location": "Warehouse1_Shelf1",
          "DisplayName": "Humidity"
        }
      ]
    }
  ]
```  
