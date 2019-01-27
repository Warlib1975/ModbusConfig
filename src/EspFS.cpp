// Please read ModbusConfig.h for information about the liscence and authors

#include "EspFS.h"
#include <FS.h>   //http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html

EspFS::EspFS()
{
  this->isSPIFFSInitialized = false;
  this->initFS();	
}

bool EspFS::loadTextFile()
{
  return this->loadTextFile(this->filename);
}

bool EspFS::loadTextFile(char* filename)
{
  int i;
  this->text = "";

  if (!isSPIFFSInitialized)
  {
    isSPIFFSInitialized = this->initFS();
  }

  if (!isSPIFFSInitialized)
  {
    return false;
  }

  Serial.println("Try to read file [" + String(filename) + "].");

  //Read File data
  File file = SPIFFS.open(filename, "r");
  
  if (!file) 
  {
    Serial.println("File [" + String(file.name()) + "] open failed.");
    return false;	
  }
  else
  {
    Serial.println("Reading Data from File: [" + String(file.name()) + "] with size [" + file.size() + "].");

    //Data from file
    for(i=0;i<file.size();i++) //Read upto complete file size
    {
      this->text += (char)file.read();
    }
    Serial.println(this->text);
    file.close();  //Close file
    Serial.println();
    Serial.println("File Closed.");
  }
  return true;
}

bool EspFS::initFS() 
{
  //Initialize File System
  if(SPIFFS.begin())
  {
    Serial.println("SPIFFS Initialize....ok");
    isSPIFFSInitialized = true;
    return true;
  }
  else
  {
    Serial.println("SPIFFS Initialization...failed");
    return false;
  }
}

bool EspFS::formatFS()
{
  //Format File System
  if(SPIFFS.format())
  {
    Serial.println("File System Formated");
    return true;
  }
  else
  {
    Serial.println("File System Formatting Error");
    return false;	
  }
}

bool EspFS::showDir()
{
  if (!isSPIFFSInitialized)
  {
    isSPIFFSInitialized = this->initFS();
  }

  if (!isSPIFFSInitialized)
  {
    return false;
  }

  String str = "";
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) 
  {
    str += dir.fileName();
    str += " / ";
    str += dir.fileSize();
    str += "\r\n";
  }
  Serial.print(str);
  return true;
}