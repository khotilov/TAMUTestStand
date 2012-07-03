#ifndef _Emu_PC_CCBBackplaneTestModule_h_
#define _Emu_PC_CCBBackplaneTestModule_h_


#include "toolbox/lang/Class.h"
#include "emu/pc/CCBBackplaneTester.h"
#include "emu/pc/TMBTestManager.h"

#include <vector>
#include <sstream>

namespace xdaq { class WebApplication; }
namespace xgi { class Input; class Output; }


namespace emu { namespace pc {

class ConfigurablePCrates;

/** \class CCBBackplaneTestModule
 *
 * A module with pages & utilities for TMB developmenmt testing through the CCB and backpanel.
 * Acts as a user interface wrapper for the TMBTester class which does the real work.
 *
 * The XGI bound methods have to be bound using bindMemberMethod to the Application that uses this module.
 *
 * \author Vadim Khotilovich
 *
 */
class CCBBackplaneTestModule : public virtual toolbox::lang::Class
{
public:

  /// constructor
  /// the pointers to application and ConfigurablePCrates data are managed externally
  CCBBackplaneTestModule(xdaq::WebApplication * app, ConfigurablePCrates * sys);

  /// The html interface page (XGI bound method)
  void CCBBackplaneTestsPage(xgi::Input * in, xgi::Output * out );

  /// The html interface page for continuously running (long) tests (XGI bound method)
  void CCBBackplaneContinuousTestsPage(xgi::Input * in, xgi::Output * out );

  /// The interface page calls this to run the tests (XGI bound method)
  void CCBBackplaneRunTest(xgi::Input * in, xgi::Output * out );

  /// Call this to check on the status of continuous FW tests
  void CheckContinuousTestsStatus(xgi::Input * in, xgi::Output * out );

  /// Store results into a log file (called from the TMBTestsPage)  (XGI bound method)
  void CCBBackplaneLogTestsOutput(xgi::Input * in, xgi::Output * out );


  /// Initialize CCBBackplaneTester's for all TMBs in the "current" crate. The "current" crate has to be already set externally.
  void Init();

  /// access the output of tests for a particular tmb
  std::ostringstream & getTestOutput(int tmb);

private:

  /// shortcut for the code to create a run-test button
  void TestButton(int tmb, const std::string &label, const std::string &test_label, xgi::Output * out);

  /// keep link to the application which uses this utility (not owned by this class!)
  xdaq::WebApplication * app_;

  /// the system under testing (not owned by this class!)
  ConfigurablePCrates * sys_;

  /// remember last tmb index
  int tmbN_;

  /// manager of groups of tests
  TMBTestManager tm_;
};


}} // namespaces


#endif
