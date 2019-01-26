// Please read ModbusConfig.h for information about the liscence and authors

#include "ModbusConfig.h"

ModbusConfig::ModbusConfig()
{}

int ModbusConfig::StrToHex(const char* str)
{
  if (strchr(str, 'x') != NULL )
  { 
    return (int) strtol(str, 0, 16);
  }
  return (int) strtol(str, 0, 10);
}

void ModbusConfig::parseModbusConfig(String json)
{

}
