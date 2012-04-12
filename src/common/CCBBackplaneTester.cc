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
#include <stdio.h>
#include <string>
#include <unistd.h> // for sleep()
#include <stdlib.h>
#include <boost/bind.hpp>


#ifndef debugV     //silent mode
  #define PRINT(x)
  #define PRINTSTRING(x)
#else              //verbose mode
  #define PRINT(x) std::cout << #x << ":\t " << x << std::endl;
  #define PRINTSTRING(x) std::cout << #x << std::endl;
#endif



namespace emu { namespace pc {

using std::endl;
using std::cout;
using std::string;
using std::hex;
using std::dec;


CCBBackplaneTester::CCBBackplaneTester()
: ccb_(0)
, tmb_(0)
, out_(&cout)
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
  // use copy constructor to create a temporary that may potentially throw
  const CCBBackplaneTester temp(rhs);

  // safely "commit" the work to this object, using a non-throwing CopyFrom operation
  CopyFrom(temp);
  return *this;
}


void CCBBackplaneTester::CopyFrom(const CCBBackplaneTester &other)
{
  ccb_ = other.ccb_;
  tmb_ = other.tmb_;

  out_ = other.out_;

  // the real need for the user defined copy c-tor and assignment comes from here:
  // we need to make sure that the test procedures are properly bound to this object.
  RegisterTestProcedures();

  testResults_  = other.testResults_;
}


void CCBBackplaneTester::RegisterTestProcedures()
{
  RegisterTheTest("All", boost::bind( &CCBBackplaneTester::TestAll, this));
  RegisterTheTest("Dummy", boost::bind( &CCBBackplaneTester::TestDummy, this));
  RegisterTheTest("PulseCountersBits", boost::bind( &CCBBackplaneTester::TestPulseCountersBits, this));
  RegisterTheTest("CommandBus", boost::bind( &CCBBackplaneTester::TestCommandBus, this));
}


void CCBBackplaneTester::RegisterTheTest(const std::string &test, TestProcedure proc)
{
  if (testProcedures_.find(test) != testProcedures_.end())
  {
    cout << __func__ << ": WARNING: test with label " << test << " was already registered. It will be redefined." << endl;
  }

  testProcedures_[test] = proc;
  testResults_[test] = -1;

  cout << "Registered CCBBackplane test: " << test << endl;
}


void CCBBackplaneTester::Reset()
{
  (*out_) << "CCBBackplaneTester: Hard reset through CCB" << endl;
  if ( ccb_ )
  {
    ccb_->hardReset();
  }
  else
  {
    (*out_) << "No CCB defined!" << endl;
  }
}


std::vector<std::string> CCBBackplaneTester::GetTestLabels()
{
  std::vector<std::string> result;
  for (std::map<std::string, int>::iterator it = testResults_.begin(); it != testResults_.end(); ++it)
  {
    result.push_back(it->first);
  }
  return result;
}


int CCBBackplaneTester::GetTestResult(const std::string &test)
{
  if (testResults_.find(test) == testResults_.end()) return -1;
  return testResults_[test];
}


void CCBBackplaneTester::SetTestResult(const std::string &test, int result)
{
  testResults_[test] = result;
}


void CCBBackplaneTester::RunTest(const std::string &test)
{
  (*out_) << "CCBBackplaneTester: starting test "<< test << endl;

  bool test_result = true;
  if (testProcedures_.find(test) == testProcedures_.end())
  {
    cout<<__func__<<": WARNING: test with label "<<test<<" was not registered. Returning PASS."<<endl;
  }
  else
  {
    // make sure CCB is in FPGA mode
    if (test != "Dummy") SetFPGAMode(ccb_);

    // run the test
    bool test_result = testProcedures_[test]();
    testResults_[test] = test_result;
  }

  MessageOK(out_, "CCBBackplaneTester: " + test + " test result .... ", test_result);
}


bool CCBBackplaneTester::TestAll()
{
  (*out_) << "CCBBackplaneTester: Beginning full set of TMB self-tests" << endl;

  Reset();

  bool result = true;

  for (std::map<std::string, TestProcedure>::iterator iproc = testProcedures_.begin(); iproc != testProcedures_.end(); ++iproc)
  {
    string test = iproc->first;

    // run the test
    bool test_result = (iproc->second)();

    MessageOK(out_, test + "............. ", test_result);

    testResults_[test] = test_result;

    result &= test_result;

    // give it a little break before the next test
    usleep(50);
  }

  (*out_) << "------------------------------" << endl;
  MessageOK(out_, "CCBBackplaneTester: All tests result .... ", result);

  return result;
}


////////////////////////////////////////////////////
// Actual tests:
////////////////////////////////////////////////////

bool CCBBackplaneTester::TestPulseCountersBits()
{
  bool result = true;

  int Niter = 250;

  // walk through the pulse counter flags bits
  for (int ibit = 0; ibit < LENGTH_PULSE_IN_COMMANDS; ++ibit)
  {
    const int command = PULSE_IN_COMMANDS[ibit];

    int counter_flag = (1 << ibit);

    cout<<__func__<<" command & flag "<<hex<<command<< " "<<counter_flag<<dec<<endl;

    // reset

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
    int counter_flags_read = LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTERS_FLAG);
    counter_flags_read = ResultRegisterData(counter_flags_read);

    cout<<"__func__"<<" flags write/read "<<(counter_flags_read==counter_flag? "OK ": "BAD ")  << counter_flag<<" "<< counter_flags_read<<endl;

    // fail the test if not equal
    result &= CompareValues(out_, "PulseCountersBits", counter_flags_read, counter_flag, true);

    // issue L1Reset to reset the counters
    ccb_->WriteRegister(CCB_VME_L1RESET, 1);
  }

  cout<<__func__<<" result "<<result<<endl;

  MessageOK(out_, "CCBBackplaneTester: PulseCountersBits .... ", result);
  return result;
}


bool CCBBackplaneTester::TestCommandBus()
{
  bool result = true;

  int Niter = 5;

  // walk over the range of values of the CSRB2 register
  for (int reg = 0; reg < 256; ++reg)
  {
    // skip some reserved values
    int command_bus = reg & ~(0x03);
    if (command_bus == 0x10 || // hard reset
        command_bus == 0x3C || // hard reset CCB
        command_bus == 0x40 || // hard reset TMB
        command_bus == 0x0C || // L1Reset
        reg == 0xCC // dedicated custom command
    )
      continue;

    cout<<endl<<__func__<<" testing CSRB2 "<<hex<<reg<<endl<<endl;

    for (int i=0; i<Niter; ++i)
    {
      // load result register with the command code and read it back
      int rr = LoadAndReadResutRegister(ccb_, tmb_->slot(), reg);

      // extract command bits
      int command_code = ResutRegisterCommand(rr);

      cout<<__func__<<" write/read "<<(command_code==reg? "OK ": "BAD ")<<" = "<<reg<<" / "<<command_code<<endl;

      // compare the read out command code to the value written into the CSRB2 register
      result &= CompareValues(out_, "CommandBus", command_code, reg, true);
    }

    // issue L1Reset to reset the counters
    ccb_->WriteRegister(CCB_VME_L1RESET, 1);
  }

  cout<<__func__<<" result "<<result<<endl;

  MessageOK(out_, "CCBBackplaneTester: CommandBus .... ", result);
  return result;
}


}} // namespaces
