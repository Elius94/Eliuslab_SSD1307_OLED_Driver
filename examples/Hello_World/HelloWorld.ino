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
  //OLED.test(); //Load a test image
}

void loop() {
  // put your main code here, to run repeatedly:
  OLED.Print("Ciao");
  OLED.full_on();
  for(;;){}
}
