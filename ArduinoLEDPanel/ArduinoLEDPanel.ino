#include <Wire.h>
#include "AdafruitLEDPanel.h"

#define I2C_SLAVE_ADDRESS   0x08
#define IMG_BUFF_LEN        768 //(32 * 64 * 3) / number of slices


#define LINES_ARRAY_SIZE    1
#define CMD_END_CHAR        '~'

#define DEBUG
//#define TEST_PARSER
#define RECTANGULAR_TEXT
//#define HORIZONTAL_TEXT
#define ERASE_SCREEN        true

#define CLK                 11  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT                 10
#define OE                  9
#define A                   A0
#define B                   A1
#define C                   A2
#define D                   A3

AdafruitLEDPanel *panel;
char imgBuffer[IMG_BUFF_LEN];
volatile int imgLen = 0;
volatile int loopDelay = 1000;

char temp;

/**
 * This method is called once during start up to initialize the device.
 */
void setup()
{
  Serial.begin(57600);
  //Serial.setTimeout(5000);
  memset(imgBuffer, 0x00, sizeof(char)*IMG_BUFF_LEN);
  //Wire.begin(I2C_SLAVE_ADDRESS);
  //Wire.onReceive(receiveEvent);
  panel = new AdafruitLEDPanel(A, B, C, D, CLK, LAT, OE, true, PANEL_WIDTH, LINES_ARRAY_SIZE);
#ifdef TEST_PARSER
  processCommand("setTextLine 0 0 0 800 0 1 -1 Testing command parser.");
#endif
#ifdef RECTANGULAR_TEXT
  panel->setTextLine(0, "Titan Robotics", 0, 1, panel->color(0, 200, 0), 1, 1, -1);
#else
  #ifdef HORIZONTAL_TEXT
  panel->setTextLine(0, "Font Size 1: Left", 0, 0, panel->color(50, 0, 0), 0, 1, -1);
  panel->setTextLine(1, "Font Size 1: Right", 0, 8, panel->color(0, 50, 0), 0, 1, 1);
  panel->setTextLine(2, "Font Size 2: Invert", 0, 0, panel->color(0, 0, 50), 2, 2, 1);
  #endif
#endif
}

/**
 * This method is called repeatedly to run the text display task.
 */
void loop()
{
  if (panel->doneRequesting())
  {
    panel->displayTextTask(ERASE_SCREEN);
    if (loopDelay > 0) delay(loopDelay);
  }
  else
  {
    int readed;
    
    Serial.println("send request");
  
    readed = Serial.readBytes(imgBuffer, 768);
    if (readed > 0)
    {
      Serial.println(readed);
      
      panel->setImageSlice(imgBuffer);
      if (panel->readyToContinue())
      {
        panel->displayBuffer();
      }
      //if (loopDelay > 0) delay(loopDelay);
    }
  }
}
