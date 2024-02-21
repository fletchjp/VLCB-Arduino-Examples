// VLCB_LCDButTsk
// This is the VLCB version similar to the CANshield_LCDBut
// This example is designed to work with the Arduino CAN shield (MERG Kit 110) with an Arduino UNO.
// The Kit 110 shield is placed on the Arduino UNO and a second shield placed on top.
// Adapted to run with VLCB on a MEGA 1280 with changed pins.

// https://www.dfrobot.com/product-51.html (DFR0009)
// This shield has a 2 by 16 character LCD display and a set of buttons.
// The code allows for CBUS events to be sent using the buttons.
// What the button actions do can be configured.
// Currently the display shows the button actions.
// It can be extended to respond to incoming CBUS events.
//////////////////////////////////////////////////////////////////////////////////
// NOTE: This code does not support the Button and LEDs for CBUS configuration.
//       This is because the display uses the same pins.
//       In any case, the button and LEDs are hidden behind the display shield.
///////////////////////////////////////////////////////////////////////////////////
// Pin Use map UNO:
// Digital pin 2          Interupt CAN
// Digital pin 3 (PWM)    Not used
// Digital pin 4          LCD pin_d4
// Digital pin 5 (PWM)    LCD pin_d5
// Digital pin 6 (PWM)    LCD pin_d6
// Digital pin 7          LCD pin_d7
// Digital pin 8          LCD pin_RS
// Digital pin 9 (PWM)    LCD pin_EN
// Digital pin 10         LCD backlight pin - this would normally be for the CS
// Digital pin 11 (MOSI)  SI    CAN
// Digital pin 12 (MISO)  SO    CAN
// Digital pin 13 (SCK)   Sck   CAN

// Digital pin 14 / Analog pin 0  Analog input from buttons
// Digital pin 15 / Analog pin 1 (SS)    CS    CAN
// Digital pin 16 / Analog pin 2  Not used
// Digital pin 17 / Analog pin 3  Not used
// Digital / Analog pin 4     Not Used - reserved for I2C
// Digital / Analog pin 5     Not Used - reserved for I2C
//////////////////////////////////////////////////////////////////////////

// IoAbstraction libraries
#include <IoAbstraction.h>
#include <DfRobotInputAbstraction.h>
#include <TaskManagerIO.h>
#include <DeviceEvents.h>
#include <LiquidCrystalIO.h>

/// This uses the default settings for analog ranges.
IoAbstractionRef dfRobotKeys = inputFromDfRobotShield();

/// It is in fact set as default defining dfRobotKeys.
#define ANALOG_IN_PIN A0

// 3rd party libraries
#include <Streaming.h>

////////////////////////////////////////////////////////////////////////////
// VLCB library header files
#include <Controller.h>     // Controller class
#include <CAN2515.h>        // CAN controller
#include <Switch.h>         // pushbutton switch
#include <LED.h>            // VLCB LEDs
#include <Configuration.h>  // module configuration
#include <Parameters.h>     // VLCB parameters
#include <vlcbdefs.hpp>     // VLCB constants
#include "MinimumNodeService.h"
#include "CanService.h"
#include "NodeVariableService.h"
#include "EventConsumerService.h"
#include "EventProducerService.h"
#include "EventTeachingService.h"
#include "SerialUserInterface.h"
//#include "CombinedUserInterface.h"

////////////DEFINE MODULE/////////////////////////////////////////////////
/// Use these values for the VLCB outputs from the display shield buttons
/// These values give intial values which are not used elsewhere.
int button = -1;
int prevbutton = -1;

// constants
const byte VER_MAJ = 2;     // code major version
const char VER_MIN = 'b';   // code minor version
const byte VER_BETA = 0;    // code beta sub-version
const byte MODULE_ID = 81;  // VLCB module type

#define NUM_NVS 10
// This defines an array to hold the state of each button.
//byte buttonState[NUM_NVS];

// Controller objects
VLCB::Configuration modconfig;  // configuration object
VLCB::CAN2515 can2515;          // CAN transport object
VLCB::SerialUserInterface serialUserInterface(&can2515);
VLCB::MinimumNodeService mnService;
VLCB::CanService canService(&can2515);
VLCB::NodeVariableService nvService;
VLCB::EventConsumerService ecService;
VLCB::EventTeachingService etService;
VLCB::EventProducerService epService;
VLCB::Controller controller( &modconfig,
                            { &mnService,&serialUserInterface, &canService, &nvService, &ecService, &epService, &etService });  // Controller object

// module name, must be 7 characters, space padded.
unsigned char mname[7] = { 'L', 'C', 'D', 'B', 'u', 't', ' ' };

// forward function declarations
void eventhandler(byte, VLCB::VlcbMessage *, bool ison, byte evval);
void processSerialInput();
void printConfig();

//CBUS pins
const byte CAN_INT_PIN = 19;
const byte CAN_CS_PIN = 53; // Changed from 10 because of the LCD shield.

const unsigned long CAN_OSC_FREQ = 16000000UL;     // Oscillator frequency on the CAN2515 board


//////////////////////////////////////////////////////////////////////////////////////////////

// LCD shield with buttons example code

/*
Arduino 2x16 LCD - Detect Buttons
modified on 18 Feb 2019
by Saeed Hosseini @ Electropeak
https://electropeak.com/learn/
Using data from here:
https://wiki.dfrobot.com/Arduino_LCD_KeyPad_Shield__SKU__DFR0009_
I have also played with the values.
*/
//LCD pin to Arduino
const int pin_RS = 8;
const int pin_EN = 9;
const int pin_d4 = 4;
const int pin_d5 = 5;
const int pin_d6 = 6;
const int pin_d7 = 7;
const int pin_BL = 10;
LiquidCrystal lcd(pin_RS, pin_EN, pin_d4, pin_d5, pin_d6, pin_d7);

int x;
int prevx = 0;
int range;
int prevrange = 0;
int y = 0;

// Serial IO
#define SERIAL_SPEED 115200  // Speed of the serial port.
/*
// This is following the methods in EzyBus_master to provide error messages.
// These have been limited to 16 chars to go on an LCD 2 by 16 display.
// blank_string is used to cancel an error message.
const char blank_string[] PROGMEM = "                ";
const char error_string_0[] PROGMEM = "no error";
const char error_string_1[] PROGMEM = "Test message";
const char error_string_2[] PROGMEM = "Emergency Stop";
const char error_string_3[] PROGMEM = "CANbus error";
const char error_string_4[] PROGMEM = "invalid error";

const char* const error_string_table[] PROGMEM = {
  blank_string, error_string_0, error_string_1, error_string_2, error_string_3, error_string_4
};

#define MAX_ERROR_NO 5

// Buffer for string output.
// This has been made safe for line termination.
#define MAX_LENGTH_OF_STRING 16
#define LENGTH_OF_BUFFER (MAX_LENGTH_OF_STRING + 1)
char error_buffer[LENGTH_OF_BUFFER];

void getErrorMessage(int i);
*/
/*
// This is new in this version of the code and may be useful elsewhere.
// It is used to transfer error details to the DrawingEvent
// which is why it is declared here.
struct Error {
  int i;
  byte x;
  byte y;
  Error()
    : i(0), x(0), y(0) {}
  Error(int ii, byte xx, byte yy)
    : i(ii), x(xx), y(yy) {}
  Error(const Error& e)
    : i(e.i), x(e.x), y(e.y) {}
};
*/

/**
 * Here we create an event that handles all the drawing for an application, in this case printing out readings
 * of a sensor when changed. It uses polling and immediate triggering to show both examples
 */
class DrawingEvent : public BaseEvent
{
private:
  volatile bool emergency;  // if an event comes from an external interrupt the variable must be volatile.
  bool hasChanged;
  bool hasKey;
  char key[7];
  //bool hasError;
  //Error error;
public:
  /** This constructor sets the initial values for various variables. */
  DrawingEvent()
  {
    hasChanged = false;
    hasKey = false;
    //key = "      ";
    //hasError = false;
  }
  /**
     * This is called by task manager every time the number of microseconds returned expires, if you trigger the
     * event it will run the exec(), if you complete the event, it will be removed from task manager.
     * @return the number of micros before calling again. 
     */
  uint32_t timeOfNextCheck() override
  {
    setTriggered(hasChanged);
    return millisToMicros(500);  // no point refreshing more often on an LCD, as its unreadable
  }

 /**
     * This is called when the event is triggered, it prints all the data onto the screen.
     * Note that each source of input has its own bool variable.
     * This ensures that only the items needing output are executed.
     */
  void exec() override
  {
    hasChanged = false;

    if (hasKey) {
      hasKey = false;
      lcd.setCursor(10, 1);
      lcd.print(key);
    }
    /*
    if (hasError) {
      getErrorMessage(error.i);
      lcd.setCursor(error.x, error.y);
      lcd.write("E: ");
      lcd.write(error_buffer);
      hasError = false;
    }
    */
  }

  /* This provides for the logging of the key information
       This is an example of something coming from an internal event. */
  void drawKey(const char* whichKey)
  {
    memcpy(key, whichKey, 7);  //= whichKey;
    hasKey = true;
    hasChanged = true;  // we are happy to wait out the 500 millis
  }
 /* This provides for the logging of the error information.
     * This is an example of something coming from an external event.
     * The Error object holds the data for plotting. */
/*
  void displayError(const Error& e)
  {
    error = e;
    hasError = true;
    hasChanged = true;  // we are happy to wait out the 500 millis
  }
  */
 /**
     * Triggers an emergency that requires immediate update of the screen
     * @param isEmergency if there is an urgent notification
     * This is not used at present and is included from the source example.
     */
  void triggerEmergency(bool isEmergency)
  {
    emergency = isEmergency;
    markTriggeredAndNotify();  // get on screen asap.
  }
};

// create an instance of the above class
DrawingEvent drawingEvent;

/*
// Add check for invalid error
void getErrorMessage(int i)
{
  if (i >= 0 && i <= MAX_ERROR_NO) {
    strncpy_P(error_buffer, (char*)pgm_read_word(&(error_string_table[i])), MAX_LENGTH_OF_STRING);
  } else {
    strncpy_P(error_buffer, (char*)pgm_read_word(&(error_string_table[MAX_ERROR_NO])), MAX_LENGTH_OF_STRING);
  }
}


void serialPrintError(int i)
{
  getErrorMessage(i);
  Serial.print(error_buffer);
}
void serialPrintErrorln(int i)
{
  getErrorMessage(i);
  Serial.println(error_buffer);
}
*/
/////////////////////////////////////////////////////////////////////
//
/// setup VLCB - runs once at power on from setup()
//
void setupVLCB()
{
  // set config layout parameters
  modconfig.EE_NVS_START = 10;
  modconfig.EE_NUM_NVS = NUM_NVS;
  modconfig.EE_EVENTS_START = 20;
  modconfig.EE_MAX_EVENTS = 32;
  modconfig.EE_PRODUCED_EVENTS = 1;
  modconfig.EE_NUM_EVS = 1;

  // initialise and load configuration
  controller.begin();

  const char *modeString;
  switch (modconfig.currentMode) {
    case MODE_NORMAL: modeString = "Normal"; break;
    case MODE_SETUP: modeString = "Setup"; break;
    case MODE_UNINITIALISED: modeString = "Uninitialised"; break;
    default: modeString = "Unknown"; break;
  }
  Serial << F("> mode = (") << _HEX(modconfig.currentMode) << ") " << modeString;
  Serial << F(", CANID = ") << modconfig.CANID;
  Serial << F(", NN = ") << modconfig.nodeNum << endl;

  // show code version and copyright notice
  printConfig();

  // set module parameters
  VLCB::Parameters params(modconfig);
  params.setVersion(VER_MAJ, VER_MIN, VER_BETA);
  params.setModuleId(MODULE_ID);

  // assign to Controller
  controller.setParams(params.getParams());
  controller.setName(mname);

  // opportunity to set default NVs after module reset
  if (modconfig.isResetFlagSet()) {
    Serial << F("> module has been reset") << endl;
    modconfig.clearResetFlag();
  }

  // register our VLCB event handler, to receive event messages of learned events
  ecService.setEventHandler(eventhandler);

  // set Controller LEDs to indicate mode
  controller.indicateMode(modconfig.currentMode);

  // configure and start CAN bus and VLCB message processing
  can2515.setNumBuffers(2, 2);     // more buffers = more memory used, fewer = less
  can2515.setOscFreq(CAN_OSC_FREQ);  // select the crystal frequency of the CAN module
  can2515.setPins(CAN_CS_PIN, CAN_INT_PIN);  // select pins for CAN bus CS and interrupt connections
  if (!can2515.begin()) {
    Serial << F("> error starting VLCB") << endl;
  }
}

bool have_error_flag;

/*
void setupModule()
{
  for (int i = 0; i < NUM_NVS; i++) {
    buttonState[i] = false;
  }
}
*/

void logKeyPressed(int pin, const char* whichKey, bool heldDown)
{
  drawingEvent.drawKey(whichKey);
  Serial.print("Key ");
  Serial.print(whichKey);
  Serial.println(heldDown ? " Held" : " Pressed");
  button = pin + 1;  // Increment to avoid event 0
}

/**
 * Along with using functions to receive callbacks when a button is pressed, we can
 * also use a class that implements the SwitchListener interface. Here is an example
 * of implementing that interface. You have both choices, function callback or
 * interface implementation.
 */
class MyKeyListener : public SwitchListener
{
private:
  const char* whatKey;
public:
  // This is the constructor where we configure our instance
  MyKeyListener(const char* what)
  {
    whatKey = what;
  }

  // when a key is pressed, this is called
  void onPressed(pinid_t pin, bool held) override
  {
    logKeyPressed(pin, whatKey, held);
    button = pin + 1;
  }

  // when a key is released this is called.
  void onReleased(pinid_t pin, bool held) override
  {
    Serial.print("Release ");
    logKeyPressed(pin, whatKey, held);
  }
};


MyKeyListener selectKeyListener("SELECT");

void setup1602()
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("VLCBshieldLCDBut");
  lcd.setCursor(0, 1);
  lcd.print("Press Key:");
}

void setupSwitches()
{
  // initialise the switches component with the DfRobot shield as the input method.
  // Note that switches is the sole instance of SwitchInput
  switches.initialise(dfRobotKeys, false);  // df robot is always false for 2nd parameter.

  // now we add the switches, each one just logs the key press, the last parameter to addSwitch
  // is the repeat frequency is optional, when not set it implies not repeating.
  switches.addSwitch(
    DF_KEY_DOWN, [](pinid_t pin, bool held) {
      logKeyPressed(pin, "DOWN  ", held);
    },
    20);
  switches.addSwitch(
    DF_KEY_UP, [](pinid_t pin, bool held) {
      logKeyPressed(pin, "UP    ", held);
    },
    20);
  switches.addSwitch(
    DF_KEY_LEFT, [](pinid_t pin, bool held) {
      logKeyPressed(pin, "LEFT  ", held);
    },
    20);
  switches.addSwitch(
    DF_KEY_RIGHT, [](pinid_t pin, bool held) {
      logKeyPressed(pin, "RIGHT ", held);
    },
    20);
  //switches.onRelease(DF_KEY_RIGHT, [](pinid_t /*pin*/, bool) { Serial.println("RIGHT has been released");});

  switches.addSwitchListener(DF_KEY_SELECT, &selectKeyListener);
}


void setup()
{
  // put your setup code here, to run once:

  // Initialise
  Serial.begin(SERIAL_SPEED);  // Start Serial IO.
  Serial << endl
         << endl
         << F("> ** VLCB LCD Buttons Task ** ") << __FILE__ << endl;
  //analogWrite(pin_d6,50);
  setupVLCB();
  have_error_flag = false;
  setup1602();
  //setupModule();
  setupSwitches();

  // This is at the end of setup()
  //taskManager.scheduleFixedRate(250, checkA0);

  taskManager.registerEvent(&drawingEvent);

  // end of setup
  Serial << F("> ready") << endl
         << endl;
}



//
/// loop - runs forever
//
void loop()
{
  //
  /// do VLCB message, switch and LED processing
  //
  controller.process();

  //
  /// check CAN message buffers
  //
  if (can2515.canp->receiveBufferPeakCount() > can2515.canp->receiveBufferSize()) {
    Serial << F("> receive buffer overflow") << endl;
  }

  if (can2515.canp->transmitBufferPeakCount(0) > can2515.canp->transmitBufferSize(0)) {
    Serial << F("> transmit buffer overflow") << endl;
  }

  //
  /// check CAN bus state
  //
  byte s = can2515.canp->errorFlagRegister();
  if (s != 0 && !have_error_flag) {
    Serial << F("> error flag register is non-zero: ") << s << endl;
    have_error_flag = true;
  }

  taskManager.runLoop();

  // check for buttons on display - I want this to be a task.
  //checkA0();
  // bottom of loop()
}

//
/// user-defined event processing function
/// called from the VLCB library when a learned event is received
/// it receives the event table index and the CAN frame
//
void eventhandler(byte index, VLCB::VlcbMessage *msg, bool ison, byte evval)
{
  // as an example, control an LED

  Serial << F("> event handler: index = ") << index << F(", opcode = 0x") << _HEX(msg->data[0]) << endl;

  // read the value of the first event variable (EV) associated with this learned event
  Serial << F("> EV1 = ") << evval << endl;

 
}

//
/// print code version config details and copyright notice
//
void printConfig()
{
  // code version
  Serial << F("> code version = ") << VER_MAJ << VER_MIN << F(" beta ") << VER_BETA << endl;
  Serial << F("> compiled on ") << __DATE__ << F(" at ") << __TIME__ << F(", compiler ver = ") << __cplusplus << endl;

  // copyright
  Serial << F("> © Duncan Greenwood (MERG M5767) 2019") << endl;
}
