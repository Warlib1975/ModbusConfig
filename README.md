# ModbusConfig
ModbusConfig library for ESP8266/ESP32/Arduino. 

The library process a sensors configuration file in JSON format and execute operations described in the file. 

## Description

Library supports not only Modbus equipment, but iWare and analog also. 

Each device described in a configuration file has set of the parameters to read data from equipment correspondng PollingInterval property. 

RS485/Modbus devices:

Each device connected to Rx/Tx pin (RS485 interface board) can have several operations block with different polling interval specified in a configuration file.
Any operation has several parameters: SlaveID, function, address, len, operation name etc to visually distinguish operations. 
When a device (slave) polling interval has reached, the libriary invoke the callback function to send telemetry data to a cloud server or poll a slave device.

JSON configuration file should be uploaded to SPIFFS file system on ESP8266/ESP32 boards using corresponding tool: http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html.  

## Config file example
There are several sections in the configuration file:
- Modbus devices
- iWare sensors (DS18B20, DS18S20 and etc.)
- Analog sensors (current 4..20 mA)
- Relays output

### Modbus configuration
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
Main section describes configuration for the RS485 interface. In case of ESP32 it can has 3 UART (hardware), so, it's possible to use no more than 3 pcs. RS485 interface card. Of course, possible to use SoftwareSerial, but reliability is less, than hardware one. 

#### Modbus main section
- "Connection": "tty0",
- "Type" - RS485 type: "RTU" or "TCP".
- "PollingInterval" - an interval of polling equipment in milliseconds. Usually used "PollingInterval" in Operations sections.
- "RetryCount" - how many times poll a equipment in case of error,
- "RetryInterval" - interval in milliseconds between polling a equipment in case of error,
- "HwId" - some human readable name of the equipment. E.g. for inventarization (Assets management). 
- "RxPin" - GPIO for RX,
- "TxPin" - GPIO for TX,
- "BaudRate" - connection speed,
- "Config" - port configuration ("SERIAL_8N1" by default),
- "HardwareSerial" - which hardware serial channel is used. E.g. for ESP32 it could be 2,

#### Modbus operations section
There are several modbus operations are possible. Operations - it's some polling action to get information from modbus equipment. To get info from the modbus device you need to know:
- Slave id - ID of the Modbus equipment
- A function: 
  - 0x01 - Read Coils
  - 0x02 - Read Discrete Inputs
  - 0x03 - Read Holding Registers
  - 0x04 - Read Input Registers
- A register address
- Len of the data to read from a register

Sections in configuration file looks like:
- "PollingInterval" - an interval of polling registers of an equipment in milliseconds.
- "SlaveId" - RS 485 slave ID number,
- "Function" - number of function,
- "Address" - a register address,
- "Len" - len of the data to read,
- "Transform" - formula to transform readed data. E.g. if you specify "0.1", it means that value was readed from the register needs to be multiply by 0.1. You can specify the same formula another way: "%V%/10" - %V% will be replaced by value readed from a sensor and divided by 10. The formula can be simple:   
- "Location" - some location description. E.g. "Warehouse_shelf_10".
- "DisplayName" - some human readable name to display. E.g. "Temp".

### iWare sensors

The structure for the iWare sensors looks similar to modbus: main section to describe interface and some common properties and "Sensors" sections - a little bit equal to "Operations" in Mobus. 

```
  "iWare": [
    {
      "GPIO": 32,
      "PollingInterval": 60000,
      "Connection": "iWare_GPIO32",
	  "Transform": "",
      "Sensors": [
        {
          "HwId": "28B11346920D02A7",
          "PollingInterval": 60000,
          "DisplayName": "Equipment2348",
          "Location": "Warehouse1_Shelf1",
          "Transform": ""
        },
        {
          "HwId": "28FF6C7997090341",
          "PollingInterval": 60000,
          "DisplayName": "Equipment1234",
          "Location": "Warehouse1_Shelf2",
          "Transform": ""
        }
      ]
    },
    {
      "GPIO": 33,
      "PollingInterval": 120000,
      "Connection": "iWare_GPIO33",
	  "Transform": "",
      "Sensors": [
        {
          "HwId": "28B11346920D02A7",
          "PollingInterval": 60000,
          "DisplayName": "Equipment2348",
          "Location": "Warehouse2_Shelf1",
          "Transform": ""
        },
        {
          "HwId": "28FF6C7997090341",
          "PollingInterval": 60000,
          "DisplayName": "Equipment1234",
          "Location": "Warehouse2_Shelf2",
          "Transform": ""
        }
      ]
    }
  ]
```
#### iWare main section
- "GPIO" - GPIO of the microcontroller used as iWare interface. E.g. 32 (without quotes),
- "PollingInterval" - an interval of polling iWare sensor in milliseconds. In case of iWare devices it's important to have such parameter at the level of interface description since sometimes there is no reasons to specify sensors parameters. E.g. iWare devices on the bus can be scanned, so, if you just need to send sensors data to a cloud, there is no need to create sensor section for each scanned sensor.   
- "Connection" - some name of the interface "iWare_GPIO32",
- "Transform" - global formula to transform readed value.  

#### iWare sensors section
- "HwId" - each iWare device has their own unique ID. E.g. "28B11346920D02A7",
- "PollingInterval"- an interval of polling iWare sensor in milliseconds,
- "DisplayName"- some human readable name to display. E.g. "Temp".
- "Location" - some location description. E.g. "Warehouse_shelf_10".
- "Transform"  - formula to transform readed data (see upper).

### Analog sensors

Analog sensors configuration have similar structure as iWare. Usually, analog sensors connected to specific ADC channel (GPIO) on a microcontroller. It can be several ADC channels on a microcontroller. Ana log sensors connect directly to specific GPIO.  
```
  "Analog": [
    {
      "PollingInterval": 120000,
      "Connection": "Analog_test",
      "Sensors": [
        {
          "HwId": "1234",
          "Channel": 6,
          "GPIO": 33,
          "PollingInterval": 60000,
          "DisplayName": "CurrenToVoltage1",
          "Location": "ServerRoom10_Chassis1",
          "Transform": "%V%+0.7"
        },
        {
          "HwId": "4321",
          "Channel": 7,
          "GPIO": 34,
          "PollingInterval": 60000,
          "DisplayName": "CurrenToVoltage2",
          "Location": "ServerRoom11_Chassis1",
          "Transform": "%V%+0.7"
        }
      ]
    }
  ]
  ```
So, only two parameters need to be described more precisely:
- "Channel" - number of ADC channel. E.g. - 6.
- "GPIO" - number of GPIO. E.g. - 33.
