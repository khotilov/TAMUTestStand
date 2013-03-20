/*
 * $Id: $
 */

// class header
#include "emu/pc/CCBCommands.h"

// Emu includes
#include "emu/pc/CCB.h"

// system includes
#include <iostream>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <unistd.h> // for sleep()


namespace emu {  namespace pc {

using std::endl;
using std::cout;
using std::string;
using std::hex;
using std::dec;


bool is25nsPulseCommand(const int command)
{
  switch(command)
  {
    case CCB_VME_BC0:
    case CCB_VME_L1ACC:
    case CCB_VME_SOFT_RESET:
    case CCB_VME_TMB_SOFT_RESET:
    case CCB_VME_CLCT_EXT_TRIGGER:
    case CCB_VME_ALCT_EXT_TRIGGER:
    case CCB_VME_DMB_CFEB_CALIB0:
    case CCB_VME_DMB_CFEB_CALIB1:
    case CCB_VME_DMB_CFEB_CALIB2:
      return true;
  }
  return false;
}


bool is500nsPulseCommand(const int command)
{
  switch(command)
  {
    case CCB_VME_ALCT_HARD_RESET:
    case CCB_VME_ALCT_ADB_PULSE_SYNC:
      return true;
  }
  return false;
}


bool isFinitePulseCommand(const int command)
{
  if (is25nsPulseCommand(command)) return true;
  if (is500nsPulseCommand(command)) return true;

  switch(command)
  {
    case CCB_VME_ALCT_ADB_PULSE_ASYNC: // ??? ns
      return true;
  }
  return false;
}


void SetFPGAMode(CCB *ccb)
{
  ccb->WriteRegister(CCB_CSRA1, 0x00); // CSRA1
  ccb->WriteRegister(CCB_CSRB1, 0xDEED); // CSRB1
  //ccb->WriteRegister(CCB_CSRB1, 0xDFEF); // CSRB1
}


void NTimesWriteRegister(CCB* ccb, int n, int reg, int value)
{
  for (int i = 0; i < n; ++i)
  {
    ccb->WriteRegister(reg, value);
    usleep(50);
  }
}


void Write5ReservedBits(CCB* ccb, int &value)
{
  // make sure the value we are writing is 5 bits wide!
  value &= 0x1F;

  // read the register contents first
  int reg = ccb->ReadRegister(CCB_CSRB6);
  // clear the DMB_reserved_out bits [14:10]:
  reg &= ~(0x1F << 10);
  // set the DMB_reserved_out bits [14:10] to given value
  reg |= (value << 10);

  // write
  ccb->WriteRegister(CCB_CSRB6, reg);
}


int Read5ReservedBits(CCB* ccb)
{
  // read from TMB_reserved_in bits [7:3]:
  int reg = ccb->ReadRegister(CCB_CSRB11);
  return ( reg >> 3) & 0x1F;
}


void WriteTMBReserved0Bit(CCB* ccb, int value)
{
  // make sure the value we are writing is 0 or 1!
  value = ((value==0) ? 0 : 1);

  // read the register contents first
  int reg = ccb->ReadRegister(CCB_CSRB6);
  // clear the TMB_reserved0 bit [2]:
  reg &= ~(0x4);
  // set the TMB_reserved bit [2] to given value
  reg |= (value << 2);

  // write
  ccb->WriteRegister(CCB_CSRB6, reg);
}


uint32_t GetBoardID()
{
  return 0;
}


uint32_t LoadAndReadResultRegister(CCB* ccb, int tmb_slot, int load_command)
{
  ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, load_command);
  ResultRegisterSerializer reader(ccb, tmb_slot);
  //reader.setVerbose(1);
  return reader.read();
}


uint32_t ResultRegisterCommand(uint32_t rr)
{
  // currently, it's first 8 bits (==TMB_RR_WIDTH)
  static const uint32_t command_mask = (0x1<<TMB_RR_COMMAND_WIDTH)-1;
  return rr & command_mask;
}

uint32_t ResultRegisterData(uint32_t rr)
{
  // currently, it's 12 bits [19:8]
  static const uint32_t data_mask = (0x1<<TMB_RR_DATA_WIDTH)-1;
  return ( rr >> TMB_RR_COMMAND_WIDTH ) & data_mask;
}


}} // namespaces
