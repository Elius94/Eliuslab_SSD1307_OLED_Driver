/*
  Eliuslab_SSD1307_OLED_Driver.h - Eliuslab_SSD1307_OLED_Driver library for ATMEGA328P - implementation
  Copyright (c) 2020 Elia Lazzari.  All right reserved.
*/

// include this library's description file
#include "Eliuslab_SSD1307_OLED_Driver.h"
#include "fonts.h"
#include <avr/pgmspace.h>

/* Display output buffer */
byte DisplayBuffer[BUFFERCOLSIZE][8];


/* Variables and pointer used by shared static member functions */
byte Eliuslab_OLED::_XPos;
byte Eliuslab_OLED::_YPos;
byte Eliuslab_OLED::_DrawMode;
const byte *Eliuslab_OLED::_FontType;
const uint16_t *Eliuslab_OLED::_FontDescriptor;
byte Eliuslab_OLED::_FontHight;

// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

Eliuslab_OLED::Eliuslab_OLED(uint8_t _width, uint8_t _height, uint8_t _ssPin, uint8_t _dcPin, uint8_t _rstPin)
{
  width = _width;
  height = _height;
  ssPin = _ssPin;
  dcPin = _dcPin;
  rstPin = _rstPin;
  
  pinMode(ssPin, OUTPUT);
  pinMode(dcPin, OUTPUT);
  pinMode(rstPin, OUTPUT);

  /* Set vertical and horizontal orientation of the display */
	_V_Ori = 0;
	_H_Ori = 0;

  _Res_Max_X = SSD1307_128_32_RES_X;
  _GRAM_Col_Start = SSD1307_128_32_GRAM_COL_START;
  _GRAM_Col_End = SSD1307_128_32_GRAM_COL_END;
  _GRAM_Page_Start = SSD1307_128_32_GRAM_PAGE_START;
  _GRAM_Page_End = SSD1307_128_32_GRAM_PAGE_END;
  _RAM_Pages = SSD1307_128_32_GRAM_PAGE_END - SSD1307_128_32_GRAM_PAGE_START + 1;

  /* Wait 100mS for DC-DC to stabilise. This can probably be reduced */
	delay(100);

  /* Set text cursor to top corner */  
	Cursor(0, 0);
  
	/* Set default font */
	SetFont(Terminal_8pt);  
  
	/* Set default draw mode */
	DrawMode(NORMAL);
}

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries

void Eliuslab_OLED::begin(void) {
  digitalWrite(dcPin, LOW); // Set display data mode pin
  SPI.begin();
  reset();
}

void Eliuslab_OLED::Initial_SSD1307ZD(void) {
  //Start OLED configuration via SPI
  Write_command(SSD1307_DISPLAY_OFF); // Display Off
  Write_command(SSD1307_DISPLAY_CLOCK); //SET DISPLAY CLOCK
  Write_command(0x80); //
  Write_command(SSD1307_SELECT_MULTIPLEX_RATIO); // Select Multiplex Ratio
  Write_command(height-1); // Default => 0x3F (1/64 Duty) 0x1F(1/32 Duty)
  Write_command(SSD1307_DISPLAY_OFFSET); //Setting Display Offset
  Write_command(0x00); //00H Reset
  Write_command(SSD1307_DISPLAY_START_LINE); //Set Display Start Line
  Write_command(SSD1307_SEGMENT_REMAP_DEFAULT); //Set Segment Re-Map Default
  //0xA0 (0x00) => column Address 0 mapped to 127
  //0xA1 (0x01) => Column Address 127 mapped to 0
  Write_command(SSD1307_SET_COM_OUTPUT_SCAN_DIRECTION); //Set COM Output Scan Direction
  Write_command(SSD1307_SET_COM_HW_CONFIGURATION); //Set COM Hardware Configuration
  Write_command(SSD1307_DEFAULT_COM_PIN); //Alternative COM Pin---See IC Spec page 34
  Write_command(SSD1307_SET_CONTRAST_CONTROL); //Set Contrast Control
  Write_command(SSD1307_DFAULT_CONTRAST);
  Write_command(SSD1307_SET_PRE_CHARGE_PERIOD); //Set Pre-Charge period
  Write_command(SSD1307_DEFAULT_PRE_CHARGE_PERIOD);
  Write_command(SSD1307_SET_DESELECT_VCOMH_LEVEL); //Set Deselect Vcomh level
  Write_command(0x00);
  Write_command(SSD1307_ENTIRE_DISPLAY_ON); //Entire Display ON
  Write_command(SSD1307_SET_NORMAL_DISPLAY); //Set Normal Display
  Write_command(SSD1307_DISPLAY_ON); // Display ON
}

void Eliuslab_OLED::reset(void) {
  digitalWrite(rstPin, HIGH);
  delay(10);
  digitalWrite(rstPin, LOW);
  delay(100);
  digitalWrite(rstPin, HIGH);

  delay(10);
  Initial_SSD1307ZD();

  /* Clear the display buffer */
	ClearBuffer();
  /* Output the display buffer to clear the display RAM */
	Refresh();
  /* Flip the display */
	/*Flip_H();
	Flip_V();	*/
}

void Eliuslab_OLED::Write_command(unsigned char command) {
  //unsigned char bMask;
  // take the SS pin low to select the chip:
  digitalWrite(ssPin, LOW);
  digitalWrite(dcPin, LOW);

  SPI.beginTransaction(SPI_SETTINGS);
  SPI.transfer(command);
  SPI.endTransaction();

  // take the SS pin high to de-select the chip:
  digitalWrite(ssPin, HIGH);
  digitalWrite(dcPin, HIGH);
}

void Eliuslab_OLED::Write_data(unsigned char data) {
  //unsigned char bMask;
  // take the SS pin low to select the chip:
  digitalWrite(ssPin, LOW);
  digitalWrite(dcPin, HIGH);
  SPI.beginTransaction(SPI_SETTINGS);
  SPI.transfer(data);
  SPI.endTransaction();

  // take the SS pin high to de-select the chip:
  digitalWrite(ssPin, HIGH);
  digitalWrite(dcPin, LOW);
}

// SCROLLING FUNCTIONS -----------------------------------------------------
// To scroll the whole display, run: display.startscrollright(0x00, 0x0F)
void Eliuslab_OLED::startScrollRight(uint8_t start, uint8_t _stop) {
  Write_command(SSD1307_RIGHT_HORIZONTAL_SCROLL);
  Write_command(0x00);
  Write_command(start);
  Write_command(0X00);
  Write_command(_stop);
  
  Write_command(0x00);
  Write_command(0xff);
  Write_command(SSD1307_ACTIVATE_SCROLL);
}

// To scroll the whole display, run: display.startscrollleft(0x00, 0x0F)
void Eliuslab_OLED::startScrollLeft(uint8_t start, uint8_t _stop) {
  Write_command(SSD1307_LEFT_HORIZONTAL_SCROLL);
  Write_command(0x00);
  Write_command(start);
  Write_command(0X00);
  Write_command(_stop);
  
  Write_command(0x00);
  Write_command(0xff);
  Write_command(SSD1307_ACTIVATE_SCROLL);
}

// display.startscrolldiagright(0x00, 0x0F)
void Eliuslab_OLED::startScrollDiagRight(uint8_t start, uint8_t _stop) {
  Write_command(SSD1307_SET_VERTICAL_SCROLL_AREA);
  Write_command(0x00);
  Write_command(32);
  Write_command(SSD1307_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
  Write_command(0x00);
  Write_command(start);
  Write_command(0x00);
  Write_command(_stop);
  Write_command(0x01);
  Write_command(SSD1307_ACTIVATE_SCROLL);
}

void Eliuslab_OLED::stopScroll(void) {
  Write_command(SSD1307_DEACTIVATE_SCROLL);
}

void Eliuslab_OLED::full_on() {
  unsigned char x, y, Z = 0xb0;
  for (x = 0; x < SSD1307_128_32_GRAM_PAGE_END; x++)
  {
    Write_command(Z);
    Z++;
    Write_command(0x10);
    Write_command(0x00);
    for (y = 0; y < SSD1307_128_32_RES_X; y++)
    {
      Write_data(0xff);
    }
  }
}

void Eliuslab_OLED::full_off() {
  unsigned char x, y, Z = 0xb0;
  for (x = 0; x < SSD1307_128_32_GRAM_PAGE_END; x++)
  {
    Write_command(Z);
    Z++;
    Write_command(0x10);
    Write_command(0x00);
    for (y = 0; y < SSD1307_128_32_RES_X; y++)
    {
      Write_data(0x00);
    }
  }
}

void Eliuslab_OLED::test(void) {
  //=====================================================================================
  /*unsigned char testBitmap[4][128] = { //Position(coordinate)
    0xFF, 0x11, 0x01, 0x01, 0x03, 0x01, 0x41, 0x81,
    0x41, 0x8F, 0x41, 0x81, 0x41, 0x81, 0x43, 0x81,
    0x41, 0x81, 0x41, 0x8F, 0x41, 0x81, 0x41, 0x81,
    0x43, 0x81, 0x41, 0x81, 0x41, 0x8F, 0x41, 0x81,
    0x41, 0x81, 0x43, 0x81, 0x41, 0x81, 0x41, 0x8F,
    0x41, 0x81, 0x41, 0x01, 0x03, 0x01, 0x81, 0x81,
    0x81, 0x0F, 0x81, 0x81, 0x81, 0x01, 0x03, 0x01,
    0x41, 0x81, 0x41, 0x8F, 0x41, 0x81, 0x41, 0x81,
    0x43, 0x81, 0x41, 0x81, 0x41, 0x8F, 0x41, 0x81,
    0x41, 0x81, 0x43, 0x81, 0x41, 0x81, 0x41, 0x8F,
    0x41, 0x81, 0x41, 0x81, 0x43, 0x81, 0x41, 0x81,
    0x41, 0x8F, 0x41, 0x81, 0x41, 0x01, 0x03, 0x01,
    0x81, 0x01, 0x81, 0x8F, 0x81, 0x01, 0x81, 0x81,
    0x83, 0x01, 0x01, 0x01, 0x41, 0x8F, 0x41, 0x81,
    0x41, 0x81, 0x43, 0x81, 0x41, 0x81, 0x41, 0x8F,
    0x41, 0x81, 0x41, 0x81, 0x43, 0x81, 0x41, 0xFF,
    0xFF, 0x42, 0x02, 0x02, 0x00, 0x00, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0x80, 0x40, 0x80, 0x4B, 0x8A,
    0x4E, 0x80, 0x4F, 0x88, 0x4F, 0x80, 0x40, 0x80,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0x80, 0x40, 0x80,
    0x4F, 0x80, 0x4F, 0x88, 0x4F, 0x80, 0x4F, 0x88,
    0x4F, 0x80, 0x40, 0x80, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xFF,
    0xFF, 0x08, 0x08, 0x08, 0x00, 0x00, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xFF,
    0xFF, 0xA1, 0xA0, 0xA0, 0x80, 0x80, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA,
    0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xAA, 0xD5, 0xFF
  };

  unsigned char x, y, Z = 0xB0;
  for (x = 0; x < 4; x++)
  {
    Write_command(Z);
    Z++;
    Write_command(0x10);
    Write_command(0x00);
    for (y = 0; y < 128; y++)
    {
      Write_data(testBitmap[x][y]);
    }
  }*/
}

/* Writes the contents of the display buffer to the display */
void Eliuslab_OLED::Refresh(void) {
	byte ColIndex;
	byte RowIndex;   
	byte Temp1, Temp2;
     
  /* set graphics ram start and end columns */
  Write_command(SETCOLADDRESS);
  Write_command(_GRAM_Col_Start);
  Write_command(_GRAM_Col_End);
  
  /* set graphics ram start and end pages */ 
  Write_command(SETPAGEADDRESS);
  Write_command(_GRAM_Page_Start);
  Write_command(_GRAM_Page_End);
    	
		
	for (RowIndex = 0; RowIndex < _RAM_Pages; RowIndex++)
	{
    /* Write to display RAM */
    for (ColIndex = 0; ColIndex < _Res_Max_X; ColIndex++)
    {
      Write_data(DisplayBuffer[ColIndex][RowIndex]);
    }
	}
}

/* Clears the contents of the output buffer */
void Eliuslab_OLED::ClearBuffer(void) {
	byte ColIndex;
	byte RowIndex;

	for (RowIndex = 0; RowIndex < 8; RowIndex++)
	{    
		for (ColIndex = 0; ColIndex < BUFFERCOLSIZE; ColIndex++)
		{
		  DisplayBuffer[ColIndex][RowIndex] = 0x00;
		}
	}
}

// DRAWING FUNCTIONS

/* Flip the horizontal orientation of the screen */
void Eliuslab_OLED::Flip_H(void) {
	_H_Ori = ~_H_Ori;

  if(_H_Ori)
  {
    Write_command(SCANDIRECTIONADD | SCANDIRREVERSE);
  }else
  {
    Write_command(SCANDIRECTIONADD | SCANDIRNORMAL);
  }
}

/* Flip the vertical orientation of the screen */
void Eliuslab_OLED::Flip_V(void) {
	_V_Ori = ~_V_Ori;
  
  if(_V_Ori)
  {
    Write_command(SEGMENTMAPADD | SEGMENTMAPREVERSE);
  }else
  {
    Write_command(SEGMENTMAPADD | SEGMENTMAPNORMAL);
  }
}

/* Write bitmap data to the LCD starting at the cursor location where:
   Cols is the number byte columns to write to.
   ByteRows is the number of rows to write to in 8 pixel chunks 
   BitmapData is an array containing the bitmap data to be written */

void Eliuslab_OLED::Bitmap(uint8_t Cols, uint8_t ByteRows, const uint8_t BitmapData[]) {
	byte XIndex;
	byte YIndex;
  
	byte BufRow;
	byte BufX;

	unsigned int BitmapIndex;
  
	/* Step through each 8 pixel row */
	for (YIndex = 0; YIndex < ByteRows; YIndex++)
	{
		/* Step through each column */
		for (XIndex = 0; XIndex < Cols; XIndex++)
		{
			BufX = XIndex + _XPos;
      
			/* If column is beyond display area then don't bother writing to it*/
			if(BufX < BUFFERCOLSIZE)
			{
				BufRow = YIndex + (_YPos / 8);
				BitmapIndex = (YIndex * Cols)+ XIndex;
      
				/* If row is beyond the display area then don't bother writing to it */
				if(BufRow < BUFFERROWSIZE)
				if (_DrawMode == NORMAL)
				{
					DisplayBuffer[BufX][BufRow] |= pgm_read_byte_near(&BitmapData[BitmapIndex]) << (_YPos%8);					
				}else
				{
					DisplayBuffer[BufX][BufRow] ^= pgm_read_byte_near(&BitmapData[BitmapIndex]) << (_YPos%8);
				}

				/* If column data overlaps to 8 bit rows then write to the second row */  
				if(_YPos%8 && (BufRow +1) < BUFFERROWSIZE)
				if (_DrawMode == NORMAL)
				{
					DisplayBuffer[BufX][BufRow+1] |= pgm_read_byte_near(&BitmapData[BitmapIndex]) >> (8 - (_YPos%8));
				}else
				{
					DisplayBuffer[BufX][BufRow+1] ^= pgm_read_byte_near(&BitmapData[BitmapIndex]) >> (8 - (_YPos%8));		  
				}
			}
		}
	}
}

/* Write to a single pixel on the display where:
   X is the x axis coordinate of the pixel
   Y is the Y axis coordinate of the pixel */
void Eliuslab_OLED::Plot(uint8_t X, uint8_t Y) {
	byte row = Y / BUFFERROWSIZE;
	
	if(X < BUFFERCOLSIZE && row < BUFFERROWSIZE)
		if (_DrawMode == NORMAL)
		{
			DisplayBuffer[X][row] |=  (0x01 << (Y % 8));
		}else if(_DrawMode == INVERT)
		{
			DisplayBuffer[X][row] ^=  (0x01 << (Y % 8));	
		}else if(_DrawMode == CLEAR)
		{
		DisplayBuffer[X][row] &=  ~(0x01 << (Y % 8));	
		}
}

/* Draw a line where:
   X1 is the starting X axis coordinate of the line
   Y1 is the starting Y axis coordinate of the line
   X2 is the starting X axis coordinate of the line
   Y2 is the starting Y axis coordinate of the line */
void Eliuslab_OLED::Line(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2) {
	double step;
	int x, y;
 
	/* If start coordinates are below and to the right of the end coordinate then flip them */
	if((X2 <= X1 && Y2 <= Y1) || (X2 >= X1 && Y1 >= Y2) || (X2 >= X1 && Y1 >= Y2)) 
	{
		X2^=X1;
		X1^=X2;
		X2^=X1;
		Y2^=Y1;
		Y1^=Y2;
		Y2^=Y1;
	}

	/* If X axis is wider than Y axis then step though X axis */
	if(((X2-X1) >= (Y2-Y1)) || ((X1 - X2) >= (Y2-Y1)))
	{
		step = (double)(Y2-Y1) / (X2-X1);
		if(X2 >= X1 && Y2 >= Y1)
		{
			for(x = X1; x <= X2; x++)
				Plot(x, ((x-X1) * step) + Y1); 
		}else
		{
			for(x = X1; x >= X2; x--)
				Plot(x, Y2 + ((x-X2) * step));
		} 
	}else /* If Y axis is wider than X axis then step though Y axis */
	{
		step = (double)(X2-X1) / (Y2-Y1);
		if(Y2 >= Y1 && X2 >= X1)
		{
			for(y = Y1; y <= Y2; y++)
				Plot(((y-Y1) * step) + X1, y); 
		}else
		{
			for(y = Y2; y >= Y1; y--)
				Plot(X2 + ((y-Y2) * step),y);	
	}
  }
}

/* Draw a rectangle where:
   X1 is the X axis coordinate of the first corner
   Y1 is the Y axis coordinate of the first corner
   X2 is the X axis coordinate of the opposite corner
   Y2 is the Y axis coordinate of the opposite corner 
   FillMode is sets the drawing mode for the rectangle. Options are:
   OUTLINE (Draws an outlined rectangle with no fill
   SOLID (Draws a filled filled rectangle) */
   
void Eliuslab_OLED::Rect(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2, uint8_t FillMode) {
	byte y;

	/* Draw the top and bottom borders */
	Line(X1, Y1, X2, Y1);
	Line(X1, Y2, X2, Y2);
  
	/* If first corner is below second corner then flip the coordinates */
	if(Y1 > Y2)
	{
		Y2^=Y1;
		Y1^=Y2;
		Y2^=Y1;  
	}
  
	/* If rectangle is wider than two pixels set Y for drawing vertical borders */
	if(Y2-Y1 > 1)
	{
		Y1++;
		Y2--;
	}
 
	/* If box is solid then fill area between top and bottom border */
	if(FillMode == SOLID)
	{
		for(y = Y1; y <= Y2; y++)
			Line(X1, y, X2, y);
	}else  /* if not solid then just draw vertical borders */
	{
		Line(X1, Y1, X1, Y2);
		Line(X2, Y1, X2, Y2); 
	}
}

/* Clears an area of the display buffer where:
   X1 is the X axis coordinate of the first corner
   Y1 is the Y axis coordinate of the first corner
   X2 is the X axis coordinate of the opposite corner
   Y2 is the Y axis coordinate of the opposite corner 
   
   Thanks to vladyslav-savchenko for improved version */
void Eliuslab_OLED::Erase(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2) {
	byte temp = _DrawMode;
	_DrawMode = CLEAR;

	for (int column = X1; column <= X2; column ++) 
		for (int row = Y1; row <= Y2; row ++) 
			Plot(column, row);
		
	_DrawMode = temp;
 }

/* Sets the drawing mode for text and graphics where:
   DrawMode sets the drawing mode. Options are:
   NORMAL (Pixels are set to foreground colour) 
   INVERT (Pixels are set to the opposite of their current state) 
   CLEAR  (Pixels are set to background colour) */

void Eliuslab_OLED::DrawMode(byte DrawMode) {
	_DrawMode = DrawMode;
}

/* Sets the location of the cursor for text and bitmap graphics where:
   X and Y are the starting top left X & Y axis coordinates */

void Eliuslab_OLED::Cursor(uint8_t X, uint8_t Y) {
	_XPos = X;
	_YPos = Y;
}

/* Print a text string to the LCD starting at the cursor coordinates where:
   TextString[] is a string array containing the text to be displayed */
void Eliuslab_OLED::Print(char TextString[]) {
	byte StringLength;
	byte Index;
  
	/* Finds length of string */
	StringLength = strlen(TextString) - 1;

	for (Index = 0; Index <= StringLength; Index++)
	{
		_WriteChar(TextString[Index] - 32);
	}
}

/* Print a floating point number to the LCD starting at the cursor coordinates where:
   value is the floating point value to display (max 10 digits including decimal places)
   digits is the number of digits to crop the number to (including decimal places)
   DecimalPlaces is the number of decimal places to display */
   
void Eliuslab_OLED::Print(float value, byte digits, byte DecimalPlaces) {
	char Buffer[10];
  
	/* Clip the number of digits to 10 */
	if (digits > 10)
		digits = 10;

	/* Convert the value to an character array */ 
	dtostrf(value, digits, DecimalPlaces, Buffer);
  
	/* Output the array to the display buffer */
	Print(Buffer);
}

/* Print a signed integer number to the LCD at the current cursor 
   coordinates where:
   Value is signed integer number of type long */
void Eliuslab_OLED::Print(long Value) {
	byte Digits[10];
	int long Temp;
	byte NumDigits = 0;
  
	/* Is the number negative ? */
	if (Value < 0)
	{
		_WriteChar(13);
		Temp = Value * -1;
	}else
	{
		Temp = Value;
	}
  
	/* Store each digit in a byte array so that they 
	   can be printed in reverse order */
	while (Temp)
	{
		Digits[NumDigits] = Temp % 10;
		Temp /= 10;
		NumDigits++;
	} 

	/* Print each digit */
	while(NumDigits)
	{
		NumDigits--;
		_WriteChar(Digits[NumDigits] + 16);
	}
}

/* Print a signed integer number with decimal point to the LCD at 
   the current cursor coordinates where:
   Value is signed integer number of type long
   DecimalPlaces is the position of the decimal point */
void Eliuslab_OLED::Print(int long Value, byte DecimalPlaces) {
	byte Digits[10];
	int long Temp;
	byte NumDigits = 0;
  
	/* Is the number negative ? */
	if (Value < 0)
	{
		_WriteChar(13); 
		Temp = Value * -1;
	}else
	{
		Temp = Value;
	}
  
	/* Store each digit in a byte array so that they 
	   can be printed in reverse order */
	while (Temp)
	{
		Digits[NumDigits] = Temp % 10;
		Temp /= 10;
		NumDigits++;
	} 

	/* If the decimal point is at the beginning of the 
	   number then pad it with a zero */ 
	if(DecimalPlaces == NumDigits)
	{
		_WriteChar(16);
	}
  
	/* Print each digit */
	while(NumDigits)
	{
		NumDigits--;
		if (NumDigits + 1 == DecimalPlaces)
			_WriteChar(14);
	
		_WriteChar(Digits[NumDigits] + 16);
	}
}

/* Write a single character or digit at the current cursor coordinate */
void Eliuslab_OLED::_WriteChar(char character) {
  const byte *FontByteAddress;
  int FontWidth;
	
	FontByteAddress = _FontType + pgm_read_word_near(_FontDescriptor + (character * 2) + 1);
	FontWidth = pgm_read_word_near(_FontDescriptor + (character * 2));
	Bitmap(FontWidth, _FontHight, FontByteAddress);  
	_XPos = _XPos + FontWidth + 2;
}

/* Sets the font to be used when writing text or numbers where:
   *Font is the font type to be used. Options are:
   Terminal_8pt (A small 8x8 fixed width font)
   MedProp_11pt (A medium two row proportional font)
   LCDLarge_24pt (A large 4 row LCD style font) */
void Eliuslab_OLED::SetFont(const byte *Font) {
	if(Font == Terminal_8pt)
	{
		_FontType = Terminal_8pt;
		_FontHight = Terminal_8ptFontInfo.CharacterHeight;
		_FontDescriptor = Terminal_8ptFontInfo.Descriptors;
	}else if(Font == MedProp_11pt)
	{
		_FontType = MedProp_11pt;
		_FontHight = MedProp_11ptFontInfo.CharacterHeight;
		_FontDescriptor = MedProp_11ptFontInfo.Descriptors;
	}else if(Font == LCDLarge_24pt  )
	{
		_FontType = LCDLarge_24pt;
		_FontHight = LCDLarge_24ptFontInfo.CharacterHeight;
		_FontDescriptor = LCDLarge_24ptFontInfo.Descriptors;
	}
}