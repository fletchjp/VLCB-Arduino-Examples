# VLCB_DUE3EEPROM

This a VLCB version of CANDUE3EEPROM which expands CANDUE3 to allow for the optional use of external EEPROM

The compiling of code optional for the DUE can be controlled using __SAM3X8E__

This code uses files called CANSAM3X8E.h and CANSAM3X8E.cpp which implement the VLCB interface for the DUE. These can be made into a library when testing is finished.

These files use the libraries due_can (Version 2.1.6) and can_common(0.3.0). Do not use other versions of due_can.

The present status of this file is that it is a test version to check installation to CAN using FCU 1.5.4.

It does not currently find the correct CANID and this is being investigated.

Further use needs more development of the VLCB Arduino library to include event teaching.

John Fletcher 13/11/2023



