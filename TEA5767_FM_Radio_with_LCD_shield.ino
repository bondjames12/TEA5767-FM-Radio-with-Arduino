
/*--------------------------------------------------------------------------------------

TEA5767 FM Radio
By Chris Rouse Jan 2016

Using Arduino Mega

Up to 10 preset stations can be selected although this can easily be increased
SELECT switches to your favourite station

Tuning is in 0.01mHz intervals to allow for fine tuning of presets

There are 4 pins to connect, SDA (pin 20), SCL (pin 21), 5volts and Gnd (power available on end socket). Plug the LCD Shield
onto the Arduino Mega

--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------
* Setup Radio
--------------------------------------------------------------------------------------*/
#include <Wire.h>
#include <TEA5767N.h>
// Init radio object
TEA5767N radio = TEA5767N();
/*--------------------------------------------------------------------------------------
* Include the standard LiquidCrystal library
--------------------------------------------------------------------------------------*/
#include <LiquidCrystal.h>
/* DIO pin definitions */
#define LCD_DATA4 4         /* LCD data DIO pin 4 */
#define LCD_DATA5 5         /* LCD data DIO pin 5 */
#define LCD_DATA6 6         /* LCD data DIO pin 6 */
#define LCD_DATA7 7         /* LCD data DIO pin 7 */
#define LCD_RESET 8         /* LCD Reset DIO pin */
#define LCD_ENABLE 9        /* LCD Enable DIO pin */
#define LCD_BACKLIGHT 10    /* LCD backlight DIO pin */
/* Initialise the LiquidCrystal library with the correct DIO pins */
LiquidCrystal lcd(LCD_RESET, LCD_ENABLE, LCD_DATA4, LCD_DATA5, LCD_DATA6, LCD_DATA7);
/*--------------------------------------------------------------------------------------
Define pins used and button return values
--------------------------------------------------------------------------------------*/
#define BUTTON_ADC_PIN A0 // A0 is the button ADC input
#define LCD_BACKLIGHT_PIN 3 // D3 controls LCD backlight
// ADC readings expected for the 5 buttons on the ADC input
// these values may need altering to suit your board
#define RIGHT_10BIT_ADC 0 // right button
#define UP_10BIT_ADC 101 // up button
#define DOWN_10BIT_ADC 259 // down button
#define LEFT_10BIT_ADC 410 // left button
#define SELECT_10BIT_ADC 640 // right button
#define BUTTONHYSTERESIS 10 // hysteresis for valid button sensing window
//return values for ReadButtons()
#define BUTTON_NONE 0
#define BUTTON_RIGHT 1
#define BUTTON_UP 2
#define BUTTON_DOWN 3
#define BUTTON_LEFT 4
#define BUTTON_SELECT 5
/*--------------------------------------------------------------------------------------
Variables
--------------------------------------------------------------------------------------*/
byte buttonJustPressed = false; //this will be true after a ReadButtons() call if triggered
byte buttonJustReleased = false; //this will be true after a ReadButtons() call if triggered
byte buttonWas = BUTTON_NONE; //used by ReadButtons() for detection of button events
double frequency = 88.00; // frequency in mHz
double topFrequency = 110.00;
double bottomFrequency = 88.00;
double intervalFreq = 0.1; // could be changed to 1 for coarse tuning
int maxPresets; // maximum number of presets used
double stationsMHZ[10]; // allow for 10 preset stations
String stations[10]; // station names
int presetCounter = 0; //pointer to preset stations
int favourite; // pressing SELECT will select this one
int sl; // signal level 0 - 10
/*--------------------------------------------------------------------------------------*/

void setup() {
  Serial.begin(9600); // start the Serial to allow results to be shown
  Wire.begin();
  // Set the correct display size (16 character, 2 line display)
  lcd.begin(16, 2);
  //button adc input
  pinMode( BUTTON_ADC_PIN, INPUT ); //ensure A0 is an input
  digitalWrite( BUTTON_ADC_PIN, LOW ); //ensure pullup is off on A0
  lcd.setCursor(0, 0);
  lcd.print("FM");
  radio.selectFrequency(bottomFrequency); // set radio initially to lowest frequency
  lcd.setCursor(0, 1);
  lcd.print(String(frequency) + "mHz   ");
  // setup preset list of stations
  stationsMHZ[1] = 88.58;
  stations[1] = "BBC Radio 2";
  stationsMHZ[2] = 96.10;
  stations[2] = "Radio Solent";
  stationsMHZ[3] = 97.50;
  stations[3] = "Heart Solent";
  stationsMHZ[4] = 98.20;
  stations[4] = "BBC Radio 1";
  stationsMHZ[5] = 103.20;
  stations[5] = "Capital FM";
  stationsMHZ[6] = 102.00;
  stations[6] = "Isle of Wight Radio";
  stationsMHZ[7] = 88.00;
  stations[7] = "----";
  stationsMHZ[8] = 88.00;
  stations[8] = "----";
  stationsMHZ[9] = 88.00;
  stations[9] = "----";
  stationsMHZ[10] = 88.00;
  stations[10] = "----";
  //
  maxPresets = 6; // number of stations in list, change this as required
  favourite = 6; // select your own favourite
}
/*--------------------------------------------------------------------------------------*/
void loop() {

  //From application_note_tea5767-8.pdf, limit the amount of noise energy.
  radio.setSoftMuteOn();
  //From application_note_tea5767-8.pdf, cut high frequencies from the audio signal.
  radio.setHighCutControlOn();
  byte button;
  //get the latest button pressed, also the buttonJustPressed, buttonJustReleased flags
  button = ReadButtons();
  //show text label for the button pressed

  switch ( button )
  {
    case BUTTON_NONE:
      {
        break;
      }
    case BUTTON_RIGHT:
      {
        // next preset station
        presetCounter = presetCounter + 1;
        if (presetCounter > maxPresets) {
          presetCounter = 1;
        }
        selectStation();
        break;
      }
    case BUTTON_UP:
      {
        frequency = frequency + intervalFreq;
        if (frequency > topFrequency) {
          frequency = bottomFrequency; // reset frequency
        }
        selectStation2();
        break;
      }
    case BUTTON_DOWN:
      {
        frequency = frequency - intervalFreq;
        if (frequency < bottomFrequency) {
          frequency = topFrequency; // reset frequency
        }
        selectStation2();
        break;
      }
    case BUTTON_LEFT:
      {
        // last preset station
        presetCounter = presetCounter - 1;
        if (presetCounter  < 1) {
          presetCounter = maxPresets;
        }
        frequency = stationsMHZ[presetCounter];
        selectStation();
        break;
      }
    case BUTTON_SELECT:
      {
        if (favourite > 0 and favourite < maxPresets + 1) {
          presetCounter = favourite;
          frequency = stationsMHZ[presetCounter];
          selectStation();
        }
        break;
      }
    default:
      {
        break;
      }
  }

  //clear the buttonJustPressed or buttonJustReleased flags, they've already done their job now.
  if ( buttonJustPressed ) {
    buttonJustPressed = false;
  }
  if ( buttonJustReleased ) {
    buttonJustReleased = false;
  }
  printStereoStatus();
  sl = radio.getSignalLevel();
  sl =  sl * 10;
  lcd.setCursor(3, 0);
  lcd.print("SS=" + String(sl) + "% ");
} // End main loop


/*--------------------------------------------------------------------------------------
  ReadButtons()
  Detect the button pressed and return the value
  Uses global values buttonWas, buttonJustPressed, buttonJustReleased.
--------------------------------------------------------------------------------------*/
byte ReadButtons()
{
  unsigned int buttonVoltage;
  byte button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn
  //read the button ADC pin voltage
  buttonVoltage = analogRead( BUTTON_ADC_PIN );
  //Serial.println(buttonVoltage); // un comment to print voltage
  //sense if the voltage falls within valid voltage windows
  if ( buttonVoltage < ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_RIGHT;
  }
  else if (   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
              && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_UP;
  }
  else if (   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
              && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_DOWN;
  }
  else if (   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
              && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_LEFT;
  }
  else if (   buttonVoltage >= ( SELECT_10BIT_ADC - BUTTONHYSTERESIS )
              && buttonVoltage <= ( SELECT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_SELECT;
  }
  //handle button flags for just pressed and just released events
  if ( ( buttonWas == BUTTON_NONE ) && ( button != BUTTON_NONE ) )
  {
    //the button was just pressed, set buttonJustPressed, this can optionally be used to trigger a once-off action for a button press event
    //it's the duty of the receiver to clear these flags if it wants to detect a new button change event
    buttonJustPressed  = true;
    buttonJustReleased = false;
  }
  if ( ( buttonWas != BUTTON_NONE ) && ( button == BUTTON_NONE ) )
  {
    buttonJustPressed  = false;
    buttonJustReleased = true;
  }
  //save the latest button value, for change event detection next time round
  buttonWas = button;
  return ( button );
}
/*--------------------------------------------------------------------------------------
Get frequency for a station in the preset list
--------------------------------------------------------------------------------------*/
void selectStation() {
  frequency = stationsMHZ[presetCounter];
  lcd.setCursor(0, 1);
  lcd.print(String(frequency) + "mHz" + "                ");
  radio.selectFrequency(frequency); // set radio frequency

  lcd.setCursor(0, 1);
  lcd.print(stations[presetCounter] + "                "); // show station name
}
/*--------------------------------------------------------------------------------------
* Send a frequency to the TEA 5767
--------------------------------------------------------------------------------------*/
void selectStation2() {
  lcd.setCursor(0, 1);
  lcd.print(String(frequency) + "mHz" + "                ");
  radio.selectFrequency(frequency); // set radio frequency
  delay(200);
}
/*--------------------------------------------------------------------------------------
* Get Stereo Status
--------------------------------------------------------------------------------------*/
void printStereoStatus() {
  lcd.setCursor(10, 0);
  if (radio.isStereo()) {
    lcd.print("Stereo");
  } else {
    lcd.print("Mono  ");
  }
}
/*--------------------------------------------------------------------------------------*/

