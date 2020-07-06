#ifndef fonts_h
#define fonts_h
#include <avr/pgmspace.h>
#include "Arduino.h"
//#include "MemorySave.h"


typedef struct _font_info 
{ 
   byte CharacterHeight; 
   char StartCharacter; 
   char EndCharacter; 
   const unsigned int *Descriptors; 
   const byte *Bitmaps; 
   //int Bitmaps;
}FONT_INFO; 


#include "Terminal_8pt.h"
#include "MedProp_11pt.h"
#include "LargeProp_25pt.h"
#include "LCDLarge_24pt.h"
#include "LCDLarge_52pt.h"


#endif