/*
 * $Id: $
 */

// class header
#include "emu/pc/TMBTestModule.h"

// Emu includes
#include "emu/pc/ConfigurablePCrates.h"
#include "emu/pc/Crate.h"
#include "emu/pc/Chamber.h"
#include "emu/pc/TMB.h"
#include "emu/pc/CCB.h"
#include "emu/pc/RAT.h"
#include "emu/utils/Cgi.h"
#include "emu/utils/System.h"

// XDAQ includes
#include "cgicc/Cgicc.h"
#include "cgicc/HTMLClasses.h"
#include "toolbox/string.h"
#include "xgi/Utils.h"
#include "xdaq/WebApplication.h"

// system includes
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unistd.h> // for sleep()


namespace emu {  namespace pc {

using std::endl;
using std::cout;
using std::string;


TMBTestModule::TMBTestModule(xdaq::WebApplication * app, ConfigurablePCrates * sys)
: app_(app)
, sys_(sys)
, tmbN_(0)
{
  // init with the "current" crate in the system, if it is set
  if (sys_->crateN() >= 0)
  {
    Init();
  }
}


void TMBTestModule::Init()
{
  if (sys_->crate() == 0)
  {
    cout<<__func__<<": current crate is not set"<<endl;
    return;
  }
  std::vector< TMB * > tmbs = sys_->crate()->tmbs();
  CCB * ccb = sys_->crate()->ccb();
  tests_.clear();

  for (size_t i = 0; i < tmbs.size(); i++)
  {
    TMBTester tmp;
    tmp.setTMB(tmbs[i]);
    tmp.setCCB(ccb);
    tmp.setRAT(tmbs[i]->getRAT());
    tests_.push_back(tmp);
  }
}


void TMBTestModule::TMBTestsPage(xgi::Input * in, xgi::Output * out )
{
  using cgicc::tr;
  using cgicc::form;
  using cgicc::input;

  cgicc::Cgicc cgi(in);

  int tmb;

  cgicc::form_iterator name = cgi.getElement("tmb");
  if(name != cgi.getElements().end())
  {
    tmb = cgi["tmb"]->getIntegerValue();
    cout << "TMBTestsPage: TMB " << tmb << endl;
    tmbN_ = tmb;
  }
  else
  {
    cout << "TMBTestsPage: No tmb index passed, using tmbN_="<<tmbN_ << endl;
    tmb = tmbN_;
  }

  TMB * thisTMB = sys_->tmbs()[tmb];
  Chamber * thisChamber = sys_->chambers()[tmb];
  Crate * thisCrate = sys_->crate();

  if (thisTMB == 0 || thisChamber == 0 || thisCrate == 0)
  {
    app_->Default(in, out);
    cout<<"No TMB selected!" << endl;
  }

  string urn = app_->getApplicationDescriptor()->getURN();

  emu::utils::headerXdaq(out, app_,
      toolbox::toString("%s TMB tests, %s slot=%d",
                        thisChamber->GetLabel().c_str(), thisCrate->GetLabel().c_str(), thisTMB->slot() )
  );

  string tmbStr = toolbox::toString("%d",tmb);

  *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;") << endl;

  ///////////////////////////////////////////
  // Run all tests button:

  *out << form().set("method","GET").set("action", "/" + urn + "/TMBRunTest" ) << endl;
  *out << input().set("type","submit").set("value", "Run All TMB tests").set("style", "color:blue") << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << input().set("type","hidden").set("value", "0").set("name", "testID");
  *out << form() << endl;

  *out << cgicc::table().set("border","1");

  ///////////////////////////////////////////
  // Run individual test buttons:

  *out << tr().set("ALIGN","center");

  TestButton(tmb, 1, "TMB test Boot register", tests_[tmb].GetResultTestBootRegister(), out);
  TestButton(tmb, 3, "TMB test firmware date", tests_[tmb].GetResultTestFirmwareDate(), out);
  TestButton(tmb, 4, "TMB test firmware type", tests_[tmb].GetResultTestFirmwareType(), out);

  *out << tr();

  ///////////////////////////////////////////////////////

  *out << tr().set("ALIGN","center");

  TestButton(tmb, 5, "TMB test firmware version", tests_[tmb].GetResultTestFirmwareVersion(), out);
  TestButton(tmb, 6, "TMB test firmware RevCode Id", tests_[tmb].GetResultTestFirmwareRevCode(), out);
  TestButton(tmb, 7, "TMB test mezzanine Id", tests_[tmb].GetResultTestMezzId(), out);

  *out << tr();

  /////////////////////////////////////////////////////////

  *out << tr().set("ALIGN","center");

  TestButton(tmb, 8, "TMB test PROM Id", tests_[tmb].GetResultTestPromId(), out);
  TestButton(tmb, 9, "TMB test PROM path", tests_[tmb].GetResultTestPROMPath(), out);
  TestButton(tmb, 10, "TMB test DSN", tests_[tmb].GetResultTestDSN(), out);

  *out << tr();

  /////////////////////////////////////////////////////////

  *out << tr().set("ALIGN","center");

  TestButton(tmb, 11, "TMB Voltages and temps", tests_[tmb].GetResultTestADC(), out);
  TestButton(tmb, 12, "TMB test 3d3444", tests_[tmb].GetResultTest3d3444(), out);
  TestButton(tmb, 13, "TMB test RAT temperature", tests_[tmb].GetResultTestRATtemper(), out);

  *out << tr();

  /////////////////////////////////////////////////////////

  *out << tr().set("ALIGN","center");

  TestButton(tmb, 14, "TMB test RAT Id Codes", tests_[tmb].GetResultTestRATidCodes(), out);
  TestButton(tmb, 15, "TMB test RAT User Codes", tests_[tmb].GetResultTestRATuserCodes(), out);
  TestButton(tmb, 16, "TMB test U760K", tests_[tmb].GetResultTestU76chip(), out);

  *out << tr();

  *out << cgicc::table();

  /////////////////////////////////////////////////////////////////////
  // Textarea with results

  *out << cgicc::textarea().set("name","TestOutput").set("WRAP","OFF").set("rows","20").set("cols","100");
  if (testOutputs_[tmb][sys_->crateN()].str().empty())
  {
    testOutputs_[tmb][sys_->crateN()] << "TMB-RAT Tests "
        << sys_->crate()->GetChamber(sys_->tmbs()[tmb]->slot())->GetLabel().c_str() << " output:" << endl;
  }
  *out << testOutputs_[tmb][sys_->crateN()].str() << endl;
  *out << cgicc::textarea();

  *out << form().set("method", "GET").set("action", "/" + urn + "/TMBLogTestsOutput" ) << endl;
  *out << input().set("type", "hidden").set("value", tmbStr).set("name", "tmb");
  *out << input().set("type", "submit").set("value", "Save log output").set("name", "LogTestsOutput") << endl;
  *out << input().set("type", "submit").set("value", "Clear").set("name", "ClearTestsOutput") << endl;
  *out << form() << endl;

  *out << cgicc::fieldset();

  emu::utils::footer(out);
}



void TMBTestModule::TestButton(int tmb, int testId, const string &label, int testResult, xgi::Output * out)
{
  *out << cgicc::td().set("ALIGN","center");
  *out << cgicc::form().set("method","GET").set("action", "/" + app_->getApplicationDescriptor()->getURN() + "/TMBRunTest" ) << endl;

  if ( testResult == -1 )
  {
    *out << cgicc::input().set("type", "submit").set("value", label).set("style", "color:blue" ) << endl;
  }
  else if ( testResult > 0 )
  {
    *out << cgicc::input().set("type", "submit").set("value", label).set("style", "color:green") << endl;
  }
  else
  {
    *out << cgicc::input().set("type", "submit").set("value", label).set("style", "color:red"  ) << endl;
  }
  *out << cgicc::input().set("type", "hidden").set("value", toolbox::toString("%d",tmb)).set("name", "tmb");
  *out << cgicc::input().set("type", "hidden").set("value", toolbox::toString("%d",testId)).set("name", "testID");
  *out << cgicc::form() << endl;
  *out << cgicc::td();
}



void TMBTestModule::TMBRunTest(xgi::Input * in, xgi::Output * out )
{
  cgicc::Cgicc cgi(in);
  cgicc::form_iterator name = cgi.getElement("tmb");

  int tmb=0;
  if(name != cgi.getElements().end())
  {
    tmb = cgi["tmb"]->getIntegerValue();
    cout << "RunTest:  TMB " << tmb << endl;
    tmbN_ = tmb;
  }

  name = cgi.getElement("testID");

  int testID=0;
  if(name != cgi.getElements().end())
  {
    testID = cgi["testID"]->getIntegerValue();
    cout << "testID " << testID << endl;
  }

  tests_[tmb].RedirectOutput(&testOutputs_[tmb][sys_->crateN()]);

  if ( testID == 1 || testID == 0 )
  {
    tests_[tmb].testBootRegister();
    ::sleep(1);
  }
  if ( testID == 2 || testID == 0 )
  {
    tests_[tmb].testVMEfpgaDataRegister();
  }
  if ( testID == 3 || testID == 0 )
  {
    tests_[tmb].testFirmwareDate();
  }
  if ( testID == 4 || testID == 0 )
  {
    tests_[tmb].testFirmwareType();
  }
  if ( testID == 5 || testID == 0 )
  {
    tests_[tmb].testFirmwareVersion();
  }
  if ( testID == 6 || testID == 0 )
  {
    tests_[tmb].testFirmwareRevCode();
  }
  if ( testID == 7 || testID == 0 )
  {
    tests_[tmb].testMezzId();
  }
  if ( testID == 8 || testID == 0 )
  {
    tests_[tmb].testPROMid();
  }
  if ( testID == 9 || testID == 0 )
  {
    tests_[tmb].testPROMpath();
  }
  if ( testID == 10 || testID == 0 )
  {
    tests_[tmb].testDSN();
  }
  if ( testID == 11 || testID == 0 )
  {
    tests_[tmb].testADC();
  }
  if ( testID == 12 || testID == 0 )
  {
    tests_[tmb].test3d3444();
  }
  if ( testID == 13 || testID == 0 )
  {
    tests_[tmb].testRATtemper();
  }
  if ( testID == 14 || testID == 0 )
  {
    tests_[tmb].testRATidCodes();
  }
  if ( testID == 15 || testID == 0 )
  {
    tests_[tmb].testRATuserCodes();
  }
  if ( testID == 16 || testID == 0 )
  {
    tests_[tmb].testU76chip();
  }

  tests_[tmb].RedirectOutput(&cout);

  //cout << "Done" << endl;

  this->TMBTestsPage(in,out);
}


void TMBTestModule::TMBLogTestsOutput(xgi::Input * in, xgi::Output * out )
{
  cout << "action TMBLogTestsOutput" << endl;

  cgicc::Cgicc cgi(in);
  cgicc::form_iterator name = cgi.getElement("tmb");

  int tmb;
  if (name != cgi.getElements().end())
  {
    tmb = cgi["tmb"]->getIntegerValue();
    cout << "TMB " << tmb << endl;
    tmbN_ = tmb;
  }
  else
  {
    cout << "No tmb in cgi input" << endl;
    tmb = tmbN_;
  }

  cgicc::form_iterator name2 = cgi.getElement("ClearTestsOutput");
  if (name2 != cgi.getElements().end())
  {
    cout << "Clear..." << endl;
    cout << cgi["ClearTestsOutput"]->getValue() << endl;
    testOutputs_[tmb][sys_->crateN()].str("");

    TMBTestsPage(in, out);
    return;
  }

  string tmb_slot = toolbox::toString("%d", sys_->tmbs()[tmb]->slot());
  string file_name = "TMBTests_slot" + tmb_slot + "_" + emu::utils::getDateTime(true) + ".log";
  emu::utils::saveAsFileDialog(out, testOutputs_[tmb][sys_->crateN()].str(), file_name);

  /*
  char buf[20];
  sprintf(buf, "/tmp/TMBTestsLogFile_%d.log", sys_->tmbs()[tmb]->slot());

  std::ofstream log_file;
  log_file.open(buf);
  log_file << testOutputs_[tmb][sys_->crateN()].str();
  log_file.close();

  testOutputs_[tmb][sys_->crateN()].str("");

  TMBTestsPage(in, out);
  */
}


}}  // namespaces
