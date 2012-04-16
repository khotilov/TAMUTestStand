/*
 * $Id: $
 */

// class header
#include "emu/pc/CCBBackplaneTester.h"

// Emu includes
#include "emu/pc/TMB.h"
#include "emu/pc/CCB.h"
#include "emu/pc/CCBCommands.h"
#include "emu/pc/TestUtils.h"

// system includes
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h> // for sleep()
#include <boost/bind.hpp>


namespace emu { namespace pc {

using std::endl;
using std::cout;
using std::string;
using std::hex;
using std::dec;


CCBBackplaneTester::CCBBackplaneTester()
: ccb_(0)
, tmb_(0)
{
  RegisterTestProcedures();
}


CCBBackplaneTester::~CCBBackplaneTester(){}


CCBBackplaneTester::CCBBackplaneTester(const CCBBackplaneTester &other)
{
  CopyFrom(other);
}


CCBBackplaneTester & CCBBackplaneTester::operator=(const CCBBackplaneTester &rhs)
{
  CopyFrom(rhs);
  return *this;
}


void CCBBackplaneTester::CopyFrom(const CCBBackplaneTester &other)
{
  ccb_ = other.ccb_;
  tmb_ = other.tmb_;

  // the real need for the user defined copy c-tor and assignment comes from here:
  // we need to make sure that the test procedures are properly bound to this object.
  RegisterTestProcedures();
}


void CCBBackplaneTester::RegisterTestProcedures()
{
  cout<<__PRETTY_FUNCTION__<<endl;

  RegisterTheTest("PulseCounters", boost::bind( &CCBBackplaneTester::TestPulseCounters, this));
  RegisterTheTest("CommandBus", boost::bind( &CCBBackplaneTester::TestCommandBus, this));
  RegisterTheTest("Dummy", boost::bind( &CCBBackplaneTester::TestDummy, this));
}


void CCBBackplaneTester::Reset()
{
  out() << "CCBBackplaneTester: Hard reset through CCB" << endl;
  if ( ccb_ )
  {
    ccb_->hardReset();
    usleep(5000);
  }
  else
  {
    out() << "No CCB defined!" << endl;
  }
}


void CCBBackplaneTester::PrepareHWForTest()
{
  // make sure CCB is in FPGA mode
  SetFPGAMode(ccb_);

  // issue L1Reset to reset the counters
  ccb_->WriteRegister(CCB_VME_L1RESET, 1);
}


////////////////////////////////////////////////////
// Actual tests:
////////////////////////////////////////////////////

bool CCBBackplaneTester::TestPulseCounters()
{
  bool result = true;

  int Niter = 100;

  // walk through the pulse counter flags bits
  for (int ibit = 0; ibit < LENGTH_PULSE_IN_COMMANDS; ++ibit)
  {
    const int command = PULSE_IN_COMMANDS[ibit];

    int counter_flag = (1 << ibit);

    cout<< __func__ <<" command & flag "<<hex<<command<< " "<<counter_flag<<dec<<endl;

    // read pulse counter flags from TMB RR:
    int counter_flags_read = LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTERS_FLAG);
    counter_flags_read = ResultRegisterData(counter_flags_read);

    // make sure counter flags are off!!!
    result &= CompareValues(out(), "PulseCounters", counter_flags_read, 0, true);

    // for finite pulse commands
    if (isFinitePulseCommand(command))
    {
      // Niter times write anything (it doesn't matter what for pulse commands)
      NTimesWriteRegister(ccb_, Niter, command, 1);
    }
    if (command == CCB_VME_TMB_RESERVED0)
    {
      // set this bit on and off a number of times
      for (int i=0; i<Niter; ++i)
      {
        WriteTMBReserved0Bit(ccb_, 1); // on
        usleep(50);
        WriteTMBReserved0Bit(ccb_, 0); // off
      }
    }

    // read pulse counter flags from TMB RR:
    counter_flags_read = LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTERS_FLAG);
    counter_flags_read = ResultRegisterData(counter_flags_read);

    cout<< __func__ <<" flags write/read "<<(counter_flags_read==counter_flag? "OK ": "BAD ")  << counter_flag<<" "<< counter_flags_read<<endl;

    // fail the test if not equal
    result &= CompareValues(out(), "PulseCounters", counter_flags_read, counter_flag, true, false);


    // read total counter for pulses from TMB RR:
    int counter_read = LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTER);
    counter_read = ResultRegisterData(counter_read);

    // compare to the expected numbers
    if (is25nsPulseCommand(command))
    {
      result &= CompareValues(out(), "PulseCounters", counter_read, Niter, true, false);
    }
    else if (is500nsPulseCommand(command))
    {
      result &= CompareValues(out(), "PulseCounters", (float)counter_read/Niter, 22., .1, false);
    }
    else if (command == CCB_VME_ALCT_ADB_PULSE_ASYNC)
    {
      result &= CompareValues(out(), "PulseCounters", (float)counter_read/Niter, 15., .1, false);
    }

    cout<< __func__ <<" counter read "<< counter_read<<"  average = "<<(float)counter_read/Niter<<endl<<endl;


    // issue L1Reset to reset the counters
    ccb_->WriteRegister(CCB_VME_L1RESET, 1);
  }

  cout<< __func__ <<" result " << result <<endl;

  return result;
}


bool CCBBackplaneTester::TestCommandBus()
{
  bool result = true;

  int Niter = 100;

  int reg[] = {0x05, 0x0A, 0x14, 0x28, 0x51, 0xA2, 0x60, 0x80};

  // walk over the range of values of the CSRB2 register
  for (int j = 0; j < 8; ++j)
  {

    cout<<endl<< __func__ <<" testing CSRB2 "<<hex<<reg[j]<<endl<<endl;

    for (int i=0; i<Niter; ++i)
    {
      // load result register with the command code and read it back
      int rr = LoadAndReadResutRegister(ccb_, tmb_->slot(), reg[j]);

      // extract command bits
      int command_code = ResutRegisterCommand(rr);
      //int pulse_counter = ResutRegisterData(rr);

      cout<< __func__ <<" write/read "<< (command_code==reg[j]? "OK ": "BAD ") << " = " << reg[j] << " / " << command_code <<endl;

      // compare the read out command code to the value written into the CSRB2 register
      result &= CompareValues(out(), "CommandBus", command_code, reg[j], true);
    }

    // issue L1Reset to reset the counters
    ccb_->WriteRegister(CCB_VME_L1RESET, 1);
  }

  cout<< __func__ <<" result "<< result <<endl;

  return result;
}



}} // namespaces
