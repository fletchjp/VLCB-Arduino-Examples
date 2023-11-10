// Copyright (C) Sven Rosvall (sven@rosvall.ie) (C( John Fletcher (john@bunbury28.plus.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

// Arduino libraries
#include <SPI.h>

// 3rd party libraries
#include <Streaming.h>

// VLCB SAM3X8E library
#include "CAN2515.h"
#include "CANSAM3X8E.h"


namespace VLCB
{

//
/// constructor
//
CANSAM3X8E::CANSAM3X8E()
  : _num_rx_buffers(NUM_RX_BUFFS)
  , _num_tx_buffers(NUM_TX_BUFFS)
  , _csPin(MCP2515_CS)
  , _intPin(MCP2515_INT)
  , _osc_freq(OSCFREQ)
{
   _instance = 0;
   _can = &Can0;
}

//
/// initialise the CAN controller and buffers, and attach the ISR
//

bool CANSAM3X8E::begin(bool poll, SPIClass spi) {

  uint32_t init_ret;
  int init_watch;

  _numMsgsSent = 0;
  _numMsgsRcvd = 0;

  // init CAN instance
  init_ret = _can->begin(CAN_BPS_125K, 255);

  if (!init_ret) {
    Serial << "> CAN error from begin(), ret = " << init_ret << endl;
    return false;
  }

  // set filter to permissive
  init_watch = _can->watchFor();

  if (init_watch == -1){ // Test changed by JPF
    Serial << "> CAN error from watchFor(), ret = " << init_watch << endl;
    return false;
  }

  Serial << "> CAN controller instance = " << _instance << " initialised successfully" << endl;
  return true;
}

bool CANSAM3X8E::available(void) {

  return _can->available();
}

CANMessage CANSAM3X8E::getNextCanMessage(void) {

  uint32_t ret;
  CAN_FRAME cf;
  CANMessage message;

  ret = _can->read(cf);

  if (ret != CAN_MAILBOX_TRANSFER_OK) {
    Serial << "> CAN read did not return CAN_MAILBOX_TRANSFER_OK, instance = " << _instance << ", ret = " << ret << endl;
  } else {
    Serial << "> received CAN message ok, instance = " << _instance << endl;
  }

  message.id = cf.id;
  message.len = cf.length;
  message.rtr = cf.rtr;
  message.ext = cf.extended;

  memcpy(message.data, cf.data.byte, cf.length);

  ++_numMsgsRcvd;

  // format_message(&_msg);
  return message;
}

//
/// send a CBUS message
//

bool CANSAM3X8E::sendCanMessage(CANMessage *msg) {

  bool ret;
  CAN_FRAME cf;                         // library-specific CAN message structure

  //makeHeader(msg, priority);            // set the CBUS header - CANID and priority bits
  // format_message(msg);

  cf.id = msg->id;
  cf.length = msg->len;
  cf.rtr = msg->rtr;
  cf.extended = msg->ext;

  memcpy(cf.data.bytes, msg->data, msg->len);

  ret = _can->sendFrame(cf);

  if (!ret) {
    Serial << "> error sending CAN message, instance = " << _instance << ", ret = " << ret << endl;
  }

  return ret;
}

//
/// display the CAN bus status instrumentation
//

void CANSAM3X8E::printStatus(void) {

  return;
}

//
/// reset the CAN driver
//

void CANSAM3X8E::reset(void) {

}

//
/// set the TX and RX pins
//

void CANSAM3X8E::setPins(byte txPin, byte rxPin) {

  return;
}

//
/// set the depth of the TX and RX queues
//

void CANSAM3X8E::setNumBuffers(byte num) {

  return;
}

//

//
/// set the CAN controller peripheral instance, there are two, default is zero
//

void CANSAM3X8E::setControllerInstance(byte instance) {

  Serial << "> setting CAN controller instance to " << instance << endl;
  _instance = instance;
  _can = (_instance == 0) ? &Can0 : &Can1;
}

// End of namespace VLCB
}