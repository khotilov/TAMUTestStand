#ifndef TESTRESULTSMANAGERMODULE_H_
#define TESTRESULTSMANAGERMODULE_H_

#include "toolbox/lang/Class.h"
#include "emu/pc/TestResultsManager.h"

#include <time.h>

namespace xdaq { class WebApplication; }
namespace xgi { class Input; class Output; }

namespace emu { namespace pc {

class ConfigurablePCrates;

class TestResultsManagerModule : public virtual toolbox::lang::Class
{
public :

  /// constructor
  /// the pointers to application and ConfigurablePCrates data are managed externally
  TestResultsManagerModule(xdaq::WebApplication * app);

  /// html interface page for test results (XGI bound method)
  void TestResultsManagerPage(xgi::Input * in, xgi::Output * out);

  /// process logs in a specific directory (XGI bound method)
  void ProcessLogDirectory(xgi::Input * in, xgi::Output * out);

  /// sets the state of the page (XGI bound method)
  void SetPageMode(xgi::Input * in, xgi::Output * out);

private:

  /// keep link to the application which uses this utility (not owned by this class!)
  xdaq::WebApplication * app_;

  /// manager of test results
  TestResultsManager trm_;

  // current mode of the page (allows switching views on a single page)
  int pageMode_;

  // summary of boards
  void BoardListTable(xgi::Input * in, xgi::Output * out );

  // test summary of individual board
  void BoardTestSummaryTable(xgi::Input * in, xgi::Output * out);

  // summary of test instances for a single test type
  void TestDetailsTable(xgi::Input * in, xgi::Output * out);

};



} } //Namespaces


#endif /* TESTRESULTSMANAGERMODULE_H_ */
