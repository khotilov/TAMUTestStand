/*
 * $Id: $
 */

// class header
#include "emu/pc/TestWorkerBase.h"

// Emu includes
#include "emu/pc/TestUtils.h"
#include "emu/utils/SimpleTimer.h"

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
: out_(&cout)
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


std::vector<std::string> TestWorkerBase::GetTestLabels()
{
  return testLabels_;
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
    cout << __func__ << ": WARNING! test with label " << test << " was not registered. Do noting." << endl;
    return;
  }
  testResults_[test] = result;
}


bool TestWorkerBase::RunTest(const std::string &test)
{
  emu::utils::SimpleTimer timer;

  bool result = true;

  // first, special case of running all tests:
  if (test == "All")
  {
    //Reset(); // WARNING: doing hard reset makes the 1st CommandBus test fail... why???

    // run All test in the order they were registered:
    for (std::vector<std::string>::iterator itest = testLabels_.begin(); itest != testLabels_.end(); ++itest)
    {
      //if (*itest == "Dummy") continue;

      // run the test
      result &= RunTest(*itest);

      // give it a little break before the next test
      usleep(50);
    }

    out() << "------------------------------" << endl;
    MessageOK(out(), "Status of All tests ... ", result);
  }
  // make sure that the test label was registerd
  else if (testProcedures_.find(test) == testProcedures_.end())
  {
    std::stringstream s;
    s << __func__<<": WARNING! test with label "<<test<<" was not registered!"<<endl;
    out() << s.str();
    cout << s.str();
    return 1;
  }
  else
  {
    out() << "Test with label "<< test << " ... start" << endl;

    if (test != "Dummy")
    {
      PrepareHWForTest();
    }

    // run the test
    result = testProcedures_[test]();
    testResults_[test] = result;

    MessageOK(out(), "Test with label " + test + " status ... ", result);
  }

  cout << "Time: " << timer.sec() << "sec for test with label " << test << endl;

  return result;
}


}} // namespaces
