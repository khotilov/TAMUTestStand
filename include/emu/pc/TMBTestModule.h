#ifndef _TMBTestModule_h_
#define _TMBTestModule_h_

#include "toolbox/lang/Class.h"
#include "emu/pc/TMBTester.h"

#include <vector>
#include <sstream>


namespace xdaq { class WebApplication; }
namespace xgi { class Input; class Output; }


namespace emu { namespace pc {

class ConfigurablePCrates;

/** \class TMBTestModule
 *
 * A module with pages & utilities for TMB testing.
 * Acts as a user interface wrapper for the TMBTester class which does the real work.
 *
 * The XGI bound methods have to be bound using bindMemberMethod to the Application that uses this module.
 *
 * The baseline code was initially adopted from EmuPeripheralCrateConfig.
 *
 * \author Vadim Khotilovich
 *
 */
class TMBTestModule : public virtual toolbox::lang::Class
{
public:

  TMBTestModule(xdaq::WebApplication * app, ConfigurablePCrates * sys);

  /// the html interface page (XGI bound method)
  void TMBTestsPage(xgi::Input * in, xgi::Output * out );

  /// the interface page calls this to run the tests (XGI bound method)
  void TMBRunTest(xgi::Input * in, xgi::Output * out );

  /// store results into a log file (called from the TMBTestsPage) (XGI bound method)
  void TMBLogTestsOutput(xgi::Input * in, xgi::Output * out );


  /// Initialize TMBTesters for all TMBs in the "current" crate. The "current" crate has to be already set externally.
  void Init();

  /// access the output of tests for a particular tmb & crate
  std::ostringstream & getTestOutput(int tmb, int crate) { return testOutputs_[tmb][crate]; }

private:

  /// shortcut for the code to create a run-test button
  void TestButton(int tmb, int testId, const std::string &label, int testResult, xgi::Output * out);

  /// keep link to the application which uses this utility
  xdaq::WebApplication * app_;

  /// the system under testing
  ConfigurablePCrates * sys_;

  /// tests runner classes for tmbs in a particular crate initialized with InitTMBTests
  std::vector<TMBTester> tests_;

  /// keeps tests' output for [tmb][crate]
  std::ostringstream testOutputs_[10][30];

  int tmbN_;
};


}} // namespaces


#endif
