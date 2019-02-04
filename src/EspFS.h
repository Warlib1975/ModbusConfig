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

#ifndef EspFS_h
#define EspFS_h

#include "Arduino.h"
#include <FS.h>   //http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html

class EspFS
{
 public:

/*!
    @brief  Create an instance of the ESPSPIFFS class.

    @code

    // Create an instance of the ESPSPIFFS class.
    ESPSPIFFS() fs;

    @endcode
*/
    EspFS();

/*!
    @brief  Attach to a pin and sets that pin's mode (INPUT, INPUT_PULLUP or OUTPUT).
            
    @param    filename
              JSON filename to load.
    @return True if the JSON script parsed successfully, otherwise false.
*/
    bool loadTextFile(char* filename);
    bool loadTextFile();


/*
    @brief   Initialize filesystem. 
    @return True if the filesystem initialization passed successfully, otherwise false.
*/
    bool initFS();

/*
    @brief   Format a filesystem. 
    @return True if the filesystem format passed successfully, otherwise false.
*/
    bool formatFS(); 

/*
    @brief   Show file list of the directory. 
    @return True if the filesystem initialization passed successfully, otherwise false.
*/
    bool showDir();

/*
    @brief   Convert char* string to int. 
    @param   str
             String containing HEX or DEC digit to convert to int.
 
    @return  int with result of conversion.
*/
    char* filename;  
    String text;	

 protected:

 private:
    bool isSPIFFSInitialized; 
};

#endif
