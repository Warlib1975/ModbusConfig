# ModbusConfig
ModbusConfig library for ESP8266/ESP32/Arduino. 

The library process a modbus configuration file in JSON format. 

Each device connected to Rx/Tx pin can have several operations block with different polling interval specified in a configuration file.
Any operation has several parameters: SlaveID, function, address, len, operation name. 
When a device (slave) polling interval has reached, the libriary invoke the callback function to send telemetry data to a cloud server or poll a slave device.


