/*
 * $Id: $
 */

// class header
#include "emu/pc/CCBBackplaneUtilsModule.h"

// Emu includes
#include "emu/pc/ConfigurablePCrates.h"
#include "emu/pc/CCBCommands.h"
#include "emu/pc/Crate.h"
#include "emu/pc/Chamber.h"
#include "emu/pc/TMB.h"
#include "emu/pc/CCB.h"
#include "emu/pc/RAT.h"
#include "emu/pc/ResultRegisterSerializer.h"
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
#include <sstream>
#include <iomanip>
#include <bitset>
#include <unistd.h> // for sleep()
#include <limits>

namespace emu {  namespace pc {

using std::endl;
using std::cout;
using std::string;
using std::hex;
using std::dec;
using std::bitset;


CCBBackplaneUtilsModule::CCBBackplaneUtilsModule(xdaq::WebApplication *app, ConfigurablePCrates *sys)
: app_(app)
, sys_(sys)
{
  CCBRegisterRead_ = -1;
  CCBRegisterValue_ = -1;
  CCBRegisterWrite_ = -1;
  CCBWriteValue_ = -1;
  CCBCmdWrite_ = CCBDataWrite_ = CCBBitsValue_ = -1;

  TMBSlot_ = 4;
  TMBStatus_ = std::numeric_limits<unsigned long long int>::max();
  
  TMBReservedBit_ = 0;

  Reserved5BitsWrite_ = Reserved5Bits_ = -1;
}


void CCBBackplaneUtilsModule::BackplaneTestsBlock(xgi::Input * in, xgi::Output * out )
{
  using cgicc::br;
  using cgicc::form;
  using cgicc::input;

  cgicc::Cgicc cgi(in);

  *out << cgicc::fieldset().set("style", "font-size: 11pt; font-family: arial;") << endl;
  *out << cgicc::legend("CCB Backplane Utils for TMB TestStand").set("style", "color:blue") << endl;

  string actionURL = "/" + app_->getApplicationDescriptor()->getURN() + "/RunBackplaneCommand";

  *out << form().set("method", "GET").set("action", actionURL) << endl;

  *out << cgicc::b("VME interface commands:") << " times to repeat " << cgicc::select().set("name", "nrepeat") << endl;
  string nrepeats[] = { "1", "2", "3", "4", "10", "32", "256" };
  const int nentries = sizeof(nrepeats) / sizeof(nrepeats[0]);

  string selected_nrepeat = "1";
  if (cgi.getElement("nrepeat") != cgi.getElements().end()) selected_nrepeat = cgi["nrepeat"]->getValue();

  for (int i = 0; i < nentries; ++i)
  {
    if (nrepeats[i] == selected_nrepeat)
    {
      *out << cgicc::option().set("value", nrepeats[i]).set("selected", "");
    }
    else
    {
      *out << cgicc::option().set("value", nrepeats[i]);
    }
    string entry = nrepeats[i] + " ";
    if (i == 0) entry += "time";
    if (i == 1) entry += "times";
    *out << entry << cgicc::option() << endl;
  }
  *out << cgicc::select() << br() << endl;

  *out << " &nbsp; " << input().set("type", "submit").set("name", "command").set("value", "BC0");
  *out << " generate BC0 pulse (P4: connectors 21,22)" << br() << endl;

  *out << " &nbsp; " << input().set("type", "submit").set("name", "command").set("value", "L1A");
  *out << " generate L1ACC pulse (P4: connectors 23,24)" << br() << endl;

  *out << " &nbsp; " << input().set("type", "submit").set("name", "command").set("value", "L1Reset");
  *out << " generate L1Reset pulse (P4: connectors 29,30)" << br() << endl;

  *out << form() << endl;


  *out << cgicc::p() << cgicc::p() << endl;

  char buf[200];
  *out << cgicc::b("Writing to command & data buses, etc.:") << br() << endl;

  *out << form().set("method", "GET").set("action", actionURL) << endl;
  *out << " &nbsp; " << "Write command (hex) " << endl;
  sprintf(buf, "%01X", CCBCmdWrite_);
  *out << input().set("type", "text").set("style", "width:2em").set("maxlength", "2").set("value", buf).set("name", "csrb2_command") << endl;
  *out << input().set("type", "submit").set("name", "command").set("value", "CCB_cmd_strobe") << br() << endl;

  *out << " &nbsp; " << "Write data (hex) " << endl;
  sprintf(buf, "%01X", CCBDataWrite_);
  *out << input().set("type", "text").set("style", "width:2em").set("maxlength", "2").set("value", buf).set("name", "csrb2_data") << endl;
  *out << input().set("type", "submit").set("name", "command").set("value", "CCB_data_strobe") << br() << endl;

  *out << " &nbsp; " << "Write TMB_reserved[0] bit (CSRA6 bit #2)  " << endl;
  sprintf(buf, "%01d", TMBReservedBit_);
  *out << input().set("type", "text").set("style", "width:2em").set("maxlength", "1").set("value", buf).set("name", "TMBReservedBit") << endl;
  *out << input().set("type", "submit").set("name", "command").set("value", "TMB_reserved") << br() << endl;
  *out << form() << endl << br() << endl;



  *out << cgicc::b("Reading through TMB Result Register (RR):") << br() << endl;

  *out << form().set("method", "GET").set("action", actionURL) << endl;
  *out << "Load TMB RR with:" << br() << endl;
  *out << " &nbsp; " << input().set("type", "submit").set("name", "command").set("value", "data bus -> RR") << endl;
  *out << " contents of CSRB3 data bus" << br() << endl;
  *out << " &nbsp; " << input().set("type", "submit").set("name", "command").set("value", "counter -> RR") << endl;
  *out << " total pulse counter" << br() << endl;
  *out << " &nbsp; " << input().set("type", "submit").set("name", "command").set("value", "counter flags -> RR") << endl;
  *out << " pulse counters flags" << br() << endl;
  *out << " &nbsp; " << input().set("type", "submit").set("name", "command").set("value", "ccb_rx0 -> RR") << endl;
  *out << " CCB rx0 counter" << br() << endl;
  *out << form() << endl;

  *out << form().set("method", "GET").set("action", actionURL) << endl;
  *out << "Read RR of TMB in slot " << endl;
  sprintf(buf, "%d", TMBSlot_);
  *out << input().set("type", "text").set("style", "width:2em").set("maxlength", "2").set("value", buf).set("name", "tmb_slot") << endl;
  *out << input().set("type", "submit").set("name", "command").set("value", "Read TMB RR") << endl;
  // only write out bit values if they were requested:
  if (TMBStatus_ < std::numeric_limits<unsigned long long int>::max())
  {
    unsigned int command_bus = TMBStatus_ & 0xFFULL; // first 8 bits
    unsigned long long int tmb_counter= TMBStatus_ >> 8; // the rest
    *out << " &nbsp; slot "<<TMBSlot_<<": 0x" << hex << TMBStatus_ << dec << " = " << tmb_counter << ".0x" << hex << command_bus << dec << " = "
        << bitset< 20 >(TMBStatus_) << " = " << bitset< 12 >(tmb_counter & 0xFFF) << "." << bitset< 8 >(command_bus) << endl;
  }
  *out << form() << endl << br() << endl;

  *out << cgicc::b("Write 5 DMB_reserved_out bits and read them back through TMB_reserved_in:") << br() << endl;
  *out << form().set("method", "GET").set("action", actionURL) << endl;
  *out << "Value to write (hex 0-1F): " << endl;
  sprintf(buf, "%02X", Reserved5BitsWrite_);
  *out << input().set("type", "text").set("style", "width:2em").set("maxlength", "2").set("value", buf).set("name", "write_5bits") << endl;
  *out << input().set("type", "submit").set("name", "command").set("value", "Write-Read Reserved Bits") << endl;
  // only write out if they were requested:
  if (Reserved5Bits_ >= 0)
  {
    *out << " &nbsp; wrote "<< hex << Reserved5BitsWrite_ << " read "<< Reserved5Bits_ << cgicc::b()
      << ((Reserved5Bits_ == Reserved5BitsWrite_) ? " OK" : " PROBLEM!") << cgicc::b() << dec << endl;
  }
  *out << form() << endl << br() << endl;


  *out << form().set("method", "GET").set("action", actionURL) << endl;
  *out << "Read cfg_done status bits for all TMBs : " << endl;
  string tmb_slots[] = { "2", "4", "6", "8", "10", "14", "16", "18", "20" };
  const int n_tmb_slots = sizeof(tmb_slots) / sizeof(tmb_slots[0]);
  *out << input().set("type", "submit").set("name", "command").set("value", "Read TMBs bits") << endl;
  // only write out bit values if they were requested:
  if (CCBBitsValue_ > 0)
  {
    *out << " &nbsp; " << cgicc::b("slot") << ":[ALCT bit][TMB bit] &nbsp; ";
    for (int i = 0; i < n_tmb_slots; ++i)
      *out << " &nbsp;&nbsp; " << cgicc::b() << tmb_slots[i] << cgicc::b() << ":" << ((CCBBitsValue_ >> (i * 2)) & 1)
          << ((CCBBitsValue_ >> (i * 2 + 1)) & 1);
    *out << endl;
  }
  *out << form() << endl;

  *out << cgicc::fieldset();
}


void CCBBackplaneUtilsModule::RunBackplaneCommand(xgi::Input * in, xgi::Output * out)
{
  cgicc::Cgicc cgi(in);

  CCB * ccb = sys_->ccb();

  if (ccb == 0)
  {
    XCEPT_RAISE(emu::exception::CCBException, "CCBBackplaneUtilsModule::BackplaneTestsBlock: ccb == 0" );
  }

  // Make sure that CCB is in FPGA mode
  SetFPGAMode(ccb);

  cgicc::form_iterator icommand = cgi.getElement("command");
  cgicc::form_iterator intimes = cgi.getElement("nrepeat");

  string command = "";
  int ntimes = 0;

  if (icommand != cgi.getElements().end()) command = cgi["command"]->getValue();
  if (intimes != cgi.getElements().end()) ntimes = atoi(cgi["nrepeat"]->getValue().c_str());
  cout << "CCBUtilsTestStandCommand: " << command << " x" << ntimes << endl;


  if (ntimes > 0) // VME interface commands
  {
    if (command == "L1A")
    {
      NTimesWriteRegister(ccb, ntimes, CCB_VME_L1ACC, 1); // write anything
    }
    else if (command == "BC0")
    {
      NTimesWriteRegister(ccb, ntimes, CCB_VME_BC0, 1); // write anything
    }
    else if (command == "L1Reset")
    {
      NTimesWriteRegister(ccb, ntimes, CCB_VME_L1RESET, 1); // write anything
    }
  }
  else // non-repeatable commands
  {
    if (command == "CCB_cmd_strobe")
    {
      CCBCmdWrite_ = -1;
      if (cgi.getElement("csrb2_command") != cgi.getElements().end())
      {
        CCBCmdWrite_ = strtol(cgi["csrb2_command"]->getValue().c_str(), NULL, 16);
      }

      if (CCBCmdWrite_ >= 0 && CCBCmdWrite_ < 257)
      {
        cout << "Write CCB_cmd_strobe " << hex << CCBCmdWrite_ << dec << endl;
        ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCBCmdWrite_); // CSRB2 - write command
      }
    }
    else if (command == "CCB_data_strobe")
    {
      CCBDataWrite_ = -1;
      if (cgi.getElement("csrb2_data") != cgi.getElements().end())
      {
        CCBDataWrite_ = strtol(cgi["csrb2_data"]->getValue().c_str(), NULL, 16);
      }

      if (CCBDataWrite_ >= 0 && CCBDataWrite_ < 257)
      {
        cout << "Write CCB_data_strobe " << hex << CCBDataWrite_ << dec << endl;
        ccb->WriteRegister(CCB_CSRB3_DATA_BUS, CCBDataWrite_); // CSRB3 - write data
      }
    }
    else if (command == "Read TMBs bits")
    {
      // read current status bits for all TMB slots
      int csra2 = ccb->ReadRegister(CCB_CSRA2_STATUS);
      int csra3 = ccb->ReadRegister(CCB_CSRA3_STATUS);
      cout << "CCB read TMB cfg_done: " << hex << csra2 << " " << csra3 << dec << endl;
      cout << "   " << bitset< 17 >(csra2) << " " << bitset< 17 >(csra3) << endl;

      CCBBitsValue_ = 0;

      // ALCT_cfg_done bits:
      csra2 >>= 1;
      for (int i = 0; i < 9; ++i)
        if (csra2 & (1 << i)) CCBBitsValue_ |= (1 << i * 2);

      // TMB_cfg_done bits:
      int tcsra2 = (csra2 >> 9);
      tcsra2 &= 0x3F;
      csra3 &= 0x07;
      csra3 <<= 6;
      csra3 |= tcsra2;
      for (int i = 0; i < 9; ++i)
        if (csra3 & (1 << i)) CCBBitsValue_ |= (1 << i * 2 + 1);
      cout << "CCB read TMB cfg_done: " << hex << csra2 << " " << csra3 << " -> " << CCBBitsValue_ << dec << endl;
      cout << "   " << bitset< 17 >(tcsra2) << " " << bitset< 17 >(csra3) << " " << bitset< 32 >(CCBBitsValue_) << endl;
    }
    else if (command == "Read TMB RR")
    {
      TMBSlot_ = 4;
      if (cgi.getElement("tmb_slot") != cgi.getElements().end())
      {
        TMBSlot_ = cgi["tmb_slot"]->getIntegerValue(2, 20);
      }

      ResultRegisterSerializer reader(ccb, TMBSlot_);
      TMBStatus_ = reader.read();
    }
    else if (command == "data bus -> RR")
    {
      // custom command through CSRB2: load CSRA3 data bus contents into TMB result register
      ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_RR_LOAD_DATA_BUS);
    }
    else if (command == "counter -> RR")
    {
      // custom command through CSRB2: load pulse counter into TMB result register
      ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_RR_LOAD_COUNTER);
    }
    else if (command == "counter flags -> RR")
    {
      // custom command through CSRB2: load pulse counters flags into TMB result register
      ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_RR_LOAD_COUNTERS_FLAG);
    }
    else if (command == "ccb_rx0 -> RR")
    {
      // custom command through CSRB2: load CCB rx0 counter into TMB result register
      ccb->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_RR_LOAD_CCB_RX0);
    }
    else if (command == "Write-Read Reserved Bits")
    {
      // Write 5 DMB_reserved_out bits and read them back through TMB_reserved_in
      if (cgi.getElement("write_5bits") != cgi.getElements().end())
      {
        Reserved5BitsWrite_ = strtol(cgi["write_5bits"]->getValue().c_str(), NULL, 16);

        // write to DMB_reserved_out
        Write5ReservedBits(ccb, Reserved5BitsWrite_);

        usleep(50);

        // read from TMB_reserved_in bits
        Reserved5Bits_ = Read5ReservedBits(ccb);
      }
    }
    
    else if (command == "TMB_reserved")
    {
      if (cgi.getElement("TMBReservedBit") != cgi.getElements().end())
      {
        TMBReservedBit_ = cgi["TMBReservedBit"]->getIntegerValue();
      }

      // write bit #2 to CSRB6
      WriteTMBReserved0Bit(ccb, (TMBReservedBit_ == 0 ) ? 0 : 4 );
    }


  }

  this->CCBLowLevelUtilsPage(in, out);
}


void CCBBackplaneUtilsModule::CCBLowLevelUtilsPage(xgi::Input * in, xgi::Output * out)
{
  using cgicc::br;
  using cgicc::form;
  using cgicc::input;

  string urn = app_->getApplicationDescriptor()->getURN();

  string Name = toolbox::toString("CCB utilities, crate=%s, slot=%d", sys_->crate()->GetLabel().c_str(), sys_->ccb()->slot());

  emu::utils::headerXdaq(out, app_, Name);

  char buf[100];

  // test-stand related backplane utilities block
  BackplaneTestsBlock(in, out);

  *out << cgicc::fieldset().set("style", "font-size: 11pt; font-family: arial;") << endl;
  *out << cgicc::legend("CCB Utils").set("style", "color:blue") << endl;

  *out << form().set("method", "GET").set("action", "/" + urn + "/ReadCCBRegister" ) << endl;
  *out << "Read Register (hex) " << endl;
  sprintf(buf, "%04X", CCBRegisterRead_);
  *out << input().set("type", "text").set("value", buf).set("name", "CCBRegister") << endl;
  *out << input().set("type", "submit").set("value", "Read CCB") << endl;
  *out << " Register value (hex): " << hex << CCBRegisterValue_ << dec << endl;
  *out << form() << endl;

  *out << form().set("method", "GET").set("action", "/" + urn + "/WriteCCBRegister" ) << endl;
  *out << "Write Register (hex) " << endl;
  sprintf(buf, "%04X", CCBRegisterWrite_);
  *out << input().set("type", "text").set("value", buf).set("name", "CCBRegister") << endl;

  *out << "Register value (hex) " << endl;
  sprintf(buf, "%04X", CCBWriteValue_);
  *out << input().set("type", "text").set("value", buf).set("name", "CCBValue") << endl;
  *out << input().set("type", "submit").set("value", "Write CCB") << endl;
  *out << form() << br() << endl;

  //*out << cgicc::a("[Read TTCrx Registers]").set("href", "/" + urn + "/ReadTTCRegister" ).set("target", "_blank") << endl;
  //*out << br() << br() << endl;

  // Begin select signal Config listbox

  std::vector< string > SignalName;
  SignalName.push_back(" ");
  SignalName.push_back("BC0"); // 1
  SignalName.push_back("OC0");
  SignalName.push_back("L1 Reset (Resync)");
  SignalName.push_back("Hard Reset");
  SignalName.push_back(" "); // 5
  SignalName.push_back("Start Trigger");
  SignalName.push_back("Stop Trigger");
  SignalName.push_back("Test Enable");
  SignalName.push_back("Private Gap");
  SignalName.push_back("Private Orbit"); // 0A
  SignalName.push_back(" ");
  SignalName.push_back(" ");
  SignalName.push_back(" ");
  SignalName.push_back(" ");
  SignalName.push_back("CCB hard reset"); // 0f
  SignalName.push_back("TMB hard reset"); // 10
  SignalName.push_back("ALCT hard reset");
  SignalName.push_back("DMB hard reset");
  SignalName.push_back("MPC hard reset");
  SignalName.push_back("DMB CFEB calibrate0"); // 14
  SignalName.push_back("DMB CFEB calibrate1"); // 15
  SignalName.push_back("DMB CFEB calibrate2"); // 16
  SignalName.push_back("DMB CFEB initiate");
  SignalName.push_back("ALCT ADB pulse Sync"); // 18
  SignalName.push_back("ALCT ADB pulse Async"); // 19
  SignalName.push_back("CLCT ext trigger");
  SignalName.push_back("ALCT ext trigger");
  SignalName.push_back("Soft Reset"); // 1C
  SignalName.push_back("DMB soft reset");
  SignalName.push_back("TMB soft reset");
  SignalName.push_back("MPC soft reset"); // 1F

  *out << form().set("action", "/" + urn + "/CCBSignals") << endl;

  int n_keys = SignalName.size();

  *out << "Choose Singal: " << endl;
  *out << cgicc::select().set("name", "cmd_signal") << endl;

  int selected_index = 0;
  char sbuf[20];
  for (int i = 0; i < n_keys; ++i)
  {
    sprintf(sbuf, "%d", i);
    if (i == selected_index)
    {
      *out << cgicc::option().set("value", sbuf).set("selected", "");
    }
    else
    {
      *out << cgicc::option().set("value", sbuf);
    }
    *out << SignalName[i] << cgicc::option() << endl;
  }

  *out << cgicc::select() << endl;

  *out << input().set("type", "submit").set("name", "command").set("value", "Generate Signal") << endl;
  *out << form() << br() << endl;

  //End select signal

  *out << form().set("method", "GET").set("action", "/" + urn + "/CCBConfig" ) << endl;
  *out << input().set("type", "submit").set("value", "CCB Configure");
  *out << form() << endl << br() << endl;

  *out << form().set("method", "GET").set("action", "/" + urn + "/HardReset" ) << endl;
  *out << input().set("type", "submit").set("value", "HardReset");
  *out << form() << endl;

  *out << cgicc::fieldset();

  emu::utils::footer(out);
}


void CCBBackplaneUtilsModule::ReadCCBRegister(xgi::Input * in, xgi::Output * out )
{
  cgicc::Cgicc cgi(in);

  int CCBregister = -1;
  if(xgi::Utils::hasFormElement(cgi, "CCBRegister"))
  {
    CCBregister = strtol(cgi["CCBRegister"]->getValue().c_str(), NULL, 16);
  }

  if(CCBregister != -1)
  {
    CCBRegisterRead_ = CCBregister;
    cout << "CCB read Register: " << hex << CCBregister << dec << endl;
    CCBRegisterValue_ = sys_->ccb()->ReadRegister(CCBregister);
  }
  this->CCBLowLevelUtilsPage(in, out);
}


void CCBBackplaneUtilsModule::WriteCCBRegister(xgi::Input * in, xgi::Output * out )
{
  cgicc::Cgicc cgi(in);

  int CCBregister = -1;
  if(xgi::Utils::hasFormElement(cgi, "CCBRegister"))
  {
    CCBregister = strtol(cgi["CCBRegister"]->getValue().c_str(), NULL, 16);
  }

  int CCBvalue = -1;
  if(xgi::Utils::hasFormElement(cgi, "CCBValue"))
  {
    CCBvalue = strtol(cgi["CCBValue"]->getValue().c_str(), NULL, 16);
  }

  if( CCBregister != -1 && CCBvalue != -1)
  {
    CCBRegisterWrite_ = CCBregister;
    CCBWriteValue_ = CCBvalue;
    cout << "CCB write Register: " << hex << CCBregister << ", Value: " << CCBvalue << dec << endl;
    sys_->ccb()->WriteRegister(CCBregister, CCBvalue);
  }
  this->CCBLowLevelUtilsPage(in, out);
}


void CCBBackplaneUtilsModule::ReadTTCRegister(xgi::Input * in, xgi::Output * out )
{
  std::ostringstream OutputTxt;
  OutputTxt.str(""); //clear the output string
  sys_->ccb()->RedirectOutput(&OutputTxt);
  sys_->ccb()->PrintTTCrxRegs();
  sys_->ccb()->RedirectOutput(&cout);
  *out << OutputTxt.str();
}


void CCBBackplaneUtilsModule::HardReset(xgi::Input * in, xgi::Output * out )
{
  cout << "hardReset" << endl;
  sys_->ccb()->hardReset();
  this->CCBLowLevelUtilsPage(in,out);
}


void CCBBackplaneUtilsModule::CCBConfig(xgi::Input * in, xgi::Output * out )
{
  sys_->ccb()->configure();
  this->CCBLowLevelUtilsPage(in,out);
}


void CCBBackplaneUtilsModule::CCBSignals(xgi::Input * in, xgi::Output * out )
{
   cgicc::Cgicc cgi(in);

   string in_value = cgi.getElement("cmd_signal")->getValue();
   int sig=atoi(in_value.c_str());
   cout << "Generating CCB signal: " << sig << endl;

   if(sig > 0 && sig <= 0x3F) sys_->ccb()->signal_csrb2(sig);
   this->CCBLowLevelUtilsPage(in,out);
}


}}  // namespaces
