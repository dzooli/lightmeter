/* 
Connections

LCD5110_Graph
      SCK  - Pin 8
      MOSI - Pin 9
      DC   - Pin 10
      RST  - Pin 11
      CS   - Pin 12

BH1750FVI LightSensor:
  Main address  0x23 
  secondary address 0x5C 
  connect this sensor as following :
  VCC >>> 3.3V
  SDA >>> A4 
  SCL >>> A5
  addr >> A3
  Gnd >>>Gnd
*/

#include <string.h>

// definitions for the Nokia 5110 LCD
#include <LCD5110_Graph.h>
#include <Wire.h>
#include <BH1750FVI.h>

#include "mathutils.h"

//#define DEBUG 1

// Fonts
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];

volatile LCD5110 myGLCD(10,11,5,6,12);
BH1750FVI myLightSensor;
#define LCD_BACKLIGHT_PIN 9
#define LCD_BACKLIGHT_VAL 100

// yPOS definitions
#define YPOS_MODE 0
#define YPOS_ISO  10
#define YPOS_TIME 10
#define YPOS_APRT 10
#define YPOS_SVAL 10    // "Settable" value (T|A) ypos
#define YPOS_CVAL 24    // Calculated result based on the mode and the measured lux
#define YPOS_EVC  40    // Calculated EV value ypos
#define YPOS_EVM  40    // Measured EV value ypos
#define YPOS_LUX  40    // Measured Lux value ypos

// Precision to floats
#define DECIMALS 3

// Feedback definitions
#define STAT_LED 13     // status led pin

#define ISO_MAX   25600
#define ISO_MIN   50

# define K 64       	  // Calibration constant for this sample routine
//# define K 78.125   	// Calibration constant based on the Sunny16 rule and "Sunny"=20000Lx
// Array of ISO values
static uint16_t ISOs[] = {50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200};               //Array of sensitivity values
static uint8_t EVs[] ={-1,   0,   1,   2,   3,    4,    5,    6,     7,     8,     9};               //Array of EV compensation based on the ISO

// Arrays of Aperture values
#define MAX_AIDX 13
static float A[] = {1, 1.4, 2, 2.8, 4, 5.6, 8, 11, 16, 22, 32, 45, 64, 90};                          //Array of aperture values
static char* Astr[]={"1", "1.4", "2", "2.8", "4", "5.6", "8", "11", "16", "22", "32", "45", "64", "90"};


// Arrays of Time values
#define MAX_TIDX 21
//                   4m   2m  1m                      1/2   1/4    1/8        1/15       1/30    1/60       1/125  1/250  1/500
static float T[] = {240, 120, 60, 30, 15, 8, 4, 2, 1, 0.5, 0.25, 0.125, 0.066666667, 0.03333333, 0.0166666, 0.008, 0.004, 0.002,
//                  1/1000  1/2000  1/4000   1/8000
                    0.001,  0.0005, 0.00025, 0.000125
};
static char* Tstr[] = {"4m", "2m", "1m", "30s", "15s", "8s", "4s", "2s", "1s", "1/2", "1/4", "1/8", "1/15", "1/30", "1/60", "1/125", "1/250", "1/500",
                       "1/1000", "1/2000", "1/4000", "1/8000"
                       };

// static string and enumeration definitons for computing mode
char tmode[] = {'T', '\0'};
char amode[] = {'A', '\0'};
enum MODE { MODE_T, MODE_A};
uint8_t mode = MODE_T;

static uint16_t cISO = 50;
static uint8_t  cTimeIdx =  14;
static uint8_t  cApIdx = 0;
static uint16_t cLux = 0;
static double   cEV = 0.0;
static double   cVal = 0.0;

// button count and index definitions
#define BT_COUNT 3
enum BT_NAMES { BT_MODE, BT_ISO, BT_VAL};
// button pin definitions
byte buttons[3] = {4, 3, 2};
bool button_state[3] = {false, false, false};

void init_buttons()
{
  // setup button inputs and button_state array
  for (int i=0; i<BT_COUNT; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    digitalWrite(buttons[i], HIGH);
    button_state[i] = false;
  }
}

void clear_buttons()
{
  // set the state of buttons to false
  for(int i=0; i<BT_COUNT; i++){
    button_state[i] = false;
  }
}

void scan_buttons()
{
  for (int i=0; i<BT_COUNT; i++){
    if (!digitalRead(buttons[i])) {
       delay(60);
      if (!digitalRead(buttons[i])) {
         digitalWrite(STAT_LED, HIGH);
         delay(60);
         digitalWrite(STAT_LED, LOW);
         button_state[i] = true;
      } else button_state[i] = false;
    }
  }
}

void handle_buttons()
{
  if (button_state[BT_MODE]) {
    switch ( mode ) {
        case MODE_T:
          mode = MODE_A;
          break;
        case MODE_A:
          mode = MODE_T;
          break;
    }
  }
  if (button_state[BT_VAL]) {
    switch ( mode ) {
      case MODE_T: {
        cTimeIdx++;
        if (cTimeIdx>MAX_TIDX) cTimeIdx=0;
        break;
      }
      case MODE_A: {
        cApIdx++;
        if (cApIdx>MAX_AIDX) cApIdx=0;
        break;
      }
    }
  }
  if (button_state[BT_ISO]) {
      cISO = cISO<<1;
      if (cISO>ISO_MAX) cISO=ISO_MIN;
  }
}

void setup()
{
  // setup LDC
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  delay(100);
  // setup backlight (PWM)
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  analogWrite(LCD_BACKLIGHT_PIN, LCD_BACKLIGHT_VAL);
  // setup buttons
  init_buttons();
  clear_buttons();
  myLightSensor.begin();
  // LightSensor.SetAddress(Device_Address_H);//Address 0x5C
  // To adjust the slave on other address , uncomment this line
  myLightSensor.SetAddress(Device_Address_L); //Address 0x23
  myLightSensor.SetMode(Continuous_L_resolution_Mode);
  delay(60);
#ifdef DEBUG
  // Just for debugging
  Serial.begin(9600);
#endif  
  //-----------------------------------------------  
}

void calculate_values() {
  cEV=luxToEV(cLux);
  switch (mode) {
    case MODE_T: {
      cVal = calcA((float)cLux, (float)T[cTimeIdx], (float)(K), (float)cISO);
      break;
    }
    case MODE_A: {
      cVal = calcT((float)cLux, (float)A[cApIdx], (float)(K), (float)cISO);
      break;
    }
    default: {
      break;
    }
  }
}

void display_data()
{
  // Clear prev values
  myGLCD.print("     ", LEFT, YPOS_ISO);
  myGLCD.print("          ", LEFT, YPOS_LUX);
  myGLCD.print("     ", RIGHT, YPOS_LUX);
  myGLCD.print("          ", RIGHT, YPOS_SVAL);
  myGLCD.print("          ", CENTER, YPOS_CVAL);

  myGLCD.print("ISO", LEFT, 0);
  
  // Display the current ISO value
  myGLCD.printNumI(cISO, LEFT, YPOS_ISO);
  // Display the current measured Lux value
  myGLCD.printNumI(cLux, RIGHT, YPOS_LUX);
  // Display the current working mode
  if (mode == MODE_T) {
    myGLCD.print((char*)&tmode, RIGHT, YPOS_MODE);
    myGLCD.print(Tstr[cTimeIdx], RIGHT, YPOS_SVAL);
    // TODO: do calculations in Time preselect mode
  } else {
    myGLCD.print((char*)&amode, RIGHT, YPOS_MODE);
    myGLCD.print(Astr[cApIdx], RIGHT, YPOS_SVAL);
    // TODO: do calculations in Aperture preselect mode
  }
  // Display the calculated Value
  myGLCD.printNumF(cVal, DECIMALS, CENTER, YPOS_CVAL);
  myGLCD.printNumF(cEV, DECIMALS, LEFT, YPOS_LUX);
  myGLCD.update();
  delay(40);
}

void loop()
{
  scan_buttons();
  handle_buttons();
  clear_buttons();  
  cLux = myLightSensor.GetLightIntensity();
  // Adjust LCD backlight
  analogWrite(LCD_BACKLIGHT_PIN, map(cLux>>2, 0, 16381, 255, 0));
  // Calc and display
  calculate_values();
  delay(50);
  display_data();
#ifdef DEBUG
// Just for debugging
  Serial.print("Light: ");
  Serial.print(cLux);
  Serial.println(" lux");
#endif
}
