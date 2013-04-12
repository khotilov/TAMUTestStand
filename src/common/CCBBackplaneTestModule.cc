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
#include <map>
#include <unistd.h> // for sleep()

namespace emu {  namespace pc {

using std::endl;
using std::cout;
using std::string;
using std::map;


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
  return tm_.GetTestOutput("CCBBackplaneTester", tmb);
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

  *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;") << endl;
  *out << cgicc::legend("Board Selection:").set("style", "color:blue") <<endl;
  *out << form().set("method","GET").set("action", "/" + urn + "/SetBoardLabel" ) << endl;
  *out << "Board Label: " << tm_.GetBoardLabel() << cgicc::br() << endl;
  *out << input().set("type", "hidden").set("name", "return").set("value", "CCBBackplaneTestsPage") << endl;
  *out << input().set("type", "text").set("name", "label").set("size", "20").set("value", "") << endl;
  *out << input().set("type","submit").set("value", "Set Board Label").set("style", "color:blue") << endl;
  *out << form() << endl;
  *out << cgicc::fieldset() << endl;

  string tmbStr = toolbox::toString("%d",tmb);

  *out << "<table cellpadding=\"2\" cellspacing=\"2\" border=\"0\" style=\"width: 100%; font-family: arial;\">" << endl;
  *out << "<tbody>" << endl;
  *out << "<tr>" << endl;
  *out << "<td width=\"50%\">" << endl;

  *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;") << endl;
  *out << cgicc::legend("CCB Backplane Tests:").set("style", "color:blue") <<endl;

  ///////////////////////////////////////////
  // Run all tests button:

  *out << form().set("method","GET").set("action", "/" + urn + "/CCBBackplaneRunTest" ) << endl;
  *out << input().set("type","submit").set("value", "Run All CCB tests").set("style", "color:blue") << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << input().set("type","hidden").set("value", "All").set("name", "test_label");
  *out << form() << endl;

  *out << cgicc::table().set("border","1");

  ///////////////////////////////////////////
  // Run individual test buttons:

  *out << tr().set("ALIGN","center");

  TestButton(tmb, "L1Reset", "L1Reset", out);
  TestButton(tmb, "TMB HardReset", "TMBHardReset", out);
  TestButton(tmb, "Command Bus", "CommandBus", out);
  TestButton(tmb, "Pulse Counters", "PulseCounters", out);
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
  if (tm_.GetTestOutput("CCBBackplaneTester", tmb).str().empty())
  {
    tm_.GetTestOutput("CCBBackplaneTester", tmb) << "TMB-CCB Backplane Tests "
        << sys_->crate()->GetChamber(sys_->tmbs()[tmb]->slot())->GetLabel().c_str() << " output:" << endl;
  }
  *out << tm_.GetTestOutput("CCBBackplaneTester", tmb).str() << endl;
  *out << cgicc::textarea();

  *out << form().set("method", "GET").set("action", "/" + urn + "/CCBBackplaneLogTestsOutput" ) << endl;
  *out << input().set("type", "hidden").set("value", tmbStr).set("name", "tmb");
  *out << input().set("type", "submit").set("value", "Save log output").set("name", "LogTestsOutput") << endl;
  *out << input().set("type", "submit").set("value", "Clear").set("name", "ClearTestsOutput") << endl;
  *out << form() << endl;

  *out << cgicc::fieldset();
  *out << "</td>" << endl;

  /////////////////////////////////////////////////////////////////////
  // Visual Test Area

  *out << "<td valign=\"top\">" << endl;
  *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;") << endl;
  *out << cgicc::legend("Visual Test:").set("style", "color:blue") <<endl;
  *out << form().set("method", "GET").set("action", "/" + urn + "/TestLEDFrontPanel") << endl;

  int TestLEDFrontPanel_status = tm_.GetTester("CCBBackplaneTester", tmbN_)->GetTestStatus("TestLEDFrontPanel");
  int TestLEDFrontPanel_result = tm_.GetTester("CCBBackplaneTester", tmbN_)->GetTestResult("TestLEDFrontPanel");

  if (TestLEDFrontPanel_status <= 0)
  {
    if(TestLEDFrontPanel_result == -1)
      *out << input().set("type","submit").set("value", "Test_LEDs").set("style", "color:blue") << endl;
    else if(TestLEDFrontPanel_result == 0)
      *out << input().set("type","submit").set("value", "Test_LEDs").set("style", "color:green") << endl;
    else if(TestLEDFrontPanel_result > 0)
      *out << input().set("type","submit").set("value", "Test_LEDs").set("style", "color:red") << endl;
    *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  }
  else if (TestLEDFrontPanel_status==1)
  {
    *out << cgicc::p() << "Please watch LEDs on CCB front panel connected to LVDS OUTPUT 2" << cgicc::br();
    *out << "LEDs should turn on one at a time for .5s" << cgicc::br();
    *out << "Make note of LEDs that do not turn on at all or fail to turn on individually" << cgicc::br();
    *out << "Click Next to begin test" << cgicc::p();
    *out << input().set("type","submit").set("value", "Next").set("style", "color:blue") << endl;
    *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  }
  else if (TestLEDFrontPanel_status == 2)
  {
    *out << cgicc::p() << "Did each LED turn on individually?" << cgicc::br() << cgicc::p();
    *out << input().set("type","submit").set("value", "Yes").set("style", "color:blue").set("name", "result") << endl;
    *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb") << endl;
    *out << input().set("type","submit").set("value", "No").set("style", "color:blue").set("name", "result") << endl;
    *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb") << endl;
  }
  else if (TestLEDFrontPanel_status == 3)
  {
    *out << cgicc::p() << "Please check which LEDs did not turn on or failed to turn on individually" << cgicc::br() << cgicc::p();
    *out << "<table cellpadding=\"0\" cellspacing=\"0\" border=\"0\" style=\"width: 100%; font-family: arial;\">" << endl;
    *out << "<tbody>" << endl;
    *out << "<tr>";
    *out << "<td rowspan=9 width=20>"; // Cell for image
    //*out << cgicc::img().set("src", "http://i.imgur.com/6bPzz.png").set("height", "180") << "</td>" << endl;
    *out << cgicc::img().set("src", "/emu/emuDCS/TAMUTestStand/images/LED_Panel.png").set("height", "180") << "</td>" << endl;
    *out << "<td style=\"height: 20px;\">" << input().set("type", "checkbox").set("name", "ledcheck8") << "</td></tr>" << endl;
    *out << "<tr><td style=\"height: 20px;\">" << input().set("type", "checkbox").set("name", "ledcheck7") << "</td></tr>" << endl;
    *out << "<tr><td style=\"height: 20px;\">" << input().set("type", "checkbox").set("name", "ledcheck6") << "</td></tr>" << endl;
    *out << "<tr><td style=\"height: 20px;\">" << input().set("type", "checkbox").set("name", "ledcheck5") << "</td></tr>" << endl;
    *out << "<tr><td style=\"height: 20px;\">" << input().set("type", "checkbox").set("name", "ledcheck4") << "</td></tr>" << endl;
    *out << "<tr><td style=\"height: 20px;\">" << input().set("type", "checkbox").set("name", "ledcheck3") << "</td></tr>" << endl;
    *out << "<tr><td style=\"height: 20px;\">" << input().set("type", "checkbox").set("name", "ledcheck2") << "</td></tr>" << endl;
    *out << "<tr><td style=\"height: 20px;\">" << input().set("type", "checkbox").set("name", "ledcheck1") << "</td></tr>" << endl;
    *out << "<tr><td style=\"height: 20px;\">" << input().set("type", "checkbox").set("name", "ledcheck0") << "</td></tr>" << endl;
    *out << "</tbody></table>" << endl;
    *out << input().set("type","submit").set("value", "Submit").set("style", "color:blue") << endl;
    *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  }


  *out << form() << endl;
  *out << cgicc::fieldset() << endl;

  *out << "</td>" << endl;

  *out << "</tr>" << endl;
  *out << "</tbody>" << endl;
  *out << "</table>" << endl;


  emu::utils::footer(out);
}

void CCBBackplaneTestModule::TestLEDFrontPanel(xgi::Input * in, xgi::Output * out)
{
  cgicc::Cgicc cgi(in);
  cgicc::form_iterator name = cgi.getElement("tmb");
  CCB * ccb = sys_->ccb();
  //TMB * tmb = sys_->tmb();
  int status = tm_.GetTester("CCBBackplaneTester", tmbN_)->GetTestStatus("TestLEDFrontPanel");
  int Niter = 5;

  if(name != cgi.getElements().end())
  {
    tmbN_ = cgi["tmb"]->getIntegerValue();
  }
  else
  {
    cout << __func__ << "No tmb# in cgi input!" << endl;
  }

  if(status <= 0)
  {
    status = 1;
  }
  else if(status == 1)
  {
    for(int i=0; i<Niter; ++i)
    {
      ccb->WriteRegister(CCB_VME_L1RESET, 1);
      for(int j=0; j<LENGTH_PULSE_COUNTER; ++j)
      {
        int n;
        if(j==0)
          n=1;
        else
          n=(0x1 << (j-1));
        NTimesWriteRegister(ccb, n, PULSE_IN_COMMANDS[0], 1);

        usleep(500000);
      }
    }

    status = 2;
  }
  else if(status == 2)
  {
    if( cgi.getElement("result")->getValue() == "Yes")
    {
      tm_.GetTester("CCBBackplaneTester", tmbN_)->SetTestResult("TestLEDFrontPanel", 0);
      tm_.GetTester("CCBBackplaneTester", tmbN_)->RunTest("TestLEDFrontPanel");
      status = 0;
    }
    else
    {
      status = 3;
    }
  }
  else if(status == 3)
  {
    int checked_boxes = 0;

    //Get the boxes checked
    for(int i=0; i<LENGTH_PULSE_COUNTER; ++i)
    {
      string name = "ledcheck";
      char numstr[21]; // enough to hold all numbers up to 64-bits
      sprintf(numstr, "%d", i);
      name+=numstr;
      checked_boxes |= (cgi.queryCheckbox(name) << i);
    }
    tm_.GetTester("CCBBackplaneTester", tmbN_)->SetTestResult("TestLEDFrontPanel", checked_boxes);
    tm_.GetTester("CCBBackplaneTester", tmbN_)->RunTest("TestLEDFrontPanel");

    status = 0;
  }
  tm_.GetTester("CCBBackplaneTester", tmbN_)->SetTestStatus("TestLEDFrontPanel", status);

  this->CCBBackplaneTestsPage(in,out);
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


void CCBBackplaneTestModule::FirmwareTestsPage(xgi::Input * in, xgi::Output * out )
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

  *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;") << endl;
  *out << cgicc::legend("Board Selection:").set("style", "color:blue") <<endl;
  *out << form().set("method","GET").set("action", "/" + urn + "/SetBoardLabel" ) << endl;
  *out << "Board Label: " << tm_.GetBoardLabel() << cgicc::br() << endl;
  *out << input().set("type", "hidden").set("name", "return").set("value", "FirmwareTestsPage") << endl;
  *out << input().set("type", "text").set("name", "label").set("size", "20").set("value", "") << endl;
  *out << input().set("type","submit").set("value", "Set Board Label").set("style", "color:blue") << endl;
  *out << form() << endl;
  *out << cgicc::fieldset() << endl;

  string tmbStr = toolbox::toString("%d",tmb);

  ///////////// Begin Firmware Test Controls box /////////////////
  *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;") << endl;
  *out << cgicc::legend("Firmware Test Controls:").set("style", "color:blue") <<endl;
  *out << cgicc::table().set("border","1");

  *out << tr().set("ALIGN","center");
  ///////////////////////////////////////////
  ///L1Reset button:

  *out << cgicc::td().set("ALIGN","center");
  *out << form().set("method","GET").set("action", "/" + urn + "/RunFirmwareCommand" ) << endl;
  *out << input().set("type","submit").set("value", "L1Reset").set("style", "color:blue") << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << input().set("type","hidden").set("value", "L1Reset").set("name", "command");
  *out << form() << endl;
  *out << cgicc::td();

  ///L1Reset button
  *out << cgicc::td().set("ALIGN","center");
  *out << form().set("method","GET").set("action", "/" + urn + "/RunFirmwareCommand" ) << endl;
  *out << input().set("type","submit").set("value", "TMB_Soft_Reset").set("style", "color:blue") << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << input().set("type","hidden").set("value", "TMBSoftReset").set("name", "command");
  *out << form() << endl;
  *out << cgicc::td();

  ///Status Button
  *out << cgicc::td().set("ALIGN","center");
  *out << cgicc::form().set("method","GET").set("action", "/" + urn + "/RunFirmwareCommand" ) << endl;
  *out << cgicc::input().set("type", "submit").set("value", "Check Status").set("style", "color:blue" ) << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << cgicc::input().set("type", "hidden").set("value", "CheckFirmwareTestStatus").set("name", "command");
  *out << cgicc::form() << endl;
  *out << cgicc::td();

  ///Loopback Button
  *out << cgicc::td().set("ALIGN","center");
  *out << cgicc::form().set("method","GET").set("action", "/" + urn + "/RunFirmwareCommand" ) << endl;

  if(tm_.GetTester("FirmwareTester")->GetTestStatus("Loopback") == 1) //Green enabled button if test is running
    *out << cgicc::input().set("type", "submit").set("value", "Loopback").set("style", "color:green" ) << endl;
  else if(tm_.GetTester("FirmwareTester")->GetTestStatus("Loopback") == 0) //Red enabled button if test is not running
    *out << cgicc::input().set("type", "submit").set("value", "Loopback").set("style", "color:red" ) << endl;
  else //Blue disabled button if status of test is unknown
    *out << cgicc::input().set("type", "submit").set("value", "Loopback").set("style", "color:blue" ).set("disabled", "true") << endl;

  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << cgicc::input().set("type", "hidden").set("value", "ToggleLoopbackTest").set("name", "command");
  *out << cgicc::form() << endl;
  *out << cgicc::td();

  ///Cable Button
  *out << cgicc::td().set("ALIGN","center");
  *out << cgicc::form().set("method","GET").set("action", "/" + urn + "/RunFirmwareCommand" ) << endl;

  if(tm_.GetTester("FirmwareTester")->GetTestStatus("Cable") == 1) //Green enabled button if test is running
    *out << cgicc::input().set("type", "submit").set("value", "Cable").set("style", "color:green" ) << endl;
  else if(tm_.GetTester("FirmwareTester")->GetTestStatus("Cable") == 0) //Red enabled button if test is not running
    *out << cgicc::input().set("type", "submit").set("value", "Cable").set("style", "color:red" ) << endl;
  else //Blue disabled button if status of test is unknown
    *out << cgicc::input().set("type", "submit").set("value", "Cable").set("style", "color:blue" ).set("disabled", "true") << endl;

  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << cgicc::input().set("type", "hidden").set("value", "ToggleCableTest").set("name", "command");
  *out << cgicc::form() << endl;
  *out << cgicc::td();

  ///Fiber Button
  *out << cgicc::td().set("ALIGN","center");
  *out << cgicc::form().set("method","GET").set("action", "/" + urn + "/RunFirmwareCommand" ) << endl;

  if(tm_.GetTester("FirmwareTester")->GetTestStatus("Fiber") == 1) //Green enabled button if test is running
    *out << cgicc::input().set("type", "submit").set("value", "Fiber").set("style", "color:green" ) << endl;
  else if(tm_.GetTester("FirmwareTester")->GetTestStatus("Fiber") == 0) //Red enabled button if test is not running
    *out << cgicc::input().set("type", "submit").set("value", "Fiber").set("style", "color:red" ) << endl;
  else //Blue disabled button if status of test is unknown
    *out << cgicc::input().set("type", "submit").set("value", "Fiber").set("style", "color:blue" ).set("disabled", "true") << endl;

  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << cgicc::input().set("type", "hidden").set("value", "ToggleFiberTest").set("name", "command");
  *out << cgicc::form() << endl;
  *out << cgicc::td();

  *out << tr();

  *out << cgicc::table();

  *out << cgicc::fieldset();
  ///////////// End Firmware Test Controls box /////////////////


  ///////////// Begin Firmware Tests box /////////////////

   *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;") << endl;
   *out << cgicc::legend("Firmware Tests:").set("style", "color:blue") <<endl;
   *out << cgicc::table().set("border","1");
   *out << tr().set("ALIGN","center");

  int testResult;

  // DMB_loopback button
  testResult = tm_.GetTester("FirmwareTester", tmb)->GetTestResult("TestDMBLoopback");
  *out << cgicc::td().set("ALIGN","center");
  *out << cgicc::form().set("method","GET").set("action", "/" + urn + "/RunFirmwareCommand" ) << endl;
  if (testResult == -1)
    *out << cgicc::input().set("type", "submit").set("value", "DMB_loopback").set("style", "color:blue" ) << endl;
  else if ( testResult == 0 ) // ok
    *out << cgicc::input().set("type", "submit").set("value", "DMB_loopback").set("style", "color:green") << endl;
  else // some non-zero error code
    *out << cgicc::input().set("type", "submit").set("value", "DMB_loopback").set("style", "color:red"  ) << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << cgicc::input().set("type", "hidden").set("value", "RunTest").set("name", "command");
  *out << cgicc::input().set("type", "hidden").set("value", "TestDMBLoopback").set("name", "test_label");
  *out << cgicc::form() << endl;
  *out << cgicc::td();

  // RPC_loopback button
  testResult = tm_.GetTester("FirmwareTester", tmb)->GetTestResult("TestRPCLoopback");
  *out << cgicc::td().set("ALIGN","center");
  *out << cgicc::form().set("method","GET").set("action", "/" + urn + "/RunFirmwareCommand" ) << endl;
  if (testResult == -1)
    *out << cgicc::input().set("type", "submit").set("value", "RPC_loopback").set("style", "color:blue" ) << endl;
  else if ( testResult == 0 ) // ok
    *out << cgicc::input().set("type", "submit").set("value", "RPC_loopback").set("style", "color:green") << endl;
  else // some non-zero error code
    *out << cgicc::input().set("type", "submit").set("value", "RPC_loopback").set("style", "color:red"  ) << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << cgicc::input().set("type", "hidden").set("value", "RunTest").set("name", "command");
  *out << cgicc::input().set("type", "hidden").set("value", "TestRPCLoopback").set("name", "test_label");
  *out << cgicc::form() << endl;
  *out << cgicc::td();

  // Cable_connector button
  testResult = tm_.GetTester("FirmwareTester", tmb)->GetTestResult("TestCableConnector");
  *out << cgicc::td().set("ALIGN","center");
  *out << cgicc::form().set("method","GET").set("action", "/" + urn + "/RunFirmwareCommand" ) << endl;
  if (testResult == -1)
    *out << cgicc::input().set("type", "submit").set("value", "Cable_connector").set("style", "color:blue" ) << endl;
  else if ( testResult == 0 ) // ok
    *out << cgicc::input().set("type", "submit").set("value", "Cable_connector").set("style", "color:green") << endl;
  else // some non-zero error code
    *out << cgicc::input().set("type", "submit").set("value", "Cable_connector").set("style", "color:red"  ) << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << cgicc::input().set("type", "hidden").set("value", "RunTest").set("name", "command");
  *out << cgicc::input().set("type", "hidden").set("value", "TestCableConnector").set("name", "test_label");
  *out << cgicc::form() << endl;
  *out << cgicc::td();

  // Fiber_connector button
  testResult = tm_.GetTester("FirmwareTester", tmb)->GetTestResult("TestFiberConnector");
  *out << cgicc::td().set("ALIGN","center");
  *out << cgicc::form().set("method","GET").set("action", "/" + urn + "/RunFirmwareCommand" ) << endl;
  if (testResult == -1)
    *out << cgicc::input().set("type", "submit").set("value", "Fiber_connector").set("style", "color:blue" ) << endl;
  else if ( testResult == 0 ) // ok
    *out << cgicc::input().set("type", "submit").set("value", "Fiber_connector").set("style", "color:green") << endl;
  else // some non-zero error code
    *out << cgicc::input().set("type", "submit").set("value", "Fiber_connector").set("style", "color:red"  ) << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << cgicc::input().set("type", "hidden").set("value", "RunTest").set("name", "command");
  *out << cgicc::input().set("type", "hidden").set("value", "TestFiberConnector").set("name", "test_label");
  *out << cgicc::form() << endl;
  *out << cgicc::td();

  *out << tr();

  *out << cgicc::table();

  /////////////////////////////////////////////////////////////////////
  // Textarea with results

  *out << cgicc::textarea().set("name","TestOutput").set("WRAP","OFF").set("rows","20").set("cols","100");
  if (tm_.GetTestOutput("FirmwareTester", tmb).str().empty())
  {
    tm_.GetTestOutput("FirmwareTester", tmb) << "Firmware Tests "
        << sys_->crate()->GetChamber(sys_->tmbs()[tmb]->slot())->GetLabel().c_str() << " output:" << endl;
  }
  *out << tm_.GetTestOutput("FirmwareTester", tmb).str() << endl;
  *out << cgicc::textarea();

  *out << form().set("method", "GET").set("action", "/" + urn + "/FirmwareLogTestsOutput" ) << endl;
  *out << input().set("type","hidden").set("value", tmbStr).set("name", "tmb");
  *out << input().set("type", "submit").set("value", "Save log output").set("name", "LogTestsOutput") << endl;
  *out << input().set("type", "submit").set("value", "Clear").set("name", "ClearTestsOutput") << endl;
  *out << form() << endl;

  *out << cgicc::fieldset();

  emu::utils::footer(out);
}

void CCBBackplaneTestModule::CheckFirmwareTestEnable()
{
  int result;
  // Walk over TEST_COUNT_COMMANDS
  for(int i = 0; i < LENGTH_FIRMWARE_TEST_COUNT_COMMANDS; ++i)
  {
    uint32_t read_0, read_1;
    read_0 = ResultRegisterData(LoadAndReadResultRegister(sys_->ccb(), sys_->tmb()->slot(), FIRMWARE_TEST_COUNT_COMMANDS[i]));
    usleep(100);
    read_1 = ResultRegisterData(LoadAndReadResultRegister(sys_->ccb(), sys_->tmb()->slot(), FIRMWARE_TEST_COUNT_COMMANDS[i]));

    // Check if test count is increasing by comparing two different reads
    result = (read_0 != read_1);
    tm_.GetTester("FirmwareTester")->SetTestStatus(FIRMWARE_TEST_COUNT_LABELS[i], result);
  }
}

void CCBBackplaneTestModule::RunFirmwareCommand(xgi::Input * in, xgi::Output * out)
{
  cgicc::Cgicc cgi(in);
  cgicc::form_iterator name = cgi.getElement("tmb");

  CCB * ccb = sys_->ccb();
  //TMB * tmb = sys_->tmb();

  if(name != cgi.getElements().end())
  {
    tmbN_ = cgi["tmb"]->getIntegerValue();
  }
  else
  {
    cout << __func__ << "No tmb# in cgi input!" << endl;
  }

  string command = "";

  name = cgi.getElement("command");
  if(name != cgi.getElements().end())
  {
    command = cgi["command"]->getValue();
  }

  try
  {
    if(command == "L1Reset")
    {
      ccb->WriteRegister(CCB_VME_L1RESET, 1);
      cout << endl << "L1Reset" << endl;
      CheckFirmwareTestEnable();
    }

    else if(command == "TMBSoftReset")
    {
      ccb->WriteRegister(CCB_VME_TMB_SOFT_RESET, 1);
      cout << endl << "TMB_Soft_Reset" << endl;
    }

    else if(command == "CheckFirmwareTestStatus")
    {
      CheckFirmwareTestEnable();
      tm_.GetTester("FirmwareTester", tmbN_)->RunTest("CheckFirmwareTestStatus");
    }

    else if(command == "ToggleLoopbackTest")
    {
      if(tm_.GetTester("FirmwareTester", tmbN_)->GetTestStatus("Loopback"))
      {
        ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_STOP_TRIG_LOOP); //Disable loopback tests
        cout << endl << "Firmware Loopback Test: Disabled" << endl;
      }
      else
      {
        ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_START_TRIG_LOOP); //Enable loopback tests
        cout << endl << "Firmware Loopback Test: Enabled" << endl;
      }

      CheckFirmwareTestEnable();
    }

    else if (command == "ToggleCableTest")
    {
      if(tm_.GetTester("FirmwareTester", tmbN_)->GetTestStatus("Cable"))
      {
        ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_STOP_TRIG_CABLE); //Disable cable tests
        cout << endl << "Firmware Cable Test: Disabled" << endl;
      }
      else
      {
        ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_START_TRIG_CABLE); //Enable cable tests
        cout << endl << "Firmware Cable Test: Enabled" << endl;
      }

      CheckFirmwareTestEnable();
    }

    else if(command == "ToggleFiberTest")
    {
      if(tm_.GetTester("FirmwareTester", tmbN_)->GetTestStatus("Fiber"))
      {
        ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_STOP_TRIG_FIBER); //Disable fiber tests
        cout << endl << "Firmware Fiber Test: Disabled" << endl;
      }
      else
      {
        ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_START_TRIG_FIBER); //Enable fiber tests on TMB
        ccb->WriteRegister(CCB_VME_BC0, 1); //Initialize fiber transmitter
        //usleep(1000); //Reset the transmitter board during this time (waiting till Jason creates comminication between crate and transmitter)
        ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_VME_TMB_SOFT_RESET); //Reset fiber test counters
        cout << endl << "Firmware Fiber Test: Enabled" << endl;
      }

      CheckFirmwareTestEnable();
    }

    else if(command == "RunTest")
    {
      name = cgi.getElement("test_label");
      string test_label = cgi["test_label"]->getValue();
      tm_.GetTester("FirmwareTester", tmbN_)->RunTest(test_label);
    }
  }
  catch (emu::exception::Exception &e)
  {
    *out << e.toHTML();
  }

  this->FirmwareTestsPage(in,out);
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
    tm_.GetTestOutput("CCBBackplaneTester", tmbN_).str("");

    CCBBackplaneTestsPage(in, out);
    return;
  }

  //string tmb_slot = toolbox::toString("%d", sys_->tmbs()[tmbN_]->slot());
  std::stringstream file_name;
  file_name << "CCBBackplaneTests_Board" << tm_.GetBoardLabel() << "_" << time(NULL) << ".log";
  emu::utils::saveAsFileDialog(out, tm_.GetTestOutput("CCBBackplaneTester", tmbN_).str(), file_name.str());
}

void CCBBackplaneTestModule::FirmwareLogTestsOutput(xgi::Input * in, xgi::Output * out )
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
    tm_.GetTestOutput("FirmwareTester", tmbN_).str("");

    FirmwareTestsPage(in, out);
    return;
  }

  //string tmb_slot = toolbox::toString("%d", sys_->tmbs()[tmbN_]->slot());
  std::stringstream file_name;
  file_name << "FirmwareTests_Board" << tm_.GetBoardLabel() << "_" << time(NULL) << ".log";
  emu::utils::saveAsFileDialog(out, tm_.GetTestOutput("FirmwareTester", tmbN_).str(), file_name.str());
}

void CCBBackplaneTestModule::SetBoardLabel(xgi::Input * in, xgi::Output * out)
{
  cgicc::Cgicc cgi(in);
  cgicc::form_iterator name = cgi.getElement("label");

  string label;

  if(name != cgi.getElements().end())
  {
    label = cgi["label"]->getValue();
    cout << __func__ << ":  label " << label << endl;
    tm_.SetBoardLabel(label, tmbN_);
  }

  name = cgi.getElement("return");

  string returnPage;

  if(name != cgi.getElements().end())
  {
    returnPage = cgi["return"]->getValue();
    cout << __func__ << ":  returnPage " << returnPage << endl;

  }

  this->CCBBackplaneTestsPage(in,out);
}


}}  // namespaces
