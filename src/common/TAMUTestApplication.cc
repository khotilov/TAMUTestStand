/*
 * $Id: $
 */

// class header
#include "emu/pc/TAMUTestApplication.h"

// Emu includes
#include "emu/utils/Cgi.h"
#include "emu/utils/System.h"
#include "emu/pc/Crate.h"
#include "emu/pc/DAQMB.h"
#include "emu/pc/TMB.h"
#include "emu/pc/CCB.h"
#include "emu/pc/MPC.h"
#include "emu/pc/ALCTController.h"
#include "emu/pc/RAT.h"
#include "emu/pc/VMECC.h"
#include "emu/pc/DDU.h"

// XDAQ includes
#include "cgicc/Cgicc.h"
#include "cgicc/HTMLClasses.h"
#include "toolbox/string.h"

// system includes
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unistd.h> // for sleep()

namespace emu {  namespace pc {

using std::endl;
using std::cout;
using std::string;
using emu::utils::bindMemberMethod;


TAMUTestApplication::TAMUTestApplication(xdaq::ApplicationStub * s)
: xdaq::WebApplication(s)
, teststand_(this)
, tmbTestModule_(this, &teststand_)
, ccbBackplaneTestModule_(this, &teststand_)
, ccbBackplaneUtilsModule_(this, &teststand_)
, testResultsManagerModule_(this)
, xmlFile_("NOT_DEFINED.xml")
, tstoreFile_("NOT_DEFINED.xml")
, tmbSlot_(-1)
, parsed_(0)
{
  //------------------------------------------------------
  // bind methods
  //------------------------------------------------------
  xgi::bind(this, &TAMUTestApplication::Default, "Default");
  xgi::bind(this, &TAMUTestApplication::SetConfFile, "SetConfFile");

  xgi::bind(this, &TAMUTestApplication::CrateSelection, "CrateSelection");
  xgi::bind(this, &TAMUTestApplication::UploadConfFile, "UploadConfFile");
  xgi::bind(this, &TAMUTestApplication::ChooseConfigXMLPage, "ChooseConfigXMLPage");

  xgi::bind(this, &TAMUTestApplication::CrateContentsPage, "CrateContentsPage");

  xgi::bind(this, &TAMUTestApplication::LogOutput, "LogOutput");

  //------------------------------------------------------
  // bind methods of tmbTestUtils_
  //------------------------------------------------------
  bindMemberMethod(this, &tmbTestModule_, &TMBTestModule::TMBTestsPage, "TMBTestsPage");
  bindMemberMethod(this, &tmbTestModule_, &TMBTestModule::TMBRunTest, "TMBRunTest");
  bindMemberMethod(this, &tmbTestModule_, &TMBTestModule::TMBLogTestsOutput, "TMBLogTestsOutput");

  //------------------------------------------------------
  // bind methods of ccbBackplaneTestModule_
  //------------------------------------------------------
  bindMemberMethod(this, &ccbBackplaneTestModule_, &CCBBackplaneTestModule::CCBBackplaneTestsPage, "CCBBackplaneTestsPage");
  bindMemberMethod(this, &ccbBackplaneTestModule_, &CCBBackplaneTestModule::FirmwareTestsPage, "FirmwareTestsPage");
  bindMemberMethod(this, &ccbBackplaneTestModule_, &CCBBackplaneTestModule::CCBBackplaneRunTest, "CCBBackplaneRunTest");
  bindMemberMethod(this, &ccbBackplaneTestModule_, &CCBBackplaneTestModule::RunFirmwareCommand, "RunFirmwareCommand");
  bindMemberMethod(this, &ccbBackplaneTestModule_, &CCBBackplaneTestModule::CCBBackplaneLogTestsOutput, "CCBBackplaneLogTestsOutput");
  bindMemberMethod(this, &ccbBackplaneTestModule_, &CCBBackplaneTestModule::FirmwareLogTestsOutput, "FirmwareLogTestsOutput");
  bindMemberMethod(this, &ccbBackplaneTestModule_, &CCBBackplaneTestModule::TestLEDFrontPanel, "TestLEDFrontPanel");
  bindMemberMethod(this, &ccbBackplaneTestModule_, &CCBBackplaneTestModule::SetBoardLabel, "SetBoardLabel");

  //------------------------------------------------------
  // bind methods of ccbBackplaneUtilsModule_
  //------------------------------------------------------
  bindMemberMethod(this, &ccbBackplaneUtilsModule_, &CCBBackplaneUtilsModule::CCBLowLevelUtilsPage, "CCBLowLevelUtilsPage");
  bindMemberMethod(this, &ccbBackplaneUtilsModule_, &CCBBackplaneUtilsModule::ReadCCBRegister, "ReadCCBRegister");
  bindMemberMethod(this, &ccbBackplaneUtilsModule_, &CCBBackplaneUtilsModule::WriteCCBRegister, "WriteCCBRegister");
  bindMemberMethod(this, &ccbBackplaneUtilsModule_, &CCBBackplaneUtilsModule::HardReset, "HardReset");
  bindMemberMethod(this, &ccbBackplaneUtilsModule_, &CCBBackplaneUtilsModule::CCBConfig, "CCBConfig");
  bindMemberMethod(this, &ccbBackplaneUtilsModule_, &CCBBackplaneUtilsModule::CCBSignals, "CCBSignals");
  bindMemberMethod(this, &ccbBackplaneUtilsModule_, &CCBBackplaneUtilsModule::RunBackplaneCommand, "RunBackplaneCommand");

  //------------------------------------------------------
  // bind methods of testResultsManagerModule_
  //------------------------------------------------------
  bindMemberMethod(this, &testResultsManagerModule_, &TestResultsManagerModule::TestResultsManagerPage, "TestResultsManagerPage");
  bindMemberMethod(this, &testResultsManagerModule_, &TestResultsManagerModule::ProcessLogDirectory, "ProcessLogDirectory");
  bindMemberMethod(this, &testResultsManagerModule_, &TestResultsManagerModule::SetPageMode, "SetPageMode");

  //----------------------------
  // initialize variables
  //----------------------------

  xdata::InfoSpace *is = getApplicationInfoSpace();
  is->fireItemAvailable("XMLFileName", &xmlFile_);
  is->fireItemAvailable("TStoreFileName", &tstoreFile_);
  is->fireItemAvailable("TMBSlot", &tmbSlot_ );
}


void TAMUTestApplication::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  using cgicc::br;

  // perform possible environment variable expansions in configuration file names:
  xmlFile_.fromString(emu::utils::performExpansions(xmlFile_));
  tstoreFile_.fromString(emu::utils::performExpansions(tstoreFile_));

  cout << "Name of Logger is " << getApplicationLogger().getName() <<endl;
  LOG4CPLUS_INFO(getApplicationLogger(), "EmuPeripheralCrate ready");

  string urn = getApplicationDescriptor()->getURN();

  emu::utils::headerXdaq(out, this, "TMB testing through CCB and backplane");

  if(!parsed_) ParseXML();


  // links to tests, etc.
  if (teststand_.tmbs().size()>0 || teststand_.dmbs().size()>0)
  {
    *out << cgicc::fieldset() << endl;
    if(teststand_.ccb())
    {
      // a link that leads straight to the test page
      *out << cgicc::b()
        << cgicc::a("[TMB-CCB Backplane Tests]").set("href", "/" + urn + "/CCBBackplaneTestsPage" )
        << cgicc::b() << endl;

      // a link that leads straight to the continuous test page
      *out << cgicc::b()
        << cgicc::a("[TMB-CCB Firmware Tests]").set("href", "/" + urn + "/FirmwareTestsPage" )
        << cgicc::b() << endl;

      // low level TMB-CCB backplane tests
      *out << cgicc::b()
        << cgicc::a("[Low-level CCB Utilities]").set("href", "/" + urn + "/CCBLowLevelUtilsPage" )
        << cgicc::b() << endl;

      // a link that leads straight to the test results page
      *out << cgicc::b()
        << cgicc::a("[TMB-CCB Test Results]").set("href", "/" + urn + "/TestResultsManagerPage" )
        << cgicc::b() << endl;
    }

    // the inspect crate contents link
    *out << cgicc::a("[Crate contents]").set("href", "/" + urn + "/CrateContentsPage" );
    *out << cgicc::fieldset() << endl;

    //*out << cgicc::a("[test]").set("href", "/" + urn + "/TMBTestsPage" ) << endl << br();
    *out << br() << endl ;
  }


  // --- Crate info & selection

  *out << cgicc::fieldset() << endl;
  *out << cgicc::legend("Crate Info & Selection").set("style", "color:blue");

  int n_crates = teststand_.crates().size();

  int active_crates = 0;
  for(int i = 0; i < n_crates; i++)
    if(teststand_.crates()[i]->IsAlive()) active_crates++;
  *out << "Crates: total " << n_crates << ", " << cgicc::b()
    << " active " << active_crates << cgicc::b() << br() << br() << endl;

  *out << cgicc::form().set("action", "/" + urn + "/CrateSelection") << endl;

  *out << cgicc::b(cgicc::i("Current crate: ")) << endl;
  
  *out << cgicc::select().set("name", "runtype").set("onchange", "this.form.submit()") << endl;
  string CrateName;
  for (int i = 0; i < n_crates; ++i)
  {
    if (teststand_.crates()[i]->IsAlive())
      CrateName = teststand_.crates()[i]->GetLabel();
    else
      CrateName = teststand_.crates()[i]->GetLabel() + " NG";

    if (i == teststand_.crateN())
      *out << cgicc::option().set("value", CrateName).set("selected", "");
    else
      *out << cgicc::option().set("value", CrateName);

    *out << CrateName << cgicc::option() << endl;
  }
  *out << cgicc::select() << endl;
  
  *out << cgicc::form() << endl;

  *out << cgicc::fieldset() << endl;
  *out << br() << endl ;
     
  cout << "MainPage - # crates active/total: "<< std::dec << active_crates << "/" << teststand_.crates().size() << " Crates" << endl;

  // --- End select crate


  // an option to change the configuration XML
  *out << cgicc::fieldset() << endl;
  *out << cgicc::legend("Configuration").set("style", "color:blue");

  *out << cgicc::b(cgicc::i("Current configuration file ["))
    << cgicc::a("change").set("href", "/" + urn + "/ChooseConfigXMLPage" )
    << cgicc::b(cgicc::i("] :")) << br() << endl;
  *out << xmlFile_.toString() << br() << endl;

  *out << cgicc::fieldset() << endl;
  *out << br() << endl ;


  // logging
  if (teststand_.tmbs().size()>0 || teststand_.dmbs().size()>0)
  {
    *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
    *out << cgicc::legend("Logging").set("style", "color:blue") ;

    *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/LogOutput" ) << endl;
    *out << cgicc::input().set("type", "submit").set("value", "Log all output").set("name", "LogOutput") << endl;
    *out << cgicc::form() << endl;

    *out << cgicc::fieldset() << endl;
  }

  emu::utils::footer(out);
}


void TAMUTestApplication::CrateSelection(xgi::Input * in, xgi::Output * out )
{
  cgicc::Cgicc cgi(in);

  string in_value = cgi.getElement("runtype")->getValue();
  cout << "Select Crate " << in_value << endl;
  if (!in_value.empty())
  {
    int k = in_value.find(" ", 0);
    string value = (k) ? in_value.substr(0, k) : in_value;
    for (unsigned i = 0; i < teststand_.crates().size(); i++)
    {
      if (value == teststand_.crates()[i]->GetLabel())
      {
        teststand_.SetCurrentCrate(i);
        teststand_.useTMBInSlot(tmbSlot_);

        tmbTestModule_.Init();

        ccbBackplaneTestModule_.Init();

        break;
      }
    }
  }
  this->Default(in, out);
}


bool TAMUTestApplication::ParseXML()
{
  bool res = teststand_.Configure(xmlFile_, tstoreFile_);

  if (!res) return false;

  if (teststand_.crates().size() <= 0) return false;

  teststand_.SetCurrentCrate(0);
  teststand_.useTMBInSlot(tmbSlot_);

  tmbTestModule_.Init();
  ccbBackplaneTestModule_.Init();

  cout << "ParseXML is done" << endl;
  parsed_ = 1;
  return true;
}


//////////////////////////////////////////////////////////////////////////
// chonfig file chooser page
//////////////////////////////////////////////////////////////////////////

void TAMUTestApplication::ChooseConfigXMLPage(xgi::Input * in, xgi::Output * out)
{
  string urn = getApplicationDescriptor()->getURN();

  xgi::Utils::getPageHeader(out, "Change Configuration File", urn, "", "");

  *out << cgicc::fieldset().set("style", "font-size: 11pt;") << endl;
  *out << cgicc::legend("Upload Configuration...").set("style", "color:blue") << endl;

  *out << cgicc::form().set("method", "POST").set("action", "/" + urn + "/SetConfFile" ) << endl;
  *out << cgicc::input().set("type", "text")
      .set("name", "xmlFilename")
      .set("size", "90")
      .set("ENCTYPE","multipart/form-data")
      .set("value", xmlFile_) << endl;
  *out << cgicc::input().set("type", "submit")
      .set("value", "Set configuration file local") << endl;
  *out << cgicc::form() << endl;

  // Upload file...

  *out << cgicc::form().set("method", "POST")
      .set("enctype", "multipart/form-data").set("action", "/" + urn + "/UploadConfFile" ) << endl;
  *out << cgicc::input().set("type", "file")
      .set("name", "xmlFilenameUpload")
      .set("size", "90") << endl;
  *out << cgicc::input().set("type", "submit")
      .set("value", "Send") << endl;
  *out << cgicc::form() << endl << endl;

  *out << cgicc::fieldset() << endl;
}


void TAMUTestApplication::SetConfFile(xgi::Input * in, xgi::Output * out)
{
  try
  {
    cgicc::Cgicc cgi(in);
    cgicc::const_file_iterator file;
    file = cgi.getFile("xmlFileName");
    cout << "GetFiles string" << endl;
    if (file != cgi.getFiles().end()) (*file).writeToStream(cout);

    string XMLname = cgi["xmlFileName"]->getValue();
    cout << XMLname << endl;
    xmlFile_ = XMLname;

    ParseXML();
    this->Default(in, out);
  }
  catch (const std::exception & e)
  {
    //XECPT_RAISE(xgi::exception::Exception, e.what());
  }
}


void TAMUTestApplication::UploadConfFile(xgi::Input * in, xgi::Output * out )
{
  try
  {
    cout << "UploadConfFile action" << endl;
    cgicc::Cgicc cgi(in);
    cgicc::const_file_iterator file;

    file = cgi.getFile("xmlFileNameUpload");
    cout << "GetFiles" << endl;
    if (file != cgi.getFiles().end())
    {
      std::ofstream TextFile;
      TextFile.open("MyConfigurationFile.xml");
      (*file).writeToStream(TextFile);
      TextFile.close();
    }

    xmlFile_ = "MyConfigurationFile.xml";

    ParseXML();

    cout << "UploadConfFile done" << endl;

    this->Default(in, out);
  }
  catch (const std::exception & e)
  {
    //XECPT_RAISE(xgi::exception::Exception, e.what());
  }
}


//////////////////////////////////////////////////////////////////////////
// Crate contents page
//////////////////////////////////////////////////////////////////////////

void TAMUTestApplication::CrateContentsPage(xgi::Input * in, xgi::Output * out )
{
  using cgicc::td;

  if(!parsed_)
  {
    cout<<"Trying to display crate contents, but configuration is not parsed!" << endl;
    this->Default(in,out);
    return;
  }

  string crate_label = teststand_.crate()->GetLabel();

  cout << "CrateContentsPage: " << crate_label << endl;

  string urn = getApplicationDescriptor()->getURN();

  emu::utils::headerXdaq(out, this, "Contents of crate " + crate_label );

  if(teststand_.crate()->IsAlive())
  {
     *out << cgicc::h2("Contents of the Current Crate: "+ crate_label ) << endl;
  }
  else
  {
     *out << cgicc::span().set("style","color:red") << cgicc::h2("Current Crate: "+ crate_label + ",  Excluded") << cgicc::span() << endl;
  }

  *out << cgicc::fieldset().set("style","font-size: 11pt; background-color:#00FF00") << endl;

  std::set<int> slots_in_use;
  slots_in_use.insert(1);
  if(teststand_.ccb()) slots_in_use.insert(teststand_.ccb()->slot());
  if(teststand_.mpc()) slots_in_use.insert(teststand_.mpc()->slot());
  for (unsigned int i=0; i<teststand_.ddus().size(); i++)  slots_in_use.insert(teststand_.ddus()[i]->slot());
  for (unsigned int i=0; i<teststand_.tmbs().size(); i++)  slots_in_use.insert(teststand_.tmbs()[i]->slot());
  for (unsigned int i=0; i<teststand_.dmbs().size(); i++)  slots_in_use.insert(teststand_.dmbs()[i]->slot());

  for(int ii=1; ii<22; ii++)
  {
    // check if the slot is in use at all
    if (slots_in_use.find(ii) == slots_in_use.end()) continue;

    *out << cgicc::table().set("border","1");

    *out << td();
    *out << "Slot " << std::setfill('0') << std::setw(2) << std::dec << ii << endl;
    *out << td();

    // Display crate controller
    if(ii==1) *out << td() << "VME Crate Controller" << td();

    // Display CCB buttons
    int slot = -1;
    if(teststand_.ccb()) slot = teststand_.ccb()->slot();
    if(slot == ii)
    {
      *out << td() << "CCB" << td();

      *out << td();
      *out << cgicc::a("CCB Tests").set("href", "/" + urn + "/CCBLowLevelUtilsPage" ) << endl;
      *out << td();
    }

    // Display MPC if it exists
    slot = -1;
    if ( teststand_.mpc() ) slot = teststand_.mpc()->slot() ;
    if(slot == ii) *out << td() << "MPC" << td();

    // Display DDUs, if it exist
    for (unsigned int i=0; i<teststand_.ddus().size(); i++)
    {
      int slot = teststand_.ddus()[i]->slot();
      if(slot == ii) *out << td() << "DDU" << td();
    }

    // Display TMBs, if it exist
    for (unsigned int i=0; i<teststand_.tmbs().size(); i++)
    {
      int slot = teststand_.tmbs()[i]->slot();
      if(slot == ii)
      {
        *out << td() << "TMB / RAT / ALCT" << td();

        *out << td();
        *out << cgicc::a("TMB Tests")
            .set("href", toolbox::toString("/%s/TMBTestsPage?tmb=%d", urn.c_str(), i) ) << endl;
        *out << td();
      }
    }

    // Display DMBs, if it exist
    for (unsigned int i=0; i<teststand_.dmbs().size(); i++)
    {
      int slot = teststand_.dmbs()[i]->slot();
      if(slot == ii) *out << td() << "DMB / CFEB" << td();
    }

    *out << cgicc::table() << cgicc::br();
  }

  *out << cgicc::fieldset();

  emu::utils::footer(out);
}




//////////////////////////////////////////////////////////////////
// Logging information
///////////////////////////////////////////////////////////////////


void TAMUTestApplication::LogOutput(xgi::Input * in, xgi::Output * out)
{
  int initial_crate = teststand_.crateN();
  int initial_tmb = teststand_.tmbN();

  string fname = "/tmp/EmuPeripheralCrateLogFile_" + emu::utils::getDateTime(true) + ".log";

  cout << "Logging output to" << fname << endl;
  std::ofstream logFile;
  logFile.open(fname.c_str());

  std::ifstream xmlFile;
  xmlFile.open(xmlFile_.toString().c_str());
  while (xmlFile.good())  logFile << (char) xmlFile.get();
  xmlFile.close();

  logFile<<endl;

  for (size_t c = 0; c < teststand_.crates().size(); c++)
  {
    teststand_.SetCurrentCrate(c);
    for (unsigned t = 0; t < teststand_.tmbs().size(); t++)
    {
      logFile << tmbTestModule_.getTestOutput(t, c).str();
    }
  }

  teststand_.SetCurrentCrate(initial_crate);
  teststand_.SetCurrentTMB(initial_tmb);

  for (unsigned t = 0; t < teststand_.tmbs().size(); t++)
  {
    logFile << ccbBackplaneTestModule_.getTestOutput(t).str();
  }

  logFile.close();


  tmbTestModule_.Init();
  ccbBackplaneTestModule_.Init();

  this->Default(in, out);
}


}}  // namespaces


// factory instantion of XDAQ application
XDAQ_INSTANTIATOR_IMPL(emu::pc::TAMUTestApplication)
