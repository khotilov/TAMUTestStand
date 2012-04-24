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
#include <set>
#include <unistd.h> // for sleep()
#include <boost/bind.hpp>


namespace emu { namespace pc {

using std::endl;
using std::cout;
using std::string;
using std::set;
using std::hex;
using std::dec;


CCBBackplaneTester::CCBBackplaneTester()
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
  // the real need for the user defined copy c-tor and assignment comes from here:
  // we need to make sure that the test procedures are properly bound to this object.
  RegisterTestProcedures();
}


void CCBBackplaneTester::RegisterTestProcedures()
{
  cout<<__PRETTY_FUNCTION__<<endl;
  RegisterTheTest("L1Reset", boost::bind( &CCBBackplaneTester::TestL1Reset, this));
  RegisterTheTest("TMBHardReset", boost::bind( &CCBBackplaneTester::TestTMBHardReset, this));
  RegisterTheTest("PulseCounters", boost::bind( &CCBBackplaneTester::TestPulseCounters, this));
  RegisterTheTest("CommandBus", boost::bind( &CCBBackplaneTester::TestCommandBus, this));
  RegisterTheTest("CCBReserved", boost::bind( &CCBBackplaneTester::TestCCBReserved, this));
  RegisterTheTest("TMBReservedOut", boost::bind( &CCBBackplaneTester::TestTMBReservedOut, this));
  RegisterTheTest("DMBReservedOut", boost::bind( &CCBBackplaneTester::TestDMBReservedOut, this));
  RegisterTheTest("DataBus", boost::bind( &CCBBackplaneTester::TestDataBus, this));
  RegisterTheTest("CCBClock40", boost::bind( &CCBBackplaneTester::TestCCBClock40, this));
  RegisterTheTest("Dummy", boost::bind( &CCBBackplaneTester::TestDummy, this));
}


////////////////////////////////////////////////////
// Actual tests:
////////////////////////////////////////////////////

int CCBBackplaneTester::TestL1Reset()
{
  bool ok = true;
  
  int Niter = 100;
  
  for(int i=0; i<Niter; ++i)
  {
    // write DataBus with some test value
    ccb_->WriteRegister(CCB_CSRB3_DATA_BUS, 0xFF);
    
    // do the L1 reset
    L1Reset();
    
    // read the counter & the counter flags to check that they are 0 after the L1Reset
    int counter_flags_read = ResultRegisterData( LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTERS_FLAG) );
    int counter_read = ResultRegisterData( LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTER) );

    // compare data bus from result register to test value written to DataBus
    ok &= CompareValues(out(), "L1Reset counter flags", counter_flags_read, 0, true);
    ok &= CompareValues(out(), "L1Reset counter", counter_read, 0, true);
    
    // read back DataBus from result register
    uint32_t rr_read = LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_DATA_BUS);
    int command_code = ResultRegisterCommand(rr_read);
    uint32_t data_bus_read = ResultRegisterData(rr_read);
    
    bool check = ((data_bus_read == 0) && 
                  (command_code == CCB_COM_RR_LOAD_DATA_BUS) &&
                  (counter_flags_read == 0) &&
                  (counter_read == 0) );
    
    cout<< __func__ << " write/read " << ( check ? "OK ": "BAD ") << " = " << hex << 0 << " / " << data_bus_read
        << " Command code: " << command_code
        << " Counter flags: " << counter_flags_read
        << " Counter: " << dec << counter_read <<endl;
    
    // compare data bus from result register to test value written to DataBus
    ok &= CompareValues(out(), "L1Reset DataBus", data_bus_read & 0xFF, 0, true);
    ok &= CompareValues(out(), "L1Reset Command Code", command_code, CCB_COM_RR_LOAD_DATA_BUS, true);
    
    usleep(50);
  }
  
  int errcode = (ok == false);
  MessageOK(cout, __func__, errcode);
  return errcode;
}


int CCBBackplaneTester::TestTMBHardReset()
{
  bool ok = true;
  
  int Niter = 5;
  
  for(int i=0; i<Niter; ++i)
  {
    // write DataBus with some test value
    ccb_->WriteRegister(CCB_CSRB3_DATA_BUS, 0xFF);
    
    // do TMB hard reset
    ccb_->WriteRegister(CCB_VME_TMB_HARD_RESET, 1);
    
    // wait several sec
    sleep(3);
    
    // read counter & counter flags to check that they are 0
    int counter_flags_read = ResultRegisterData( LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTERS_FLAG) );
    int counter_read = ResultRegisterData( LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTER) );

    // compare data bus from result register to test value written to DataBus
    ok &= CompareValues(out(), "TMBHardReset counter flags", counter_flags_read, 0, true);
    ok &= CompareValues(out(), "TMBHardReset counter", counter_read, 0, true);
    
    // read back DataBus from result register
    uint32_t rr_read = LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_DATA_BUS);
    int command_code = ResultRegisterCommand(rr_read);
    uint32_t data_bus_read = ResultRegisterData(rr_read);
    
    // compare data bus from result register to test value written to DataBus
    ok &= CompareValues(out(), "TMBHardReset DataBus", data_bus_read & 0xFF, 0, true);
    ok &= CompareValues(out(), "TMBHardReset Command Code", command_code, CCB_COM_RR_LOAD_DATA_BUS, true);

    bool check = ((data_bus_read == 0) && 
                  (command_code == CCB_COM_RR_LOAD_DATA_BUS) &&
                  (counter_flags_read == 0) &&
                  (counter_read == 0) );
    
    cout<< __func__ << " write/read " << (check ? "OK " : "BAD ") << " = " << hex << 0 << " / " << data_bus_read
        << " Command code: " << command_code << " Counter flags: " << counter_flags_read << " Counter: " << dec << counter_read << endl;
  }
  
  int errcode = (ok == false);
  MessageOK(cout, __func__, errcode);
  return errcode;
}

int CCBBackplaneTester::TestPulseCounters()
{
  bool ok = true;

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
    ok &= CompareValues(out(), "PulseCounters", counter_flags_read, 0, true);

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

    cout<< __func__ << " flags write/read " << (counter_flags_read == counter_flag ? "OK " : "BAD ") << counter_flag << " " << counter_flags_read << endl;

    // fail the test if not equal
    ok &= CompareValues(out(), "PulseCounters", counter_flags_read, counter_flag, true, false);


    // read the total pulse counter out of the TMB RR:
    uint32_t counter_read = ResultRegisterData( LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTER) );

    // compare to the expected numbers
    if (is25nsPulseCommand(command))
    {
      ok &= CompareValues(out(), "PulseCounters", counter_read, Niter, true, false);
    }
    else if (is500nsPulseCommand(command))
    {
      ok &= CompareValues(out(), "PulseCounters", (float)counter_read/Niter, 22., .1, false);
    }
    else if (command == CCB_VME_ALCT_ADB_PULSE_ASYNC)
    {
      ok &= CompareValues(out(), "PulseCounters", (float)counter_read/Niter, 15., .1, false);
    }

    cout << __func__ << " counter read " << counter_read << "  average = " << (float)counter_read / Niter << endl << endl;

    // issue L1Reset to reset the counters
    L1Reset();
  }

  int errcode = (ok == false);
  MessageOK(cout, __func__, errcode);
  return errcode;
}


int CCBBackplaneTester::TestCommandBus()
{
  bool ok = true;

  int Niter = 100;

  // a set of 8-bit patterns that Jason have found to have good coverage
  uint32_t reg[] = {0x05, 0x0A, 0x14, 0x28, 0x51, 0xA2, 0x60, 0x80};

  // walk over the range of values of the CSRB2 register
  for (int j = 0; j < 8; ++j)
  {
    cout << endl << __func__ << " testing CSRB2 " << hex << reg[j] << endl << endl;

    for (int i = 0; i < Niter; ++i)
    {
      // load result register with the command code, read it back, and extract the command field
      uint32_t command_code = ResultRegisterCommand( LoadAndReadResutRegister(ccb_, tmb_->slot(), reg[j]) );

      cout << __func__ << " write/read " << (command_code == reg[j] ? "OK " : "BAD ") << " = " << reg[j] << " / " << command_code << endl;

      // compare the read out command code to the value written into the CSRB2 register
      ok &= CompareValues(out(), "CommandBus", command_code, reg[j], true);
    }

    // issue L1Reset to reset the counters
    L1Reset();
  }

  int errcode = (ok == false);
  MessageOK(cout, __func__, errcode);
  return errcode;
}


int CCBBackplaneTester::TestCCBReserved()
{
  bool ok = true;

  RR0Bits rr0;

  //  CCB_reserved0 - trivial read-only test
  rr0.r = LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR0);
  cout << " CCB_reserved0: read bit value " << rr0.CCB_reserved0 <<endl;
  // reset the counters
  L1Reset();


  //  CCB_reserved1 - trivial read-only test
  rr0.r = LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR0);
  cout << " CCB_reserved1: read bit value " << rr0.CCB_reserved1 <<endl;
  // reset the counters
  L1Reset();


  // ----- CCB_reserved2 & CCB_reserved3

  int Niter = 50;

  // read in  CSRB6 register
  CSRB6Bits csrb6;
  csrb6.r = ccb_->ReadRegister(CCB_CSRB6);

  // odd iteration number: activate CCB_reserved2 & CCB_reserved3 bits
  // even iteration number: de-activate CCB_reserved2 & CCB_reserved3 bits
  for (int i = 0; i < Niter; ++i)
  {
    if (i % 2) // de-activate
    {
      csrb6.CCB_reserved2 = 0;
      csrb6.CCB_reserved3 = 0;
    }
    else // activate
    {
      csrb6.CCB_reserved2 = 1;
      csrb6.CCB_reserved3 = 1;
    }
    ccb_->WriteRegister(CCB_CSRB6, csrb6.r);

    // give it a break
    usleep(50);

    // read RR0 bits from TMB
    rr0.r = LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR0);

    // compare the read out bit to the written value
    ok &= CompareValues(out(), "CCBReserved2", rr0.CCB_reserved2, csrb6.CCB_reserved2, true);
    ok &= CompareValues(out(), "CCBReserved3", rr0.CCB_reserved3, csrb6.CCB_reserved3, true);

    cout << " CCB_reserved2 & 3: written " << csrb6.CCB_reserved2 << " " << csrb6.CCB_reserved3
        << "  read " << rr0.CCB_reserved2 << " " << rr0.CCB_reserved3 <<endl;

    // issue L1Reset to reset the counters
    L1Reset();
  }


  // TODO: loopback board related tests for CCB_reserved2 & CCB_reserved3


  int errcode = (ok == false);
  MessageOK(cout, __func__, errcode);
  return errcode;
}


int CCBBackplaneTester::TestTMBReservedOut()
{
  bool ok = true;

  int Niter = 20;

  // read the current value of CSRB6
  CSRB6Bits csrb6;
  csrb6.r = ccb_->ReadRegister(CCB_CSRB6);

  // walk over the range of values of 3 bits of TMB_reserved_out (not including 111)
  for (uint32_t pattern = 0; pattern < 7; ++pattern)
  {
    cout << endl << __func__ << " testing TMB_reserved_out " << hex << pattern << endl << endl;
    
    // change the value of the TMB_reserved_out bits
    csrb6.TMB_reserved_out = pattern;

    for (int i=0; i<Niter; ++i)
    {
      // load CSRB6 with the test value
      ccb_->WriteRegister(CCB_CSRB6, csrb6.r);

      // read back from result register
      RR0Bits rr;
      rr.r = LoadAndReadResutRegister(ccb_, tmb_->slot(), 0);

      cout<< __func__ << " write/read " << (pattern == rr.TMB_reserved_out ? "OK " : "BAD ")
          << " = " << pattern  << " / " << rr.TMB_reserved_out << endl;

      // compare the the result from written value
      ok &= CompareValues(out(), "TMBReservedOut", rr.TMB_reserved_out, pattern, true);

      // issue L1Reset after each iteration
      L1Reset();
    }
  }

  int errcode = (ok == false);
  MessageOK(cout, __func__, errcode);
  return errcode;
}


int CCBBackplaneTester::TestDMBReservedOut()
{
  bool ok = true;

  int Niter = 100;

  // read the current value of CSRB6
  CSRB6Bits csrb6;
  csrb6.r = ccb_->ReadRegister(CCB_CSRB6);

  // walk over the range of values of 5 bits of TMB_reserved_out, not including 11111
  for (uint32_t pattern = 0; pattern < 31; ++pattern)
  {
    cout << endl << __func__ << " testing DMB_reserved_out " << hex << pattern << endl << endl;
    
    // change the value of the DMB_reserved_out bits
    csrb6.DMB_reserved_out = pattern;

    for (int i=0; i<Niter; ++i)
    {
      // load CSRB6 with the test value
      ccb_->WriteRegister(CCB_CSRB6, csrb6.r);
      
      // read back from CSRB11
      CSRB11Bits csrb11;
      csrb11.r = ccb_->ReadRegister(CCB_CSRB11);

      cout<< __func__ <<" write/read "<< (pattern == csrb11.TMB_reserved_in? "OK ": "BAD ")
          << " = " << pattern << " / " << csrb11.TMB_reserved_in <<endl;

      // compare the the result from written value
      ok &= CompareValues(out(), "DMBReservedOut", csrb11.TMB_reserved_in, pattern, true);

      // issue L1Reset after each iteration
      L1Reset();
    }
  }

  int errcode = (ok == false);
  MessageOK(cout, __func__, errcode);
  return errcode;
}


int CCBBackplaneTester::TestDataBus()
{
  bool ok = true;
  
  int Niter = 20;
  
  // a set of 8-bit patterns that Jason have found to have good coverage
  uint32_t reg[] = {0x05, 0x0A, 0x14, 0x28, 0x51, 0xA2, 0x60, 0x80};
  
  // walk over the range of values of the DataBus (CSRB2)
  for (int j = 0; j < 8; ++j)
  {
    for (int i = 0; i < Niter; ++i)
    {
      // write DataBus with test value
      ccb_->WriteRegister(CCB_CSRB3_DATA_BUS, reg[j]);
      
      // read back DataBus from result register
      uint32_t data_bus_read = ResultRegisterData( LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_DATA_BUS) );
      
      cout<< __func__ <<" write/read "<< (data_bus_read == reg[j]? "OK ": "BAD ")
          << " = " << hex << reg[j] << " / " << data_bus_read << dec <<endl;
      
      // compare data bus from result register to test value written to DataBus
      ok &= CompareValues(out(), "DataBus", data_bus_read & 0xFF, reg[j], true);

      // check that the four clock_status bits are zero
      ok &= CompareValues(out(), "DataBus clock_status", (data_bus_read >> 8) & 0x0F, 0, true);
    }
  }
  
  int errcode = (ok == false);
  MessageOK(cout, __func__, errcode);
  return errcode;
}


int CCBBackplaneTester::TestCCBClock40()
{
  bool ok = true;

  int Niter = 250;

  double tolerance = .05;
  
  int repeat_count = 0;

  uint32_t prev_clock_val = ResultRegisterData( LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_CCB_RX0) );
  for(int i=1; i<Niter; ++i)
  {
    cout << " clock_val " << prev_clock_val << endl;
    
    // load ccb_clock40_enable or ccb_rx0 value into RR
    uint32_t clock_val = ResultRegisterData( LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_CCB_RX0) );
    
    if (clock_val == prev_clock_val)
    {
      cout << i << " repeat value" << endl;
      ++repeat_count;
    }
    prev_clock_val = clock_val;
  }
  
  ok = CompareValues(out(), "CCBClock40", (float)(Niter - repeat_count), (float)Niter, tolerance, false);
  
  cout<< __func__ << " iterations/ unique values " << (ok ? "OK " : "BAD ")
      << " = " << dec << Niter << " / " << (Niter - repeat_count) << endl;
  
  int errcode = (ok == false);
  MessageOK(cout, __func__, errcode);
  return errcode;
}


}} // namespaces
