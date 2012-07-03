/*
 * $Id: $
 */

// class header
#include "emu/pc/ResultRegisterSerializer.h"

// Emu includes
#include "emu/pc/CCBCommands.h"
#include "emu/pc/CCB.h"
#include "emu/exception/Exception.h"
#include "emu/utils/SimpleTimer.h"

// XDAQ includes
#include "cgicc/Cgicc.h"
#include "toolbox/string.h"

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

// The slots that are assigned to TMBs:
// 2, 4, 6, 8, 10, 14, 16, 18, 20
// LUT slot -> sequential TMB index
int ResultRegisterSerializer::slotToIndexLUT_[22] = {0,0,0,0,1,0,2,0,4,0,5,0,0,0,6,0,7,0,8,0,9};


ResultRegisterSerializer::ResultRegisterSerializer(CCB *ccb, int tmb_slot)
: ccb_(ccb)
, tmbSlot_(tmb_slot)
, verbose_(0)
{
  shiftCsra2Alct_ = slotToIndexLUT_[tmb_slot] + 1;
  shiftCsra2Tmb_ = slotToIndexLUT_[tmb_slot] + 10;
  shiftCsra3Tmb_ = slotToIndexLUT_[tmb_slot] - 7;

  if (ccb_ == 0)
  {
    XCEPT_RAISE(emu::exception::CCBException, "CCBStatusBitsSerializer constructor: ccb == 0" );
  }

  // Make sure that CCB is in FPGA mode
  SetFPGAMode(ccb_);
}

void ResultRegisterSerializer::setVerbose(int level)
{
  verbose_ = level;
}


uint32_t ResultRegisterSerializer::read(uint32_t length)
{
  using std::hex;
  using std::dec;

  emu::utils::SimpleTimer timer;

  unsigned long long int result = 0;
  if(verbose_>0)
    cout << "--- ResultRegisterSerializer::read ---" << endl;

  uint32_t counter = 0;
  while(counter < length && counter < TMB_RESULT_REGISTER_WIDTH)
  {
    // CSRB2 data bus command: read TMB result register through
    ccb_->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_RR_READ);

    usleep(50);

    // read status registers
    int csra2 = ccb_->ReadRegister(CCB_CSRA2_STATUS);
    if(verbose_>0)
      cout << counter << ": csra2 = " << hex << csra2 << dec  <<" "<< std::bitset< 17 >(csra2);

    int data_bit = 0;
    if (tmbSlot_ < 16)  // TMB status bit is in CSRA2
    {
      data_bit = (csra2 >> shiftCsra2Tmb_) & 1;
    }
    else                // TMB status bit is in CSRA3
    {
      int csra3 = ccb_->ReadRegister(CCB_CSRA3_STATUS);
      if(verbose_>0)
        cout << counter << ": csra3 = " << hex << csra3 << dec << " " << std::bitset< 17 >(csra3);
      data_bit = (csra3 >> shiftCsra3Tmb_) & 1;
    }

    int status_bit = (csra2 >> shiftCsra2Alct_) & 1;
    if(verbose_>0)
      cout << " Status bit " << status_bit << " data " << data_bit ;

    result |= (data_bit << counter);

    if(verbose_>0)
      cout <<"   result " << result << " " << std::bitset< 17 >(result) << endl;

    if(status_bit)
    {
      break;
    }
    else
    {
      ++counter;
    }
  }
  
  if(verbose_>0)
    cout<<"ResultRegisterSerializer::read[msec] "<<timer.sec()*1000.<<endl;

  return result;
}


}} // namespaces
