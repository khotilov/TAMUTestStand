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
, out_(&std::cout)
{
  RegisterTest("All", boost::bind( &CCBBackplaneTester::TestAll, this));
  RegisterTest("Dummy", boost::bind( &CCBBackplaneTester::TestDummy, this));
  RegisterTest("PulseCountersBits", boost::bind( &CCBBackplaneTester::TestPulseCountersBits, this));

}


CCBBackplaneTester::~CCBBackplaneTester(){}



void CCBBackplaneTester::RegisterTest(const std::string &test, TestProcedure proc)
{
  if (testProcedures_.find(test) != testProcedures_.end())
  {
    cout<<__func__<<": WARNING: test with label "<<test<<" was already registered. It will be redefined."<<endl;
  }

  testProcedures_[test] = proc;
  testResults_[test] = -1;

  cout<<"Registerd CCBBackplane test "<<test<<endl;
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
    (*out_) << "No CCB defined" << endl;
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

  bool AllOK = true;

  for (std::map<std::string, TestProcedure>::iterator iproc = testProcedures_.begin(); iproc != testProcedures_.end(); ++iproc)
  {
    string test = iproc->first;

    // run the test
    bool test_result = (iproc->second)();

    MessageOK(out_, test + "............. ", test_result);

    testResults_[test] = test_result;

    AllOK &= test_result;

    // give it a little break
    usleep(50);
  }

  (*out_) << "------------------------------" << endl;
  MessageOK(out_, "CCBBackplaneTester: All tests result .... ", AllOK);

  return AllOK;
}


////////////////////////////////////////////////////
// Actual tests:
////////////////////////////////////////////////////

bool CCBBackplaneTester::TestPulseCountersBits()
{
  bool result = true;

  int Niter = 1000;

  // walk through the pulse counter flags bits
  for (int ibit = 0; ibit < LENGTH_PULSE_IN_COMMANDS; ibit++)
  {
    const int command = PULSE_IN_COMMANDS[ibit];

    int counter_bit = (1 << ibit);

    // reset

    // for 25ns pulse commands
    if (is25nsPulseCommand(command) || is500nsPulseCommand(command) )
    {
      // Niter times write anything (it doesn't matter what for pulse commands)
      NTimesWriteRegister(ccb_, Niter, command, 1);

      // read pulse counter flags from TMB RR:
      int counter_bits_read = LoadAndReadResutRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_COUNTERS_FLAG);

      // fail the test if not equal
      result &= CompareValues(out_, "PulseCountersBits", counter_bits_read, counter_bit, true);
    }
  }

  return result;
}


bool CCBBackplaneTester::TestCommandBus()
{
  bool result = true;

  return result;
}


}} // namespaces
