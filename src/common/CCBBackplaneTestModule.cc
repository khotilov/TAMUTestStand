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
#include "emu/utils/System.h"
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
, tmbN_(0)
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
    cout<<__PRETTY_FUNCTION__<<": current crate is not set"<<endl;
    return;
  }

  tm_.Init(sys_);
}


std::ostringstream& CCBBackplaneTestModule::getTestOutput(int tmb)
{
  return tm_.GetTestOutput(tmb);
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

  if (thisCrate == 0)
  {
    *out << __PRETTY_FUNCTION__ << ": Current crate is not set!!!" << endl;
  }
  if (thisChamber == 0)
  {
    *out << __PRETTY_FUNCTION__ << ": Current chamber is not set!!!" << endl;
  }
  if (thisTMB == 0)
  {
    *out << __PRETTY_FUNCTION__ << ": CurrenTestt TMB is not set!!!" << endl;
  }
  if (thisTMB == 0 || thisChamber == 0 || thisCrate == 0)  return;


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
  *out << input().set("type","submit").set("Testvalue", "Run All TMB tests").set("style", "color:blue") << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << input().set("type","hidden").set("value", "All").set("name", "test_label");
  *out << form() << endl;

  *out << cgicc::table().set("border","1");

  ///////////////////////////////////////////
  // Run individual test buttons:

  *out << tr().set("ALIGN","center");

  TestButton(tmb, "L1Reset", "L1Reset", out);
  TestButton(tmb, "TMB HardReset", "TMBHardReset", out);
  TestButton(tmb, "Pulse Counters", "PulseCounters", out);
  TestButton(tmb, "Command Bus", "CommandBus", out);
  TestButton(tmb, "Data Bus", "DataBus", out);

  *out << tr();

  /////////////////////////////////////////////////////////

  *out << tr().set("ALIGN","center");

  TestButton(tmb, "Clock 40", "CCBClock40", out);
  TestButton(tmb, "CCB_reserved", "CCBReserved", out);
  TestButton(tmb, "TMB_reserved_out", "TMBReservedOut", out);
  TestButton(tmb, "DMB_reserved_out", "DMBReservedOut", out);
  TestButton(tmb, "Dummy", "Dummy", out);

  *out << tr();
  /////////////////////////////////////////////////////////

  *out << tr().set("ALIGN","center");

  TestButton(tmb, "DMB_reserved_in-loop", "DMBReservedInLoopback", out);
  TestButton(tmb, "DMB_L1A_Release-loop", "DMBL1AReleaseLoopback", out);

  *out << tr();

  *out << cgicc::table();

  /////////////////////////////////////////////////////////////////////
  // Textarea with results

  *out << cgicc::textarea().set("name","TestOutput").set("WRAP","OFF").set("rows","20").set("cols","100");
  if (tm_.GetTestOutput(tmb).str().empty())
  {
    tm_.GetTestOutput(tmb) << "TMB-CCB Backplane Tests "
        << sys_->crate()->GetChamber(sys_->tmbs()[tmb]->slot())->GetLabel().c_str() << " output:" << endl;
  }
  *out << tm_.GetTestOutput(tmb).str() << endl;
  *out << cgicc::textarea();

  *out << form().set("method", "GET").set("action", "/" + urn + "/CCBBackplaneLogTestsOutput" ) << endl;
  *out << input().set("type", "hidden").set("value", tmbStr).set("name", "tmb");
  *out << input().set("type", "submit").set("value", "Save log output").set("name", "LogTestsOutput") << endl;
  *out << input().set("type", "submit").set("value", "Clear").set("name", "ClearTestsOutput") << endl;
  *out << form() << endl;

  *out << cgicc::fieldset();

  emu::utils::footer(out);
}


void CCBBackplaneTestModule::TestButton(int tmb, const string &label, const string &test_label, xgi::Output * out)
{
  int testResult = tm_.GetTester("CCBBackplaneTester", tmb)->GetTestResult(test_label);

  *out << cgicc::td().set("ALIGN","center");
  *out << cgicc::form().set("method","GET").set("action", "/" + app_->getApplicationDescriptor()->getURN() + "/CCBBackplaneRunTest" ) << endl;

  if ( testResult == -1 ) // wasn't run yet
  {
    *out << cgicc::input().set("type", "submit").set("value", label).set("style", "color:blue" ) << endl;
  }
  else if ( testResult == 0 ) // ok
  {
    *out << cgicc::input().set("type", "submit").set("value", label).set("style", "color:green") << endl;
  }
  else // some non-zero error code
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

  if(name != cgi.getElements().end())
  {
    tmbN_ = cgi["tmb"]->getIntegerValue();
  }
  else
  {
    cout << __func__ << "No tmb# in cgi input!" << endl;
  }

  string test_label = "";

  name = cgi.getElement("test_label");
  if(name != cgi.getElements().end())
  {
    test_label = cgi["test_label"]->getValue();
  }

  cout << __func__ << ":  test " << test_label<< "  for TMB #" << tmbN_  << endl;

  try
  {
    tm_.GetTester("CCBBackplaneTester", tmbN_)->RunTest(test_label);
  }
  catch (emu::exception::Exception &e)
  {
    *out << e.toHTML();
  }

  this->CCBBackplaneTestsPage(in,out);
}


void CCBBackplaneTestModule::CCBBackplaneLogTestsOutput(xgi::Input * in, xgi::Output * out )
{
  cgicc::Cgicc cgi(in);
  cgicc::form_iterator name = cgi.getElement("tmb");

  if (name != cgi.getElements().end())
  {
    tmbN_ = cgi["tmb"]->getIntegerValue();
  }
  else
  {
    cout << __func__ << "No tmb# in cgi input!" << endl;
  }

  cout << __func__ << ":  for TMB #" << tmbN_  << endl;

  cgicc::form_iterator name2 = cgi.getElement("ClearTestsOutput");
  if (name2 != cgi.getElements().end())
  {
    cout << __func__ << "ClearTestsOutput" << endl;
    tm_.GetTestOutput(tmbN_).str("");

    CCBBackplaneTestsPage(in, out);
    return;
  }

  string tmb_slot = toolbox::toString("%d", sys_->tmbs()[tmbN_]->slot());
  string file_name = "CCBBackplaneTests_TMBslot" + tmb_slot + "_" + emu::utils::getDateTime(true) + ".log";
  emu::utils::saveAsFileDialog(out, tm_.GetTestOutput(tmbN_).str(), file_name);
}


}}  // namespaces
