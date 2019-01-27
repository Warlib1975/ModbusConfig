# ModbusConfig
ModbusConfig library for Arduino. 

Mobusa configuration as JSON. Config example is:

  [
  {
    "Slave": {
      "Connection": "ttyS0",
      "PollingInterval": "4000",
      "RetryCount": "10",
      "RetryInterval": "100",
      "HwId": "TermoSensor-0a:01:01:01:01:02",
      "RxPin": "15",
      "TxPin": "13",
      "BaudRate": "9600",
      "Config": "SERIAL_8N1",
      "Ops": [
        {
          "PollingInterval": "2000",
          "UnitId": "1",
          "Function": "0x04",
          "Address": "0x01",
          "Len": "1",
          "DisplayName": "Temp"
        },
        {
          "PollingInterval": "2000",
          "UnitId": "1",
          "Function": "0x04",
          "Address": "0x02",
          "Len": "1",
          "DisplayName": "Humidity"
        }          
      ]
    }
  },
  {
    "Slave": {
      "Connection": "192.168.1.2",
      "PollingInterval": "4000",
      "TcpPort": "502",
      "HwId": "TermoSensor-0a:01:01:01:01:02",
      "Ops": [
        {
          "PollingInterval": "2000",
          "UnitId": "1",
          "Function": "0x04",
          "Address": "0x01",
          "Len": "1",
          "DisplayName": "Temp"
        }  
      ]
    }
  }
]
