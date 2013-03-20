/*
 * $Id: $
 */

// class header
#include "emu/pc/FirmwareTester.h"

// Emu includes
#include "emu/pc/TMB.h"
#include "emu/pc/CCB.h"
#include "emu/pc/CCBCommands.h"
#include "emu/pc/TestUtils.h"

// system includes
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <unistd.h> // for sleep()
#include <boost/bind.hpp>


namespace emu { namespace pc {

using std::endl;
using std::cout;
using std::string;
using std::set;
using std::hex;
using std::dec;
using std::boolalpha;
using std::map;


FirmwareTester::FirmwareTester()
{
  RegisterTestProcedures();
}


FirmwareTester::~FirmwareTester(){}


FirmwareTester::FirmwareTester(const FirmwareTester &other)
{
  CopyFrom(other);
}


FirmwareTester & FirmwareTester::operator=(const FirmwareTester &rhs)
{
  CopyFrom(rhs);
  return *this;
}


void FirmwareTester::CopyFrom(const FirmwareTester &other)
{
  // the real need for the user defined copy c-tor and assignment comes from here:
  // we need to make sure that the test procedures are properly bound to this object.
  RegisterTestProcedures();
}


void FirmwareTester::RegisterTestProcedures()
{
  cout<<__PRETTY_FUNCTION__<<endl;
  RegisterTheTest("CheckFirmwareTestStatus", boost::bind( &FirmwareTester::CheckFirmwareTestStatus, this));
  RegisterTheTest("TestDMBLoopback", boost::bind( &FirmwareTester::TestDMBLoopback, this));
  RegisterTheTest("TestRPCLoopback", boost::bind( &FirmwareTester::TestRPCLoopback, this));
  RegisterTheTest("TestCableConnector", boost::bind( &FirmwareTester::TestCableConnector, this));
  RegisterTheTest("TestFiberConnector", boost::bind( &FirmwareTester::TestFiberConnector, this));

  SetTestStatus("Loopback", -1);
  SetTestStatus("Cable", -1);
  SetTestStatus("Fiber", -1);
}


////////////////////////////////////////////////////
// Actual tests:
////////////////////////////////////////////////////

int FirmwareTester::CheckStatusLoopback(int const * command_array, int const length_command_array, string label)
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;
  const int error_count_command = command_array[0];

  LoopErrorCount r;
  r.r = LoadAndReadResultRegister(ccb_, tmb_->slot(), error_count_command);
  int test_count = ResultRegisterData(LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_LOOP_TEST_COUNT));
  cout << label <<"_loopback Error Count: " << dec << r.error_count << endl;
  cout << "Test Count: " << test_count << endl;
  cout << "Counter Overflow: "<< boolalpha << r.overflow << endl;

  ok &= CompareValues(out(), test + " " + label + " error count", r.error_count, 0, true);
  ok &= CompareValues(out(), test + " " + label+ " counter overflow", r.overflow, 0, true);

  if(r.error_count!=0) //Indicates errors
  {
   /* // Report error count and counter overflow status
    if(r.overflow)
      out() << label << " Loopback Error Count: " << dec << r.error_count << " Counter Overflow!" << "\n";
    else
      out() << label << "Loopback Error Count: " << dec << r.error_count << "\n";*/

    //out() << "\tErrors from bits: ";
    // Walk over range of commands
    for(int command_index = 1; command_index < length_command_array; ++command_index)
    {
      // Get status flags
      uint32_t stat = ResultRegisterData(LoadAndReadResultRegister(ccb_, tmb_->slot(), command_array[command_index]));

      std::stringstream stat_label;
      stat_label << label << "LOOP" << command_index << "_STAT";

      ok &= CompareValues(out(), " " + stat_label.str(), stat, 0, true);

      // Begin log of status register
      cout << "\t" << stat_label.str() << ": ";

      // Walk over status bits
      for(uint32_t stat_index=0; stat_index < TMB_RR_DATA_WIDTH; ++stat_index)
      {
        // Check for individual bit errors
        if((stat>>stat_index) & 0x1)
        {
          ok &= false;
          //out() << (command_index-1)*TMB_RR_DATA_WIDTH + stat_index << ' ';
          cout << '1';
        }
        else
        {
          cout << '0';
        }
      }
      cout << endl;

    }
    //out() << endl;

  }
  cout << endl;

  int errcode = (ok == false);
  //MessageOK(cout, test + " result", errcode);
  return errcode;
}

int FirmwareTester::TemplateTestLoopback(int const * command_array, int const length_command_array, string label, string test)
{
  bool ok = true;
  unsigned int sleep_time = 1;
  const int Niter = 24;

  // Start test
  ccb_->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_START_TRIG_LOOP);
  for (int i=0; i<Niter; ++i)
  {
    usleep(sleep_time);
    ok &= !(CheckStatusLoopback(command_array, length_command_array, label));
    sleep_time*=2;
  }

  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}

int FirmwareTester::TestDMBLoopback()
{
  string test = TestLabelFromProcedureName(__func__);
  int errcode = 0;
  int tmp_errcode = 0;

  L1Reset();
  usleep(10);
  tmp_errcode = TemplateTestLoopback(DMBLOOP_COMMANDS, LENGTH_DMBLOOP_COMMANDS, "DMB", test);
  errcode |= tmp_errcode;
  out() << "DMBLoopback Test With Fiber OFF -> " << (tmp_errcode ? "FAIL":"PASS") << endl;

  return errcode;
}

int FirmwareTester::TestRPCLoopback()
{
  string test = TestLabelFromProcedureName(__func__);
  int errcode = 0;
  int tmp_errcode = 0;

  L1Reset();
  tmp_errcode = TemplateTestLoopback(RPCLOOP_COMMANDS, LENGTH_RPCLOOP_COMMANDS, "RPC", test);
  errcode |= tmp_errcode;
  out() << "RPCLoopback Test With Fiber OFF -> " << (tmp_errcode ? "FAIL":"PASS") << endl;

  L1Reset();
  ccb_->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_START_TRIG_FIBER);
  tmp_errcode = TemplateTestLoopback(RPCLOOP_COMMANDS, LENGTH_RPCLOOP_COMMANDS, "RPC", test);
  errcode |= tmp_errcode;
  out() << "RPCLoopback Test With Fiber ON -> " << (tmp_errcode ? "FAIL":"PASS") << endl;

  L1Reset();
  tmp_errcode = TemplateTestLoopback(RPCLOOP_SLOW_COMMANDS, LENGTH_RPCLOOP_SLOW_COMMANDS, "RPCSlow", test);
  errcode |= tmp_errcode;
  out() << "RPCLoopbackSlow Test With Fiber OFF -> " << (tmp_errcode ? "FAIL":"PASS") << endl;

  L1Reset();
  ccb_->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_START_TRIG_FIBER);
  tmp_errcode = TemplateTestLoopback(RPCLOOP_SLOW_COMMANDS, LENGTH_RPCLOOP_SLOW_COMMANDS, "RPCSlow", test);
  errcode |= tmp_errcode;
  out() << "RPCLoopbackSlow Test With Fiber ON -> " << (tmp_errcode ? "FAIL":"PASS") << endl;

  return errcode;
}

int FirmwareTester::CheckStatusConnector(int const * command_array, int const length_command_array, string label, bool report = true)
{
  bool ok = true;
  const int load_stat_command = command_array[0];

  // Indicates which bits are enabled
  uint32_t connector_stat = ResultRegisterData(LoadAndReadResultRegister(ccb_, tmb_->slot(), load_stat_command));
  //cout << label << " Connector Stat: ";

  // Walk over bits in connector_stat
  /*
  for(unsigned int j=0; j < TMB_RR_DATA_WIDTH; ++j)
  {
    if(connector_stat&(0x1<<j))
      cout << '1';
    else
      cout << '0';
  }
  cout << endl;
  */

  for(int stat_index=1; stat_index < length_command_array; ++stat_index)
  {
    int errorcount = ResultRegisterData(LoadAndReadResultRegister(ccb_, tmb_->slot(), command_array[stat_index]));
    bool on = (connector_stat>>stat_index) & 0x1;
    ok &= (on && !errorcount && (connector_stat & 0x1));
    if(report)
    {
      out() << label << stat_index << ":";
      if(on)
        out() << "ON";
      else
        out() << "OFF";
      out() << ": Error Count: "<< dec << errorcount << endl;
    }
    //cout << '\t' << label << stat_index << " Error Count: "<< dec << errorcount << endl;
  }

  int errcode = (ok == false);
  //MessageOK(cout, test + " result", errcode);
  return errcode;
}


int FirmwareTester::TemplateTestConnector(int const start_command, int const * command_array, int const length_command_array, string label, string test)
{
  bool ok = true;
  unsigned int sleeptime = 1;
  const int Niter = 24;

  // Start timer
  emu::utils::SimpleTimer timer;
  L1Reset();
  // Start test
  ccb_->WriteRegister(CCB_CSRB2_COMMAND_BUS, start_command);
  ccb_->WriteRegister(CCB_VME_BC0, 1);
  for (int i=0; i<Niter; ++i)
  {
    usleep(sleeptime);
    ok &= !(CheckStatusConnector(command_array, length_command_array, label));
    sleeptime*=2;
  }

  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}

int FirmwareTester::TestCableConnector()
{
  string test = TestLabelFromProcedureName(__func__);
  int errcode = TemplateTestConnector(CCB_COM_START_TRIG_CABLE, CABLE_COMMANDS, LENGTH_CABLE_COMMANDS, "Cable", test);
  return errcode;
}

/*
 * Tests Fiber Connection. Requires bench mezz for communication.
 * Note that switch 8 must be on to enable the test, and that switch 7 alters the function of the push button (L1Reset and Force Error)
 */
int FirmwareTester::TestFiberConnector()
{
  int error = 0;
  int errcode = 0;
  int Niter = 24;
  string test = TestLabelFromProcedureName(__func__);

  L1Reset();

  // Enable fiber link
  ccb_->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_START_TRIG_FIBER);
  ccb_->WriteRegister(CCB_VME_BC0, 1);

  // Get fiber status
  uint32_t connector_stat = ResultRegisterData(LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_FIBER_STAT)); // Pass if all 1's
  uint32_t invalid_stat = ResultRegisterData(LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_FIBER_INVALID_STAT)); // Pass if all 0's

  // Check fibers are active
  cout << connector_stat << endl;
  cout << (0xFF) << endl;
  if(connector_stat & 0xFF == (0xFF)) // Check if all 1's
  {
    out() << "Fiber Test: Enabled" << endl;
    out() << "All Links Active: True" << endl;
  }
  else if(connector_stat & 0x1 == false)
  {
    out() << "Fiber Test: Disabled" << endl;
    errcode = 1;
    return errcode;
  }
  else
  {
    out() << "Fiber Test: Enabled" << endl;
    out() << "All Links Active: False" << endl;
    out() << "Fiber Status: " << hex << connector_stat << endl;
    out() << "Fiber Invalid Status: " << hex << invalid_stat << endl;
    errcode = 1;
    return errcode;
  }

  // Run checking procedure
  int sleeptime = 1;
  error |= CheckStatusConnector(FIBER_COMMANDS, LENGTH_FIBER_COMMANDS, "Fiber", true);
  for (int i=0; i<Niter; ++i)
  {
    usleep(sleeptime);
    error |= CheckStatusConnector(FIBER_COMMANDS, LENGTH_FIBER_COMMANDS, "Fiber", error);
    sleeptime*=2;
  }
  if(error)
  {
    errcode = 1;
    return errcode;
  }

  // Attempt to generate an error on a single fiber
  Niter = 100;
  for(int i=0; i<Niter && !error; ++i)
  {
    bool first_error = false;
    ccb_->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_FORCE_FIBER_ERROR);
    for(int stat_index=1; stat_index < LENGTH_FIBER_COMMANDS; ++stat_index)
    {
      int errorcount = ResultRegisterData(LoadAndReadResultRegister(ccb_, tmb_->slot(), FIBER_COMMANDS[stat_index]));
      if(errorcount)
      {
        if(first_error)
        {
          out() << "Unable to generate error on single fiber!" << endl;
          CheckStatusConnector(FIBER_COMMANDS, LENGTH_FIBER_COMMANDS, "Fiber");
          errcode = 1;
          return errcode;
        }
        else
        {
          first_error = true;
        }
      }
    }
    if(first_error)
    {
      out() << "Successful generation of error on single fiber!" << endl;
      error = true;
    }
  }
  if(!error)
  {
    out() << "Unable to generate errors on any fiber!" << endl;
    CheckStatusConnector(FIBER_COMMANDS, LENGTH_FIBER_COMMANDS, "Fiber");
    errcode = 1;
    return errcode;
  }
  else
  {
    error = false; //Reset error status
  }

  // Attempt to generate an error on a all fibers
  Niter = 10;
  for(int i=0; i<Niter && !error; ++i)
  {
    bool has_all_errors = true;
    for(int j=0; j<100; ++j)
      ccb_->WriteRegister(CCB_CSRB2_COMMAND_BUS, CCB_COM_FORCE_FIBER_ERROR);
    for(int stat_index=1; stat_index < LENGTH_FIBER_COMMANDS; ++stat_index)
    {
      int errorcount = ResultRegisterData(LoadAndReadResultRegister(ccb_, tmb_->slot(), FIBER_COMMANDS[stat_index]));
      has_all_errors &= (errorcount > 0);
    }
    if(has_all_errors)
    {
      out() << "Successful generation of errors on all fibers!" << endl;
      error = true;
    }
  }
  if(!error)
  {
    out() << "Unable to generate errors on all fibers!" << endl;
    CheckStatusConnector(FIBER_COMMANDS, LENGTH_FIBER_COMMANDS, "Fiber");
    errcode = 1;
    return errcode;
  }
  else
  {
    error = false; //Reset error status
  }

  // Attempt to clear error counters


  return errcode;
}

int FirmwareTester::CheckFirmwareTestStatus()
{
  bool ok = true;

  cout << endl;

  // Check if loopback test is running
  out() << "Loopback Firmware Test Status: ";
  if(GetTestStatus("Loopback") > 0)
    out() << "ON" << endl;
  else
    out() << "OFF" << endl;
  // Check for errors in tests
  ok &= !(CheckStatusLoopback(DMBLOOP_COMMANDS, LENGTH_DMBLOOP_COMMANDS, "DMB"));
  ok &= !(CheckStatusLoopback(RPCLOOP_COMMANDS, LENGTH_RPCLOOP_COMMANDS, "RPC"));
  ok &= !(CheckStatusLoopback(RPCLOOP_SLOW_COMMANDS, LENGTH_RPCLOOP_SLOW_COMMANDS, "RPC_Slow"));
  out() << "RPC Loop Hz Test Status: ";

  // Check if Hz test is running
  uint32_t read_0, read_1;
  read_0 = ResultRegisterData(LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_RPCLOOP_HZ_TEST_COUNT));
  usleep(1000000);
  read_1 = ResultRegisterData(LoadAndReadResultRegister(ccb_, tmb_->slot(), CCB_COM_RR_LOAD_RPCLOOP_HZ_TEST_COUNT));
  if(read_0 != read_1)
    out() << "ON" << endl;
  else
    out() << "OFF" << endl;

  // Check if cable test is running
  out() << "Cable Firmware Test Status: ";
  if(GetTestStatus("Cable") > 0)
    out() << "ON" << endl;
  else
    out() << "OFF" << endl;
  // Check for errors in cable test
  ok &= !(CheckStatusConnector(CABLE_COMMANDS, LENGTH_CABLE_COMMANDS, "Cable"));

  // Check if fiber test is running
  out() << "Fiber Firmware Test Status: ";
  if(GetTestStatus("Fiber") > 0)
    out() << "ON" << endl;
  else
    out() << "OFF" << endl;
  // Check for errors in fiber test
  ok &= !(CheckStatusConnector(FIBER_COMMANDS, LENGTH_FIBER_COMMANDS, "Fiber"));

  int errcode = (ok == false);
  //MessageOK(cout, test + " result", errcode);
  return errcode;
}


}} // namespaces
