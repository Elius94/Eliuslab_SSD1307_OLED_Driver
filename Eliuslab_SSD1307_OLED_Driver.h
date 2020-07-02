/*
  Test.h - Test library for Wiring - description
  Copyright (c) 2006 John Doe.  All right reserved.
*/

// ensure this library description is only included once
#ifndef Test_h
#define Test_h

#include <SPI.h>

extern const PROGMEM byte Terminal_8pt[];
extern const PROGMEM byte MedProp_11pt[];
extern const PROGMEM byte LCDLarge_24pt[];

// Display Settings
#define SSD1307_DISPLAY_OFF 0xAE // Display Off
#define SSD1307_DISPLAY_CLOCK 0xD5 //SET DISPLAY CLOCK
#define SSD1307_SELECT_MULTIPLEX_RATIO 0xA8 // Select Multiplex Ratio
#define SSD1307_DISPLAY_OFFSET 0xD3 //Setting Display Offset
#define SSD1307_DISPLAY_START_LINE 0x40 //Set Display Start Line
#define SSD1307_SEGMENT_REMAP_DEFAULT 0xA1 //Set Segment Re-Map Default
#define SSD1307_SEGMENT_REMAP_INVERTED 0xA0 //Set Segment Re-Map Default
//0xA0 (0x00) => column Address 0 mapped to 127
//0xA1 (0x01) => Column Address 127 mapped to 0
#define SSD1307_SET_COM_OUTPUT_SCAN_DIRECTION 0xC0 //Set COM Output Scan Direction
#define SSD1307_SET_COM_HW_CONFIGURATION 0xDA //Set COM Hardware Configuration
#define SSD1307_DEFAULT_COM_PIN 0x12 //Alternative COM Pin---See IC Spec page 34
#define SSD1307_SET_CONTRAST_CONTROL 0x81 //Set Contrast Control
#define SSD1307_DFAULT_CONTRAST 0x48
#define SSD1307_SET_PRE_CHARGE_PERIOD 0xD9 //Set Pre-Charge period
#define SSD1307_DEFAULT_PRE_CHARGE_PERIOD 0xA1
#define SSD1307_SET_DESELECT_VCOMH_LEVEL 0xDB //Set Deselect Vcomh level
#define SSD1307_ENTIRE_DISPLAY_ON 0xA4 //Entire Display ON
#define SSD1307_SET_NORMAL_DISPLAY 0xA6 //Set Normal Display
#define SSD1307_DISPLAY_ON 0xAF // Display ON

// Display Scrolling Parameters
#define SSD1307_RIGHT_HORIZONTAL_SCROLL 0x26              ///< Init rt scroll
#define SSD1307_LEFT_HORIZONTAL_SCROLL 0x27               ///< Init left scroll
#define SSD1307_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1307_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A  ///< Init diag scroll
#define SSD1307_DEACTIVATE_SCROLL 0x2E                    ///< Stop scroll
#define SSD1307_ACTIVATE_SCROLL 0x2F                      ///< Start scroll
#define SSD1307_SET_VERTICAL_SCROLL_AREA 0xA3             ///< Set scroll range

#define SSD1307_128_32_RES_X 128
#define SSD1307_128_32_RES_Y 32
#define SSD1307_128_32_GRAM_COL_START 0x00
#define SSD1307_128_32_GRAM_COL_END 0x7F
#define SSD1307_128_32_GRAM_PAGE_START 0
#define SSD1307_128_32_GRAM_PAGE_END 4

#define SPI_SETTINGS SPISettings(8000000, MSBFIRST, SPI_MODE0)

/* Display resolution */
#define BUFFERCOLSIZE 128
#define BUFFERROWSIZE 8

/* Display command register addresses */
#define COLLOWADD 0x00
#define COLHIGHADD 0x10
#define MEMORYADDRESSMODE 0x20
#define SETCOLADDRESS 0x21
#define SETPAGEADDRESS 0x22
#define CHARGEPUMP 0x8D
#define SETMUXRATIO 0xA8
#define DISPLAYONADD 0xAE
#define PAGEADD 0xB0
#define STARTLINEADD 0x40
#define SCANDIRECTIONADD 0xC0
#define SEGMENTMAPADD 0xA0

/* Command register parameters */
#define SCANDIRNORMAL 0x00
#define SCANDIRREVERSE 0x08
#define SEGMENTMAPNORMAL 0
#define SEGMENTMAPREVERSE 1
#define ENABLECHARGEPUMP 0x14
#define HORIZONTALADDRESSMODE 0x00


/* Various draw modes */
enum HCuOLEDDrawModes
{
	NORMAL    = 1,
	INVERT    = 2,
	CLEAR     = 3,
	OUTLINE   = 4,
	SOLID     = 5
	
};

// library interface description
class Eliuslab_OLED
{
  // user-accessible "public" interface
  public:
    Eliuslab_OLED(uint8_t _width, uint8_t _height, uint8_t _ssPin, uint8_t _dcPin, uint8_t _rstPin);
    void begin(void);
    void startScrollRight(uint8_t start, uint8_t _stop);
    void startScrollLeft(uint8_t start, uint8_t _stop);
    void startScrollDiagRight(uint8_t start, uint8_t _stop);
    void stopScroll(void);
    void full_on(void);
    void full_off(void);
    void reset(void);
    void test(void);

    // DRAWING FUNCTIONS
    void Refresh(void);
    static void ClearBuffer(void);
    //void ClearDisplayRam(void);
    void Flip_H(void);
    void Flip_V(void);
    static void Bitmap(uint8_t Cols, uint8_t ByteRows, const uint8_t BitmapData[]);
    static void Cursor(uint8_t X, uint8_t Y);
    static void Print(char TextString[]);
    static void Print(long Value);
    static void Print(int long Value, byte DecimalPlaces);
    static void Print(float value, byte digits, byte DecimalPlaces);
    static void SetFont(const byte *Font);
    static void Plot(uint8_t X, uint8_t Y);
    static void Line(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2);
    static void Rect(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2, uint8_t FillMode);
    static void Erase(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2);
    static void DrawMode(byte DrawMode);

  // library-accessible "private" interface
  private:
    uint8_t width, height;
    uint8_t ssPin, dcPin, rstPin;
    void Initial_SSD1307ZD(void);
    void Write_command(unsigned char command);
    void Write_data(unsigned char data);
    unsigned char testBitmap[4][128];
    static void _WriteChar(char character);
    boolean _V_Ori;
    boolean _H_Ori;
    static byte _XPos;
    static byte _YPos;
    static const byte *_FontType;
    static const uint16_t *_FontDescriptor;
    static byte _FontHight;
    //byte _DisplayRAMColSize; 
    static byte _DrawMode;
    byte _Res_Max_X;
    byte _Res_Max_Y;
    byte _GRAM_Col_Start;
    byte _GRAM_Col_End;
    byte _GRAM_Page_Start;
    byte _GRAM_Page_End;
    byte _RAM_Pages;
};

#endif

