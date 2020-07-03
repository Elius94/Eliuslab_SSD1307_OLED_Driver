#include <Eliuslab_SSD1307_OLED_Driver.h>

#define CS_PIN 10
#define DC_PIN 8
#define RST_PIN 9
#define SERVICE_LED 14

#define OLED_WIDTH 128
#define OLED_HEIGHT 32

Eliuslab_OLED OLED(OLED_WIDTH, OLED_HEIGHT, CS_PIN, DC_PIN, RST_PIN);

void setup() {
  pinMode(SERVICE_LED, OUTPUT);
  OLED.begin(); //Init the OLED
}

void loop() {
  // put your main code here, to run repeatedly:
  //OLED.test();
  /* Display some text using a large 4 line LCD style font */
  OLED.Cursor(20,0);
  OLED.SetFont(MedProp_11pt);
  OLED.Print("Ciao KORISS! <3");
  

  /*OLED.Rect(21,25,32,31, SOLID);
  OLED.Rect(96,25,107,31, SOLID);*/
  OLED.Refresh();
  OLED.startScrollLeft(0,0x0f);

  while(1){}
}