#include <Eliuslab_SSD1307_OLED_Driver.h>
#include <Wire.h>
#include <EEPROM.h>

// I2C ADDRESS HERE
#define I2C_ADDRESS 3

#define CS_PIN 10
#define DC_PIN 8
#define RST_PIN 9
#define SERVICE_LED 14
#define ENABLE_PIN PIN_PD2

#define OLED_WIDTH 128
#define OLED_HEIGHT 32

#define MAX_NAMES 10

#define EEPROM_SETTINGS_TWOROWS           500
#define EEPROM_SETTINGS_CHANGE_INTERVAL   501
#define EEPROM_SETTINGS_ACTIVE_NAMES      502
#define EEPROM_SETTINGS_SCROLLING         503
#define EEPROM_SETTINGS_SHOW_BITMAP       504
#define EEPROM_SETTINGS_ROLL_MODE         505
#define EEPROM_BITMAP_ADDRESS             510

// I2C Commands
#define I2C_MASTER_COMMAND_SET_NAME               0x10
#define I2C_MASTER_COMMAND_SET_TWOROWS            0x11
#define I2C_MASTER_COMMAND_SET_CHANGE_INTERVAL    0x12
#define I2C_MASTER_COMMAND_SET_ACTIVE_NAMES       0x13
#define I2C_MASTER_COMMAND_SET_SCROLLING          0x14
#define I2C_MASTER_COMMAND_SET_UPDATING           0x15
#define I2C_MASTER_COMMAND_RUN_TEST               0x16
#define I2C_MASTER_COMMAND_SET_SHOW_BITMAP        0x17
#define I2C_MASTER_COMMAND_SET_BITMAP             0x18
#define I2C_MASTER_COMMAND_SET_ROLL_MODE          0x19 // Concat all active names into one big String and virtually start scroll
#define I2C_MASTER_COMMAND_HARD_RESET             0xDD

bool default_twoRows = false;
int default_changeInterval = 1000;
byte default_activeNames = MAX_NAMES;
bool default_scrolling = false;
bool default_showBitmap = false;
bool default_rollMode = false;

uint16_t nameEEPROMAddress[MAX_NAMES] = { 0, 50, 100, 150, 200, 250, 300, 350, 400, 450 };
String default_names[MAX_NAMES] = {"Nome 1", "Nome 2", "Nome 3", "Nome 4", "Nome 5", "Nome 6", "Nome 7", "Nome 8", "Nome 9", "Nome 10"};
String names[MAX_NAMES];

bool twoRows;
int changeInterval;
byte activeNames;
bool scrolling;
bool showBitmap;
bool rollMode;
bool updating = false;
bool updating_ctrl = false;
bool enabled = true;

enum settings {
  SETTINGS_TWOROWS,
  SETTINGS_CHANGE_INTERVAL,
  SETTINGS_ACTIVE_NAMES,
  SETTINGS_SCROLLING,
  SETTINGS_SHOW_BITMAP,
  SETTINGS_ROLL_MODE
};

Eliuslab_OLED OLED(OLED_WIDTH, OLED_HEIGHT, CS_PIN, DC_PIN, RST_PIN);

void writeStringToEEPROM(int addrOffset, const String &strToWrite) {
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset) {
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; // the character may appear in a weird way, you should read: 'only one backslash and 0'
  return String(data);
}

void resetNames() {
  for (int ind = 0; ind < MAX_NAMES; ind++) {
    //Serial.printf("%i\n", nameEEPROMAddress[ind]);
    writeStringToEEPROM(nameEEPROMAddress[ind], default_names[ind]);
  }
}

void resetSettings() {
  EEPROM.write(EEPROM_SETTINGS_TWOROWS, default_twoRows);
  EEPROM.write(EEPROM_SETTINGS_CHANGE_INTERVAL, (byte)(default_changeInterval / 100));
  EEPROM.write(EEPROM_SETTINGS_ACTIVE_NAMES, default_activeNames);
  EEPROM.write(EEPROM_SETTINGS_SCROLLING, default_scrolling);
  EEPROM.write(EEPROM_SETTINGS_SHOW_BITMAP, default_showBitmap);
  EEPROM.write(EEPROM_SETTINGS_ROLL_MODE, default_rollMode);
}

void resetBitmap() {
  OLED.BurnTestBitmapToEEPROM(EEPROM_BITMAP_ADDRESS);
}

void initNames() {
  for (int x = 0; x < MAX_NAMES; ++x) {
    //names[x] = default_names[x];
    names[x] = readStringFromEEPROM(nameEEPROMAddress[x]);
  }
}

void initSettings() {
  twoRows = EEPROM.read(EEPROM_SETTINGS_TWOROWS);
  changeInterval = EEPROM.read(EEPROM_SETTINGS_CHANGE_INTERVAL) * 100;
  activeNames = EEPROM.read(EEPROM_SETTINGS_ACTIVE_NAMES);
  scrolling = EEPROM.read(EEPROM_SETTINGS_SCROLLING);
  showBitmap = EEPROM.read(EEPROM_SETTINGS_SHOW_BITMAP);
  rollMode = EEPROM.read(EEPROM_SETTINGS_ROLL_MODE);
}

void initMemory(bool _reset = false) {
  if (_reset) {
    resetNames();
    resetSettings();
    resetBitmap();
  }
  initNames();
  initSettings();
}

bool changeName(byte id, String value, bool save = false) {
  if (value.length() < 50) {
    names[id] = (String)value;
    if (save) {
      writeStringToEEPROM(nameEEPROMAddress[id], (String)value);
    }
    return true;
  }
  return false;
}

bool changeSetting(settings setting, int value, bool save = false) {
  switch (setting) {
    case SETTINGS_TWOROWS: {
        twoRows = (bool)value;
        if (save) {
          EEPROM.write(EEPROM_SETTINGS_TWOROWS, value);
        }
        return true;
      }
      break;
    case SETTINGS_CHANGE_INTERVAL: {
        changeInterval = value;
        if (save) {
          EEPROM.write(EEPROM_SETTINGS_CHANGE_INTERVAL, (byte)(value / 100));
        }
        return true;
      }
      break;
    case SETTINGS_ACTIVE_NAMES: {
        activeNames = value;
        if (save) {
          EEPROM.write(EEPROM_SETTINGS_ACTIVE_NAMES, (byte)value);
        }
        return true;
      }
      break;
    case SETTINGS_SCROLLING: {
        scrolling = (bool)value;
        if (save) {
          EEPROM.write(EEPROM_SETTINGS_SCROLLING, value);
        }
        return true;
      }
      break;
    case SETTINGS_SHOW_BITMAP: {
        showBitmap = (bool)value;
        if (save) {
          EEPROM.write(EEPROM_SETTINGS_SHOW_BITMAP, value);
        }
        return true;
      }
      break;
    case SETTINGS_ROLL_MODE: {
        rollMode = (bool)value;
        if (save) {
          EEPROM.write(EEPROM_SETTINGS_ROLL_MODE, value);
        }
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

void blinkServiceLed() {
  digitalWrite(SERVICE_LED, HIGH);
  digitalWrite(SERVICE_LED, LOW);
}

void setup() {
  delay(1000);
  //Serial.begin(9600);
  //delay(500);
  pinMode(SERVICE_LED, OUTPUT);
  pinMode(ENABLE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENABLE_PIN), handleEnablePin, CHANGE);
  digitalWrite(SERVICE_LED, HIGH);
  OLED.begin(); //Init the OLED
  delay(500);
  initMemory(false); // true for init EEPROM
  Wire.begin(I2C_ADDRESS);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  delay(500);
  digitalWrite(SERVICE_LED, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  //OLED.test();
  //OLED.Bitmap(128,4,logo_bmp);
  /*OLED.Rect(21,25,32,31, SOLID);
    OLED.Rect(96,25,107,31, SOLID);*/

  OLED.Refresh();
  while (1) {
    if (updating) {
      if (updating_ctrl) {
        updating_ctrl = false;
        printUpdateScreen();
        blinkServiceLed();
      }
    } else {
      if (enabled) {
        if (rollMode) {
          printRollMode(names, showBitmap, changeInterval);
        } else {
          horizontalSequence(twoRows, names, showBitmap, changeInterval);
        }
      } else {
        disableDisplay();
      }
    }
  }
}

void disableDisplay() {
  OLED.ClearBuffer();
  OLED.full_off();
}

void handleEnablePin() {
  if (digitalRead(ENABLE_PIN) == HIGH) {
    enabled = true;
  } else {
    enabled = false;
  }
}

void printUpdateScreen() {
  OLED.ClearBuffer();
  OLED.SetFont(_LargeProp_25pt);
  OLED.Cursor(0, 2);
  OLED.Print("UPDATE ");
  OLED.Refresh();
  OLED.startScrollLeft(0, 0x0f);
}

void printRollMode(String names[], bool drawBitmap, int waitFor) {
  if (drawBitmap) {
    if (updating || enabled == false) {
      return;
    }
    OLED.ClearBuffer();
    OLED.PrintBitmapFromEEPROM(EEPROM_BITMAP_ADDRESS);
    if (scrolling) {
      OLED.startScrollLeft(0, 0x0f);
    } else {
      OLED.stopScroll();
    }
    delay(waitFor * 10);
  }
  String text = "";
  String spacer = "  ";
  for (int i = 0; i < activeNames; ++i) {
    text.concat(names[i] + spacer);
  }

  byte maxChars = 10;
  OLED.SetFont(_LargeProp_25pt);

  for (int from = 0; from < text.length() - (maxChars - 1); ++from) {
    if (updating || enabled == false) {
      return;
    }
    String sliced = text.substring(from, from + maxChars);
    int str_len = sliced.length() + 1;
    char slice[str_len];
    sliced.toCharArray(slice, str_len);
    OLED.ClearBuffer();
    OLED.Cursor(0, 4);
    OLED.Print(slice);
    OLED.Refresh();
    delay(waitFor);
  }
  delay(waitFor * 10);
}

void horizontalSequence(bool twoRows, String names[], bool drawBitmap, int waitFor) {
  if (drawBitmap) {
    if (updating || enabled == false) {
      return;
    }
    OLED.ClearBuffer();
    OLED.PrintBitmapFromEEPROM(EEPROM_BITMAP_ADDRESS);
    if (scrolling) {
      OLED.startScrollLeft(0, 0x0f);
    } else {
      OLED.stopScroll();
    }
    delay(waitFor);
  }
  if (twoRows) {
    OLED.SetFont(_MedProp_11pt);
    for (int i = 0; i < activeNames; i = i + 2) {
      if (updating || enabled == false) {
        return;
      }
      OLED.ClearBuffer();
      int str_len_1 = names[i].length() + 1;
      char _name_1[str_len_1];
      names[i].toCharArray(_name_1, str_len_1);
      int str_len_2 = names[i + 1].length() + 1;
      char _name_2[str_len_2];
      names[i + 1].toCharArray(_name_2, str_len_2);
      OLED.Cursor(0, 2);
      OLED.Print(_name_1);
      OLED.Cursor(0, 16);
      OLED.Print(_name_2);
      OLED.Refresh();
      if (scrolling) {
        OLED.startScrollLeft(0, 0x0f);
      } else {
        OLED.stopScroll();
      }
      delay(waitFor);
    }
  } else {
    OLED.SetFont(_LargeProp_25pt);
    for (int i = 0; i < activeNames; ++i) {
      if (updating || enabled == false) {
        return;
      }
      int str_len = names[i].length() + 1;
      char _name[str_len];
      names[i].toCharArray(_name, str_len);
      OLED.ClearBuffer();
      OLED.Cursor(0, 4);
      OLED.Print(_name);
      OLED.Refresh();
      if (scrolling) {
        OLED.startScrollLeft(0, 0x0f);
      } else {
        OLED.stopScroll();
      }
      delay(waitFor);
    }
  }
}

void oledPrintF(String intput) {
  int str_len = intput.length() + 1;
  char in[str_len];
  intput.toCharArray(in, str_len);
  OLED.ClearBuffer();
  OLED.SetFont(_MedProp_11pt);
  OLED.Cursor(0, 10);
  OLED.Print(in);
  OLED.Refresh();
  OLED.startScrollLeft(0, 0x0f);
  delay(1000);
}

void receiveEvent(int howMany)
{
  //oledPrintF("Incoming: " + (String)howMany);
  blinkServiceLed();
  bool saveToEEPROM = true; // set true to store value to EEPROM
  if (Wire.available()) { // loop through all but the last
    byte command = Wire.read(); // receive byte as a character
    switch (command) {
      case I2C_MASTER_COMMAND_SET_NAME: {
          byte number;
          String value = "";
          if (Wire.available()) {
            blinkServiceLed();
            number = Wire.read();
            while (Wire.available()) {
              blinkServiceLed();
              value.concat((char)Wire.read());
            }
            changeName(number, value, saveToEEPROM);
          }
        }
        break;
      case I2C_MASTER_COMMAND_SET_TWOROWS: {
          if (Wire.available()) {
            blinkServiceLed();
            bool value = (bool)Wire.read();
            changeSetting(SETTINGS_TWOROWS, value, saveToEEPROM);
          }
        }
        break;
      case I2C_MASTER_COMMAND_SET_CHANGE_INTERVAL: {
          if (Wire.available()) {
            blinkServiceLed();
            uint16_t value = Wire.read() * 100;
            //oledPrintF("Value: " + (String)value);
            Serial.printf("Change interval: %d", value);
            changeSetting(SETTINGS_CHANGE_INTERVAL, value, saveToEEPROM);
          }
        }
        break;
      case I2C_MASTER_COMMAND_SET_ACTIVE_NAMES: {
          if (Wire.available()) {
            blinkServiceLed();
            byte value = Wire.read();
            if (value <= 10) {
              changeSetting(SETTINGS_ACTIVE_NAMES, value, saveToEEPROM);
            }
          }
        }
        break;
      case I2C_MASTER_COMMAND_SET_SCROLLING: {
          if (Wire.available()) {
            blinkServiceLed();
            bool value = (bool)Wire.read();
            changeSetting(SETTINGS_SCROLLING, value, saveToEEPROM);
          }
        }
        break;
      case I2C_MASTER_COMMAND_SET_UPDATING: {
          if (Wire.available()) {
            blinkServiceLed();
            bool value = (bool)Wire.read();
            if (value) {
              printUpdateScreen();
            } else {
              OLED.ClearBuffer();
            }
            updating_ctrl = value;
            updating = value;
          }
        }
        break;
      case I2C_MASTER_COMMAND_RUN_TEST: {
          OLED.test();
        }
        break;
      case I2C_MASTER_COMMAND_SET_SHOW_BITMAP: {
          if (Wire.available()) {
            blinkServiceLed();
            bool value = (bool)Wire.read();
            changeSetting(SETTINGS_SHOW_BITMAP, value, saveToEEPROM);
          }
        }
        break;
      case I2C_MASTER_COMMAND_SET_BITMAP: {
          if (Wire.available() == 17) {
            blinkServiceLed();
            uint8_t page = (byte)Wire.read();
            uint8_t counter = 0;
            while (Wire.available()) {
              blinkServiceLed();
              writeBitmapToEEPROMSliced(page, counter, (byte)Wire.read());
              counter++;
            }
          }
        }
        break;
      case I2C_MASTER_COMMAND_SET_ROLL_MODE: {
          if (Wire.available()) {
            blinkServiceLed();
            bool value = (bool)Wire.read();
            changeSetting(SETTINGS_ROLL_MODE, value, saveToEEPROM);
          }
        }
        break;
      case I2C_MASTER_COMMAND_HARD_RESET: {
          if (Wire.available()) {
            blinkServiceLed();
            bool check = (bool)Wire.read();
            if (check) {
              initMemory(true);
            }
          }
        }
        break;
      default:
        break;
    }
  }
}

void writeBitmapToEEPROMSliced(byte page, byte counter, byte value) {
  EEPROM.update(EEPROM_BITMAP_ADDRESS + counter + (16 * page), value);
}
