/*
 * $Id: $
 */

// class header
#include "emu/pc/CCBBackplaneTestModule.h"

// Emu includes
#include "emu/pc/ConfigurablePCrates.h"
#include "emu/pc/CCBCommands.h"
#include "emu/pc/Crate.h"
#include "emu/pc/Chamber.h"
#include "emu/pc/TMB.h"
#include "emu/pc/CCB.h"
#include "emu/pc/RAT.h"
#include "emu/utils/Cgi.h"
#include "emu/exception/Exception.h"

// XDAQ includes
#include "cgicc/Cgicc.h"
#include "cgicc/HTMLClasses.h"
#include "toolbox/string.h"
#include "xgi/Utils.h"
#include "xdaq/WebApplication.h"

// system includes
#include <iostream>
#include <unistd.h> // for sleep()

namespace emu {  namespace pc {

using std::endl;
using std::cout;
using std::string;


CCBBackplaneTestModule::CCBBackplaneTestModule(xdaq::WebApplication *app, ConfigurablePCrates *sys)
: app_(app)
, sys_(sys)
{
  // init with the "current" crate in the system, if it is set
  if (sys_->crateN() >= 0)
  {
    Init();
  }
}


void CCBBackplaneTestModule::Init()
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
    CCBBackplaneTester tmp;
    tmp.SetTMB(tmbs[i]);
    tmp.SetCCB(ccb);
    tests_.push_back(tmp);
  }
}


void CCBBackplaneTestModule::CCBBackplaneTestsPage(xgi::Input * in, xgi::Output * out )
{
  using cgicc::tr;
  using cgicc::form;
  using cgicc::input;

  cgicc::Cgicc cgi(in);

  int tmb = sys_->tmbN();
  TMB * thisTMB = sys_->tmb();
  Chamber * thisChamber = sys_->chamber();
  Crate * thisCrate = sys_->crate();

  if (thisTMB == 0 || thisChamber == 0 || thisCrate == 0)
  {
    cout<<__func__<<": Current TMB is not set!!!" << endl;
    app_->Default(in, out);
  }

  string urn = app_->getApplicationDescriptor()->getURN();

  emu::utils::headerXdaq(out, app_,
      toolbox::toString("%s TMB-CCB tests, %s slot=%d",
                        thisChamber->GetLabel().c_str(), thisCrate->GetLabel().c_str(), thisTMB->slot() )
  );

  string tmbStr = toolbox::toString("%d",tmb);

  *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;") << endl;

  ///////////////////////////////////////////
  // Run all tests button:

  *out << form().set("method","GET").set("action", "/" + urn + "/CCBBackplaneRunTest" ) << endl;
  *out << input().set("type","submit").set("value", "Run All TMB tests").set("style", "color:blue") << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << input().set("type","hidden").set("value", "All").set("name", "test_label");
  *out << form() << endl;

  *out << cgicc::table().set("border","1");

  ///////////////////////////////////////////
  // Run individual test buttons:

  *out << tr().set("ALIGN","center");

  TestButton(tmb, "Pulse Counter Bits", "PulseCountersBits", out);
  TestButton(tmb, "Dummy", "Dummy", out);
  TestButton(tmb, "Dummy", "Dummy", out);

  *out << tr();

  /////////////////////////////////////////////////////////
/*
  *out << tr().set("ALIGN","center");

  TestButton(tmb, 5, "Dummy", tests_[tmb].GetTestResult("Dummy"), out);
  TestButton(tmb, 6, "Dummy", tests_[tmb].GetTestResult("Dummy"), out);
  TestButton(tmb, 7, "Dummy", tests_[tmb].GetTestResult("Dummy"), out);

  *out << tr();
*/
  *out << cgicc::table();

  /////////////////////////////////////////////////////////////////////
  // Textarea with results

  *out << cgicc::textarea().set("name","TestOutput").set("WRAP","OFF").set("rows","20").set("cols","100");
  if (testOutputs_[tmb].str().empty())
  {
    testOutputs_[tmb] << "TMB-CCB Backplane Tests "
        << sys_->crate()->GetChamber(sys_->tmbs()[tmb]->slot())->GetLabel().c_str() << " output:" << endl;
  }
  *out << testOutputs_[tmb].str() << endl;
  *out << cgicc::textarea();

  *out << form().set("method", "GET").set("action", "/" + urn + "/CCBBackplaneLogTestsOutput" ) << endl;
  *out << input().set("type", "hidden").set("value", tmbStr).set("name", "tmb");
  *out << input().set("type", "submit").set("value", "Log output").set("name", "LogTestsOutput") << endl;
  *out << input().set("type", "submit").set("value", "Clear").set("name", "ClearTestsOutput") << endl;
  *out << form() << endl;

  *out << cgicc::fieldset();

  emu::utils::footer(out);
}



void CCBBackplaneTestModule::TestButton(int tmb, const string &label, const string &test_label, xgi::Output * out)
{
  int testResult = tests_[tmb].GetTestResult(test_label);

  *out << cgicc::td().set("ALIGN","center");
  *out << cgicc::form().set("method","GET").set("action", "/" + app_->getApplicationDescriptor()->getURN() + "/CCBBackplaneRunTest" ) << endl;

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
  *out << cgicc::input().set("type", "hidden").set("value", test_label).set("name", "test_label");
  *out << cgicc::form() << endl;
  *out << cgicc::td();
}



void CCBBackplaneTestModule::CCBBackplaneRunTest(xgi::Input * in, xgi::Output * out )
{
  cgicc::Cgicc cgi(in);
  cgicc::form_iterator name = cgi.getElement("tmb");

  int tmb=0;
  if(name != cgi.getElements().end())
  {
    tmb = cgi["tmb"]->getIntegerValue();
    cout << __func__ << ":  TMB " << tmb << endl;
    tmbN_ = tmb;
  }

  string test_label = "";

  name = cgi.getElement("test_label");
  if(name != cgi.getElements().end())
  {
    test_label = cgi["test_label"]->getValue();
    cout << __func__ << ":  test_label " << test_label << endl;
  }

  tests_[tmb].RedirectOutput(&testOutputs_[tmb]);

  tests_[tmb].RunTest(test_label);

  tests_[tmb].RedirectOutput(&cout);

  //cout << "Done" << endl;

  this->CCBBackplaneTestsPage(in,out);
}


void CCBBackplaneTestModule::CCBBackplaneLogTestsOutput(xgi::Input * in, xgi::Output * out )
{
  cout << "action TMBTestModule::LogTestsOutput" << endl;

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
    testOutputs_[tmb].str("");

    CCBBackplaneTestsPage(in, out);
    return;
  }

  char buf[20];
  sprintf(buf, "/tmp/TMBTestsLogFile_%d.log", sys_->tmbs()[tmb]->slot());

  std::ofstream log_file;
  log_file.open(buf);
  log_file << testOutputs_[tmb].str();
  log_file.close();

  testOutputs_[tmb].str("");

  CCBBackplaneTestsPage(in, out);
}


}}  // namespaces
