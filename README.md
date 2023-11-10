# VLCB-Arduino-Examples
 Examples using the VLCB Arduino library
 
## Arduino UNO Examples

These examples are for Arduino UNO hardware with one of the Kit 110 Arduino shields.

### VLCBLCDBut

This is code to use the DFRobot LCD shield with buttons. This is running currently without events.

Note that the CS pin is changed from 10 to 15 for this hardware.

Full VLCB             28074/1315

### VLCBLCDButTsk

This is the same code as above, with TaskManagerIO used to control the code for the analog buttons. This is very tight on memory and I want to experiment to see what of VLCB I can remove to save memory.

Full VLCB             32010/1382   (31994/1366 in the old location) the difference must be to do with different length of location information.
NoLEDUserService      31072/1265

This means that the overhead for the task manager is about 4k memory but not much data.

### VLCB_DUE3EEPROM

This a VLCB version of CANDUE3EEPROM which expands CANDUE3 to allow for the optional use of external EEPROM

It has with it files for CANSAM3X8E which provides the CAN interface for the DUE.

The code now compiles and runs and needs more testing.


