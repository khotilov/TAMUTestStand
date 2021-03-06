/*
 * $Id: $
 */

// class header
#include "emu/pc/TestWorkerBase.h"

// Emu includes
#include "emu/pc/TMB.h"
#include "emu/pc/CCB.h"
#include "emu/pc/CCBCommands.h"
#include "emu/pc/TestUtils.h"
#include "emu/utils/SimpleTimer.h"
#include "emu/exception/Exception.h"

// system includes
#include <iostream>
#include <sstream>
#include <unistd.h> // for sleep()
#include <boost/bind.hpp>


namespace emu { namespace pc {

using std::endl;
using std::cout;
using std::string;


TestWorkerBase::TestWorkerBase()
: ccb_(0)
, tmb_(0)
, out_(&cout)
, log_()
{}


TestWorkerBase::~TestWorkerBase() {}


void TestWorkerBase::RedirectOutput(std::ostream * output)
{
  out_ = output ;
}


void TestWorkerBase::RegisterTheTest(const std::string &test, TestProcedure proc)
{
  if (testProcedures_.find(test) != testProcedures_.end())
  {
    // test was registered before, only need to re-set its procedure binding
    cout << __func__ << ": re-setting test procedure with label " << test << endl;
  }
  else
  {
    // test was not registered before, do the full registration
    cout << __func__ << " with label " << test << endl;
    testLabels_.push_back(test);
    testResults_[test] = -1;
  }

  testProcedures_[test] = proc;
}


std::vector<std::string> TestWorkerBase::GetTestLabels() const
{
  return testLabels_;
}


int TestWorkerBase::NTests() const
{
  return testResults_.size();
}


int TestWorkerBase::NTestsPass() const
{
  int n = 0;
  for (std::map<string, int>::const_iterator it = testResults_.begin(); it != testResults_.end(); ++it)
  {
    if (it->second == 0) ++n;
  }
  return n;
}


int TestWorkerBase::NTestsFail() const
{
  int n = 0;
  for (std::map<string, int>::const_iterator it = testResults_.begin(); it != testResults_.end(); ++it)
  {
    if (it->second > 0) ++n;
  }
  return n;
}


int TestWorkerBase::GetTestResult(const std::string &test)
{
  if (testResults_.find(test) == testResults_.end())
  {
    cout << __func__ << ": WARNING! test with label " << test << " was not registered. Returning -1" << endl;
    return -1;
  }
  return testResults_[test];
}


void TestWorkerBase::SetTestResult(const std::string &test, int result)
{
  if (testResults_.find(test) == testResults_.end())
  {
    cout << __func__ << ": WARNING! test with label " << test << " was not registered. Do nothing." << endl;
    return;
  }
  testResults_[test] = result;
}


int TestWorkerBase::GetTestStatus(const std::string &test)
{
  if (testStatuses_.find(test) == testStatuses_.end())
  {
    cout << __func__ << ": WARNING! test with label " << test << " was not registered. Returning -1" << endl;
    return -1;
  }
  return testStatuses_[test];
}


void TestWorkerBase::SetTestStatus(const std::string &test, int status)
{
  testStatuses_[test] = status;
}

void TestWorkerBase::ReportError(int error)
{
  log_.reportError(error);
}

void TestWorkerBase::SetBoardLabel(std::string board)
{
  log_.setBoard(board);
}

int TestWorkerBase::RunTest(const std::string &test)
{
  emu::utils::SimpleTimer timer;

  int result = 0;

  // check if ccb exists
  /*if(!ccb_->exist())
  {
    cout << __func__ << ": WARNING! CCB does not exist! Crate may be disconnected or powered down!" << endl;
    out() << "WARNING! CCB does not exist! Crate may be disconnected or powered down!" << endl;
    return -1;
  }*/

  // first, special case of running all tests:
  if (test == "All")
  {
    //HardReset(); // WARNING: doing hard reset makes the 1st CommandBus test fail... why???

    // run All test in the order they were registered:
    for (std::vector<string>::iterator itest = testLabels_.begin(); itest != testLabels_.end(); ++itest)
    {
      //if (*itest == "Dummy") continue;
      if (*itest == "TestLEDFrontPanel") continue;

      // run the test, if any test fails (>0), all fail
      result |= RunTest(*itest);

      // give it a little break before the next test
      usleep(50);
    }

    out() << "------------------------------" << endl;
    MessageOK(out(), "Status of All tests ... ", result);
  }
  else if (testProcedures_.find(test) == testProcedures_.end()) // make sure that the test label was registerd
  {
    std::stringstream s;
    s << __func__ << ": WARNING! test with label " << test << " was not registered!" << endl;
    out() << s.str();
    cout << s.str();
    return 0;
  }
  else
  {
    out() << "Test with label " << test << " ... start" << endl;
    log_.startTest(test);
    if (test != "Dummy")
    {
      PrepareHWForTest();
    }
    // run the test
    result = testProcedures_[test]();
    log_.endTest(result);
    testResults_[test] = result;
    MessageOK(out(), "Test with label " + test + " status ... ", result);
  }
  
  cout << "Time: " << timer.sec() << "sec for test with label " << test << endl;
  return result;
}

void TestWorkerBase::SetTester(std::string tester)
{
  log_.setTester(tester);
}


void TestWorkerBase::HardReset()
{
  out() << "TestWorkerBase: Hard reset through CCB" << endl;
  if (ccb_)
  {
    ccb_->hardReset();
    usleep(5000);
  }
  else
  {
    out() << __PRETTY_FUNCTION__ << " No CCB defined!" << endl;
    XCEPT_RAISE(emu::exception::HardwareException, "zero pointer to CCB");
  }
}


std::string TestWorkerBase::TestLabelFromProcedureName(const std::string& proc)
{
  // the current convention is that test procedure names start with "Test",
  // and the rest of the name is a test label
  if (proc.substr(0,4) == "Test")
  {
    return proc.substr(4);
  }
  else
  {
    cout << __func__ << " !!!!! WARNING !!!! test procedure name doesn't start with Test: " << proc <<endl;
    return proc;
  }
}


void TestWorkerBase::L1Reset()
{
  if ( ccb_ )
  {
    ccb_->WriteRegister(CCB_VME_L1RESET, 1);
  }
  else
  {
    out() << __PRETTY_FUNCTION__ << " No CCB defined!" << endl;
    XCEPT_RAISE(emu::exception::HardwareException, "zero pointer to CCB");
  }
}


void TestWorkerBase::PrepareHWForTest()
{
  if (ccb_)
  {
    // make sure CCB is in FPGA mode
    SetFPGAMode(ccb_);

    // issue L1Reset to reset the counters
    //L1Reset(); // Tests should handle resets by themselves especially firmware related tests
  }
  else
  {
    out() << __PRETTY_FUNCTION__ << " No CCB defined!" << endl;
    XCEPT_RAISE(emu::exception::HardwareException, "zero pointer to CCB");
  }
}


}} // namespaces
