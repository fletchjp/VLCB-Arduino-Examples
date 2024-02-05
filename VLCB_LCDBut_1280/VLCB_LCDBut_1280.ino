// VLCB_LCDBut_1280
// This is a version of a code with a long name.
// LCDshieldButtonsSerialDFRobot.ino 
// Adapted to run with VLCB on a MEGA 1280 with changed pins.

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

// constants
const byte VER_MAJ = 1;     // code major version
const char VER_MIN = 'b';   // code minor version
const byte VER_BETA = 0;    // code beta sub-version
const byte MODULE_ID = 99;  // VLCB module type

const byte LED_GRN = 4;  // VLCB green Unitialised LED pin
const byte LED_YLW = 7;  // VLCB yellow Normal LED pin
const byte SWITCH0 = 8;  // VLCB push button switch pin

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
                            { &mnService, &serialUserInterface, &canService, &nvService, &ecService, &epService, &etService });  // Controller object

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
#include <LiquidCrystal.h>
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

/////////////////////////////////////////////////////////////////////
//
/// setup VLCB - runs once at power on from setup()
//
void setupVLCB()
{
  // set config layout parameters
  modconfig.EE_NVS_START = 10;
  modconfig.EE_NUM_NVS = 10;
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
  Serial << F(", CANID = ") << modconfig.CANID;
  Serial << F(", NN = ") << modconfig.nodeNum << endl;
  Serial << F("> mode = (") << _HEX(modconfig.currentMode) << ") " << modeString;

  // show code version and copyright notice
  printConfig();

  // set module parameters
  VLCB::Parameters params(modconfig);
  params.setVersion(VER_MAJ, VER_MIN, VER_BETA);
  params.setModuleId(MODULE_ID);

  // assign to Controller
  controller.setParams(params.getParams());
  controller.setName(mname);

  //  {
  //    Serial << F("> switch was pressed at startup in Uninitialised mode") << endl;
  //    modconfig.resetModule(&userInterface);
  //  }

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
  can2515.setOscFreq(CAN_OSC_FREQ);   // select the crystal frequency of the CAN module
  can2515.setPins(CAN_CS_PIN, CAN_INT_PIN);   // select pins for CAN bus CE and interrupt connections
  if (!can2515.begin()) {
    Serial << F("> error starting VLCB") << endl;
  }
}

bool have_error_flag;


void setup()
{
  // put your setup code here, to run once:

  // Initialise
  Serial.begin(SERIAL_SPEED);  // Start Serial IO.
  Serial << endl
         << endl
         << F("> ** VLCB LCD Buttons ** ") << __FILE__ << endl;
  //analogWrite(pin_d6,50);
  setupVLCB();
  have_error_flag = false;

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("VLCB LCD");
  lcd.setCursor(0, 1);
  lcd.print("Press Key:");
  // end of setup
  Serial << F("> ready") << endl
         << endl;
}

void checkA0()
{
  // put your main code here, to run repeatedly:
  x = analogRead(0);
  if (x < 175) {  // was 50
    range = 1;
  } else if (x < 350) {  // was 250
    range = 2;
  } else if (x < 500) {  // unchanged
    range = 3;
  } else if (x < 800) {  // was 650
    range = 4;
  } else if (x < 850) {  // unchanged
    range = 5;
  }  //else { range = 0; }
  if (range != prevrange) {
    Serial.print(range);
    Serial.print(" ");
    Serial.print(x);
    lcd.setCursor(10, 1);
    switch (range) {
      case 1:
        {
          lcd.print("Right ");
          //if (y == 0) {
          Serial.println(" Right");
          //y = 1;
          //}
          break;
        }
      case 2:
        {
          lcd.print("Up    ");
          Serial.println(" Up");
          break;
        }
      case 3:
        {
          lcd.print("Down  ");
          Serial.println(" Down");
          break;
        }
      case 4:
        {
          lcd.print("Left  ");
          Serial.println(" Left ");
          break;
        }
      case 5:
        {
          lcd.print("Select");
          Serial.println(" Select");
          break;
        }
      default:
        break;
    }
    prevrange = range;
  }
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
    Serial << F("> error flag register is non-zero") << endl;
    have_error_flag = true;
  }

  // check for buttons on display - I want this to be a task.
  checkA0();
  // bottom of loop()
}


//
/// test for switch input
/// as an example, it must be have been pressed or released for at least half a second
/// then send a long VLCB event with opcode ACON for on and ACOF for off

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
