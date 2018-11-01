// TCS230 sensor calibration and color readings
//
// Input and output using the Serial console.
//
#include <MD_TCS230.h>
/*  MD_TCS230 Color Sensor Arduino Library  1.2
 *  Unfortunately there will be no chance to integrate these libraries as all of them relies on H/W counters that the ESP8266 doesn't have.
 *  https://majicdesigns.github.io/MD_TCS230/page_calibration.html */
#include <FreqCount.h>
/* FreqCount Library: FreqCount measures the frequency of a signal by counting the number of pulses during a fixed time. See FreqCount vs FreqMeasure below to choose the best library.
https://www.pjrc.com/teensy/td_libs_FreqCount.html */

#define   BLACK_CAL 0
#define   WHITE_CAL 1
#define   READ_VAL  2

// Pin definitions
#define   S0_OUT    4   // GRAY
#define   S1_OUT    5   // WHITE
#define   S2_OUT    6   // ORANGE
#define   S3_OUT    7   // BROWN
#define   OE_OUT    8   // PURPLE


/*
#define   S2_OUT  12
#define   S3_OUT  13
#define   OE_OUT   8    // LOW = ENABLED 
*/

/*
NAME  | NUM | I/O | DESCRIPTION
GND     4     I     Power supply ground. All voltages are referenced to GND.
OE      3     I     Enable for fo (active low).
OUT     6     O     Output frequency (fo).
S0, S1  1, 2  I     Output frequency scaling selection inputs.
S2, S3  7, 8  I     Photodiode type selection inputs.
VDD     5           Supply voltage
*/
 
MD_TCS230 CS(S2_OUT, S3_OUT, OE_OUT);

void setup() {
pinMode(S0_OUT, OUTPUT);
pinMode(S1_OUT, OUTPUT);
pinMode(S2_OUT, OUTPUT);
pinMode(S3_OUT, OUTPUT);
pinMode(OE_OUT, INPUT);
// Setting frequency-scaling to 20%
digitalWrite(S0_OUT,HIGH);
digitalWrite(S1_OUT,LOW);
Serial.begin(57600);
Serial.print(F("\n [TCS230 Calibrator Example - 01|11|2018]"));
Serial.print(F("\n [Setting frequency-scaling to 20%]"));
CS.begin();}

char getChar()
// blocking wait for an input character from the input stream
{
  while (Serial.available() == 0)
    ;
  return(toupper(Serial.read()));
}

void clearInput()
// clear all characters from the serial input
{
  while (Serial.read() != -1)
    ;
}

uint8_t fsmReadValue(uint8_t state, uint8_t valType, uint8_t maxReads)
// Finite State Machine for reading a value from the sensor
// Current FSM state is passed in and returned
// Type of value being read is passed in
{
  static  uint8_t selChannel;
  static  uint8_t readCount;
  static  sensorData  sd;

  switch(state)
  {
  case 0: // Prompt for the user to start
    Serial.print(F("\n\nReading value for "));
    switch(valType)
    {
    case BLACK_CAL: Serial.print(F("BLACK calibration")); break;
    case WHITE_CAL: Serial.print(F("WHITE calibration")); break;
    case READ_VAL:  Serial.print(F("DATA"));        break;
    default:    Serial.print(F("??"));          break;
    }
    
    Serial.print(F("\nPress any key to start ..."));
    state++;
    break;

  case 1: // Wait for user input
    getChar();
    clearInput();
    state++;
    break;

  case 2: // start the reading process
    CS.read();
    state++;
    break;

  case 3: // wait for a read to complete
    if (CS.available()) 
    {
      sensorData  sd;
      colorData rgb;

      switch(valType)
      {
      case BLACK_CAL: 
        CS.getRaw(&sd); 
        CS.setDarkCal(&sd);   
        break;

      case WHITE_CAL: 
        CS.getRaw(&sd); 
        CS.setWhiteCal(&sd);  
        break;

      case READ_VAL:  
        CS.getRGB(&rgb);
        Serial.print(F("\nRGB is ["));
        Serial.print(rgb.value[TCS230_RGB_R]);
        Serial.print(F(","));
        Serial.print(rgb.value[TCS230_RGB_G]);
        Serial.print(F(","));
        Serial.print(rgb.value[TCS230_RGB_B]);
        Serial.print(F("]"));
        break;
      }
      state++;
    }
    break;

  default:  // reset fsm
    state = 0;
    break;
  }

  return(state);
}


void loop() 
{
  static uint8_t  runState = 0;   
  static uint8_t  readState = 0;

  switch(runState)
  {
  case 0: // calibrate black
    readState = fsmReadValue(readState, BLACK_CAL, 2);
    if (readState == 0) runState++;
    break;

  case 1: // calibrate white
    readState = fsmReadValue(readState, WHITE_CAL, 2);
    if (readState == 0) runState++;
    break;

  case 2: // read color
    readState = fsmReadValue(readState, READ_VAL, 1);
    break;

  default:
    runState = 0; // start again if we get here as something is wrong
  }
}


