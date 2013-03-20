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
using std::boolalpha;
using std::map;


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
  RegisterTheTest("DMBReservedInLoopback", boost::bind( &CCBBackplaneTester::TestDMBReservedInLoopback, this));
  RegisterTheTest("DMBL1AReleaseLoopback", boost::bind( &CCBBackplaneTester::TestDMBL1AReleaseLoopback, this));
  RegisterTheTest("TestLEDFrontPanel", boost::bind( &CCBBackplaneTester::TestLEDFrontPanel, this));
  RegisterTheTest("Dummy", boost::bind( &CCBBackplaneTester::TestDummy, this));

  SetTestStatus("TestLEDFrontPanel", -1);
}


////////////////////////////////////////////////////
// Actual tests:
////////////////////////////////////////////////////

int CCBBackplaneTester::TestL1Reset()
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;
  int Niter = 100;
  
  for(int i=0; i<Niter; ++i)
  {
    // write DataBus with some test value
    ccb_->WriteRegister(CCB_CSRB3_DATA_BUS, 0xFF);
    
    // do the L1 reset
    L1Reset();

    // read the counter & the counter flags to check that they are 0 after the L1Reset
    int counter_flags_read = ResultRegisterData( LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTERS_FLAG) );
    int counter_read = ResultRegisterData( LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_PULSE_COUNTER) );

    // compare data bus from result register to test value written to DataBus
    ok &= CompareValues(out(), test + " counter flags", counter_flags_read, 0, true);
    ok &= CompareValues(out(), test + " counter", counter_read, 0, true);
    
    // read back DataBus from result register
    uint32_t rr_read = LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_DATA_BUS);
    int command_code = ResultRegisterCommand(rr_read);
    uint32_t data_bus_read = ResultRegisterData(rr_read);
    
    bool check = ((data_bus_read == 0) && 
                  (command_code == CCB_COM_RR_LOAD_DATA_BUS) &&
                  (counter_flags_read == 0) &&
                  (counter_read == 0) );
    
    cout<< test + " write/read " << ( check ? "OK ": "BAD ") << " = " << hex << 0 << " / " << data_bus_read
        << " Command code: " << command_code
        << " Counter flags: " << counter_flags_read
        << " Counter: " << dec << counter_read <<endl;
    
    // compare data bus from result register to test value written to DataBus
    ok &= CompareValues(out(), test + " DataBus", data_bus_read & 0xFF, 0, true);
    ok &= CompareValues(out(), test + " Command Code", command_code, CCB_COM_RR_LOAD_DATA_BUS, true);
    
    usleep(50);
  }
  
  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}


int CCBBackplaneTester::TestTMBHardReset()
{
  string test = TestLabelFromProcedureName(__func__);
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
    int counter_flags_read = ResultRegisterData( LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTERS_FLAG) );
    int counter_read = ResultRegisterData( LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_PULSE_COUNTER) );

    // compare data bus from result register to test value written to DataBus
    ok &= CompareValues(out(), test + " counter flags", counter_flags_read, 0, true);
    ok &= CompareValues(out(), test + " counter", counter_read, 0, true);
    
    // read back DataBus from result register
    uint32_t rr_read = LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_DATA_BUS);
    int command_code = ResultRegisterCommand(rr_read);
    uint32_t data_bus_read = ResultRegisterData(rr_read);
    
    // compare data bus from result register to test value written to DataBus
    ok &= CompareValues(out(), test + " DataBus", data_bus_read & 0xFF, 0, true);
    ok &= CompareValues(out(), test + " Command Code", command_code, CCB_COM_RR_LOAD_DATA_BUS, true);

    bool check = ((data_bus_read == 0) && 
                  (command_code == CCB_COM_RR_LOAD_DATA_BUS) &&
                  (counter_flags_read == 0) &&
                  (counter_read == 0) );
    
    cout<< test + " write/read " << (check ? "OK " : "BAD ") << " = " << hex << 0 << " / " << data_bus_read
        << " Command code: " << command_code << " Counter flags: " << counter_flags_read << " Counter: " << dec << counter_read << endl;
  }
  
  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}

int CCBBackplaneTester::TestPulseCounters()
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;
  int Niter = 100;

  // walk through the pulse counter flags bits
  for (int ibit = 0; ibit < LENGTH_PULSE_IN_COMMANDS; ++ibit)
  {
    const int command = PULSE_IN_COMMANDS[ibit];

    int counter_flag = (1 << ibit);

    cout << test + " command & flag " << hex << command << " " << counter_flag << dec << endl;

    // read pulse counter flags from TMB RR:
    int counter_flags_read = LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTERS_FLAG);
    counter_flags_read = ResultRegisterData(counter_flags_read);

    // make sure counter flags are off!!!
    ok &= CompareValues(out(), test + " init", counter_flags_read, 0, true);

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
    counter_flags_read = LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTERS_FLAG);
    counter_flags_read = ResultRegisterData(counter_flags_read);

    cout<< test + " flags write/read " << (counter_flags_read == counter_flag ? "OK " : "BAD ") << counter_flag << " " << counter_flags_read << endl;

    // fail the test if not equal
    ok &= CompareValues(out(), test + " flag", counter_flags_read, counter_flag, true, false);


    // read the total pulse counter out of the TMB RR:
    uint32_t counter_read = ResultRegisterData( LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_PULSE_COUNTER) );

    // compare to the expected numbers
    if (is25nsPulseCommand(command))
    {
      ok &= CompareValues(out(), test + " counter", counter_read, Niter, true, false);
    }
    else if (is500nsPulseCommand(command))
    {
      ok &= CompareValues(out(), test + " counter", (float)counter_read/Niter, 22., .1, false);
    }
    else if (command == CCB_VME_ALCT_ADB_PULSE_ASYNC)
    {
      ok &= CompareValues(out(), test + " counter", (float)counter_read/Niter, 15., .1, false);
    }

    cout << test + " counter read " << counter_read << "  average = " << (float)counter_read / Niter << endl << endl;

    // issue L1Reset to reset the counters
    L1Reset();
  }

  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}


int CCBBackplaneTester::TestCommandBus()
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;
  const int Niter = 100;

  // a set of 8-bit patterns that Jason have found to have good coverage
  uint32_t reg[] = {0x05, 0x0A, 0x14, 0x28, 0x51, 0xA2, 0x60, 0x80};

  // walk over the range of values of the CSRB2 register
  for (int j = 0; j < 8; ++j)
  {
    cout << endl << test + " testing CSRB2 " << hex << reg[j] << endl << endl;

    for (int i = 0; i < Niter; ++i)
    {
      // load result register with the command code, read it back, and extract the command field
      uint32_t command_code = ResultRegisterCommand( LoadAndReadResultRegister(ccb_, tmb_->slot(), reg[j]) );

      cout << test + " write/read " << (command_code == reg[j] ? "OK " : "BAD ") << " = " << reg[j] << " / " << command_code << endl;

      // compare the read out command code to the value written into the CSRB2 register
      ok &= CompareValues(out(), test, command_code, reg[j], true);
    }

    // issue L1Reset to reset the counters
    L1Reset();
  }

  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}


int CCBBackplaneTester::TestCCBReserved()
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;
  RR0Bits rr0;

  //  CCB_reserved0 - trivial read-only test
  rr0.r = LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR0);
  cout << " CCB_reserved0: read bit value " << rr0.CCB_reserved0 <<endl;
  // reset the counters
  L1Reset();


  //  CCB_reserved1 - trivial read-only test
  rr0.r = LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR0);
  cout << " CCB_reserved1: read bit value " << rr0.CCB_reserved1 <<endl;
  // reset the counters
  L1Reset();


  // ----- CCB_reserved2 & CCB_reserved3

  const int Niter = 50;

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
    rr0.r = LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR0);

    // compare the read out bit to the written value
    ok &= CompareValues(out(), "CCBReserved2", rr0.CCB_reserved2, csrb6.CCB_reserved2, true);
    ok &= CompareValues(out(), "CCBReserved3", rr0.CCB_reserved3, csrb6.CCB_reserved3, true);

    uint32_t CCB_reserved2_via_DMBloop = (rr0.DMB_reserved_in & (0x1<<2))>>2; // DMB_reserved_in is 3 bits long, we want the highest bit
    uint32_t CCB_reserved3_via_DMBloop = rr0.DMB_L1A_release;

    // compare the read out bit to the written value via DMB_loopback
    ok &= CompareValues(out(), "CCBReserved2", CCB_reserved2_via_DMBloop, csrb6.CCB_reserved2, true);
    ok &= CompareValues(out(), "CCBReserved3", CCB_reserved3_via_DMBloop, csrb6.CCB_reserved3, true);

    cout << " CCB_reserved2 & 3: written " << csrb6.CCB_reserved2 << " " << csrb6.CCB_reserved3
        << "  read " << rr0.CCB_reserved2 << " " << rr0.CCB_reserved3 <<endl;

    // issue L1Reset to reset the counters
    L1Reset();
  }

  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}


int CCBBackplaneTester::TestTMBReservedOut()
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;
  const int Niter = 20;

  // read the current value of CSRB6
  CSRB6Bits csrb6;
  csrb6.r = ccb_->ReadRegister(CCB_CSRB6);

  // walk over the range of values of 3 bits of TMB_reserved_out (not including 111)
  for (uint32_t pattern = 0; pattern < 7; ++pattern)
  {
    cout << endl << test + " testing pattern " << hex << pattern << endl << endl;
    
    // change the value of the TMB_reserved_out bits
    csrb6.TMB_reserved_out = pattern;

    for (int i=0; i<Niter; ++i)
    {
      // load CSRB6 with the test value
      ccb_->WriteRegister(CCB_CSRB6, csrb6.r);

      // read back from result register
      RR0Bits rr;
      rr.r = LoadAndReadResultRegister(ccb_, tmb_->slot(), 0);

      cout<< test + " write/read " << (pattern == rr.TMB_reserved_out ? "OK " : "BAD ")
          << " = " << pattern  << " / " << rr.TMB_reserved_out << endl;

      // compare the the result from written value
      ok &= CompareValues(out(), test, rr.TMB_reserved_out, pattern, true);

      // issue L1Reset after each iteration
      L1Reset();
    }
  }

  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}


int CCBBackplaneTester::TestDMBReservedOut()
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;
  const int Niter = 100;

  // read the current value of CSRB6
  CSRB6Bits csrb6;
  csrb6.r = ccb_->ReadRegister(CCB_CSRB6);

  // walk over the range of values of 5 bits of TMB_reserved_out, not including 11111
  for (uint32_t pattern = 0; pattern < 31; ++pattern)
  {
    cout << endl << test + " testing pattern " << hex << pattern << endl << endl;
    
    // change the value of the DMB_reserved_out bits
    csrb6.DMB_reserved_out = pattern;

    for (int i=0; i<Niter; ++i)
    {
      // load CSRB6 with the test value
      ccb_->WriteRegister(CCB_CSRB6, csrb6.r);
      
      // read back from CSRB11
      CSRB11Bits csrb11;
      csrb11.r = ccb_->ReadRegister(CCB_CSRB11);

      cout<< test + " write/read " << (pattern == csrb11.TMB_reserved_in? "OK ": "BAD ")
          << " = " << pattern << " / " << csrb11.TMB_reserved_in << endl;

      // compare the the result from written value
      ok &= CompareValues(out(), test, csrb11.TMB_reserved_in, pattern, true);

      // issue L1Reset after each iteration
      L1Reset();
    }
  }

  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}


int CCBBackplaneTester::TestDataBus()
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;
  const int Niter = 20;
  
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
      uint32_t data_bus_read = ResultRegisterData( LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_DATA_BUS) );
      
      cout<< test + " write/read "<< (data_bus_read == reg[j]? "OK ": "BAD ")
          << " = " << hex << reg[j] << " / " << data_bus_read << dec <<endl;
      
      // compare data bus from result register to test value written to DataBus
      ok &= CompareValues(out(), test + " data", data_bus_read & 0xFF, reg[j], true);

      // check that the four clock_status bits are zero
      ok &= CompareValues(out(), test + " clock_status", (data_bus_read >> 8) & 0x0F, 0, true);
    }
  }
  
  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}


int CCBBackplaneTester::TestCCBClock40()
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;
  
  const int Niter = 250;
  const double tolerance = .05;

  int repeat_count = 0;

  uint32_t prev_clock_val = ResultRegisterData( LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_CCB_RX0) );
  for(int i=1; i<Niter; ++i)
  {
    cout << " clock_val " << prev_clock_val << endl;
    
    // load ccb_clock40_enable or ccb_rx0 value into RR
    uint32_t clock_val = ResultRegisterData( LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_CCB_RX0) );
    
    if (clock_val == prev_clock_val)
    {
      cout << i << " repeat value" << endl;
      ++repeat_count;
    }
    prev_clock_val = clock_val;
  }
  
  ok = CompareValues(out(), test, (float)(Niter - repeat_count), (float)Niter, tolerance, false);
  
  cout<< test + " iterations/ unique values " << (ok ? "OK " : "BAD ")
      << " = " << dec << Niter << " / " << (Niter - repeat_count) << endl;
  
  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}


int CCBBackplaneTester::TestDMBReservedInLoopback()
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;
  const int Niter = 20;

  // read the current value of CSRB6
  CSRB6Bits csrb6;
  //csrb6.r = ccb_->ReadRegister(CCB_CSRB6);
  csrb6.r = 0;

  // walk over the range of values of possible combinations of 3 bits
  for (uint32_t pattern = 0; pattern < 8; ++pattern)
  {
    // reverse bits 0 & 1 to write:
    uint32_t write_pattern =  ( (pattern & 0x4) | ((pattern & 0x2)>>1) | ((pattern & 0x1)<<1) );
    cout << endl << test + " testing pattern " << hex << pattern <<" -> "<< write_pattern << endl << endl;

    // change the value of the DMB_reserved_in bit 2
    csrb6.CCB_reserved2 = ((write_pattern & 0x4) >> 2);
    cout << " CCB_reserved2 " << csrb6.CCB_reserved2 <<endl;

    // set bits [4:3] of DMB_reserved_in to zero
    csrb6.DMB_reserved_out &= 0x7;
    cout << " DMB_reserved_out " << csrb6.DMB_reserved_out <<endl;

    // set bits [4:3] of DMB_reserved_in to bits [0:1] of pattern
    csrb6.DMB_reserved_out |= ((write_pattern & 0x3) << 3);
    cout << " DMB_reserved_out " << csrb6.DMB_reserved_out <<endl;

    for (int i=0; i<Niter; ++i)
    {
      // load CSRB6 with the test value
      ccb_->WriteRegister(CCB_CSRB6, csrb6.r);

      // read rr0 bits
      RR0Bits rr0;
      rr0.r = LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR0);

      //cout << " DMB_reserved_in: read " << rr0.DMB_reserved_in <<endl;
      // swap DMB_reserved_in bits 0 and 1 for comparison to pattern
      //rr0.DMB_reserved_in = ((rr0.DMB_reserved_in & 0x4) | ((rr0.DMB_reserved_in & 0x2)>>1) | ((rr0.DMB_reserved_in & 0x1)<<1));
      cout << " DMB_reserved_in: read bit value " << rr0.DMB_reserved_in <<endl;
      cout << " ccb_reserved2: read bit value " << rr0.CCB_reserved2 <<endl;

      // compare the result from written value
      ok &= CompareValues(out(), test + " pattern", rr0.DMB_reserved_in, pattern, true);

      CSRB11Bits csrb11;
      csrb11.r = ccb_->ReadRegister(CCB_CSRB11);
      cout << " CSRB11.DMB_reserved_in: " << csrb11.DMB_reserved_in <<endl;

      // compare to the result from CSRB11
      ok &= CompareValues(out(), test + " CSRB11", rr0.DMB_reserved_in, csrb11.DMB_reserved_in, true);

      // issue L1Reset after each iteration
      L1Reset();
    }
  }

  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}

int CCBBackplaneTester::TestDMBL1AReleaseLoopback()
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;
  const int Niter = 20;

  // read the current value of CSRB6
  CSRB6Bits csrb6;
  //csrb6.r = ccb_->ReadRegister(CCB_CSRB6);
  csrb6.r = 0;

  // walk over the range of values of possible combinations of 1 bits
  for (uint32_t pattern = 0; pattern < 2; ++pattern)
  {
    cout << endl << test + " testing pattern " << hex << pattern << endl << endl;

    // change the value of CSRB6[0]
    csrb6.CCB_reserved3 = pattern;

    for (int i=0; i<Niter; ++i)
    {
      // load CSRB6 with the test value
      ccb_->WriteRegister(CCB_CSRB6, csrb6.r);

      // read rr0 bits
      RR0Bits rr0;
      rr0.r = LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR0);

      cout << " DMB_L1A_release: wrote/read bit value  " << csrb6.CCB_reserved3 << " / " << rr0.DMB_L1A_release << dec << endl;

      // compare the the result from written value
      ok &= CompareValues(out(), test + " DMB_L1A_release", rr0.DMB_L1A_release, pattern, true);

      // issue L1Reset after each iteration
      L1Reset();
    }
  }

  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}

int CCBBackplaneTester::TestLEDFrontPanel()
{
  string test = TestLabelFromProcedureName(__func__);
  // Result is updated in CCBBackplaneTesterModule
  int result = GetTestResult("TestLEDFrontPanel");

  CompareValues(out(), test + " LED_Errors", 0, result, true);
  /*//Log test results
  if(result)
  {
    out() << "The following LEDs did not display correctly: ";
    cout << "LED ERRORS: ";
    for(int i=0; i<LENGTH_PULSE_COUNTER; ++i)
    {
      if(result & (0x1<<i))
      {
        out() << i << " ";
        cout << '1';
      }
      else
        cout << '0';
    }
    out() << endl;
    cout << endl;
  }*/

  int errcode = result;
  MessageOK(cout, test + " result", errcode);
  return errcode;
}

}} // namespaces
