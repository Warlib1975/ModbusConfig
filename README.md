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
