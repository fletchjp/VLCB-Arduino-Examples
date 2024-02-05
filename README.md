# VLCB-Arduino-Examples
 
Examples using the VLCB Arduino library
 
These examples have now been autoformatted. 
 
## Arduino UNO Examples

These examples are for Arduino UNO hardware with one of the Kit 110 Arduino shields.

### VLCBLCDBut

This is code to use the DFRobot LCD shield with buttons. This is running currently without events.

Note that the CS pin is changed from 10 to 15 for this hardware.

Full VLCB             28074/1315

### VLCBLCDBut_1280

This is code to use the DFRobot LCD shield with buttons. This is running currently without events.

This is a version for the MEGA 1280

Note that the CS pin is changed from 10 to 53 for this hardware. INT pin is 19.

No LED user service   27292/1102

### VLCBLCDButTsk

This is the same code as above, with TaskManagerIO used to control the code for the analog buttons. This is very tight on memory and I want to experiment to see what of VLCB I can remove to save memory.

Full VLCB             32010/1382   (31994/1366 in the old location) the difference must be to do with different length of location information.
NoLEDUserService      31072/1265

This means that the overhead for the task manager is about 4k memory but not much data.

This module is now too big for a UNO.

### VLCBLCDButTsk_1280

This is a version of the previous example for the MEGA 1280 with the same pin changes.

NoLEDUserService      35198/12691

### VLCB_DUE3EEPROM

This example is running on a CANDUE with a dual CAN interface.

There is also an external EEPROM which is now being used optionally. This is used to save the configuration which is otherwise lost when the code is reloaded.

This is a VLCB version of CANDUE3EEPROM which expands CANDUE3 to allow for the optional use of external EEPROM

It has with it files for CANSAM3X8E which provides the CAN interface for the DUE.

The code now compiles and runs and needs more testing.

It is now working to integrate a module into a CAN configuration using FCU 1.5.4

It is important to have the libraries due_can 2.1.6 and can_common 0.3.0 installed. Earlier versions of due_can available from the Arduino IDE system do NOT work.

There is a residual problem with the CANID allocated which is to be worked on.

John Fletcher 2/12/2023 



