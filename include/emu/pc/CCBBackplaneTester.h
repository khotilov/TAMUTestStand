#ifndef emu_pc_CCBBackplaneTester_h
#define emu_pc_CCBBackplaneTester_h

#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <boost/function.hpp>

namespace emu { namespace pc {

class TMB;
class CCB;


/** class CCBBackplaneTester
 *
 * Runs actual test units for TMB-CCB testing using backplane.
 *
 * \author Vadim Khotilovich
 */
class CCBBackplaneTester
{
public:

  // not responsible for deleting pointers
  CCBBackplaneTester();

  /// copy c-tor (allows, e.g., storing objects of this class properly in std::vectors etc.)
  CCBBackplaneTester(const CCBBackplaneTester &);

  virtual ~CCBBackplaneTester();

  /// custom assignment operator is also needed
  CCBBackplaneTester & operator=(const CCBBackplaneTester &);

  /// Allows to set the results output destination.
  /// Initially, the internal out_ output is set to std::cout in the constructor.
  void RedirectOutput(std::ostream * output) { out_ = output ; }

  void SetTMB(TMB * tmb) {tmb_ = tmb;}
  void SetCCB(CCB * ccb) {ccb_ = ccb;}

  /// returns labels of available registered tests
  std::vector<std::string> GetTestLabels();

  /** Getter for the result of a test with specific label.
   * The results are first initialized to -1 in the constructor.
   * Running the tests through RunTest would be setting
   * result values to either 0 (not pass) or 1 (pass).
   */
  int GetTestResult(const std::string &test);

  /// setter for the result of a test with specific label
  void SetTestResult(const std::string &test, int result);

  /// run test with specific label
  void RunTest(const std::string &test);

  /// issue HardReset
  void Reset();

private:

  /// helper copying method for internal use by the copy c-tor and the assignment operator
  void CopyFrom(const CCBBackplaneTester &other);

  /// test procedure should have this type: no parameters, returns bool
  typedef boost::function<bool ()> TestProcedure;

  /// register all the test procedures
  void RegisterTestProcedures();

  /// register a new test procedure with a given label
  void RegisterTheTest(const std::string &test, TestProcedure proc);

  // actual test procedures

  /// run all tests
  bool TestAll();

  /// dummy test
  bool TestDummy() {return true;}

  /**
   * Send various pulse triggering commands through VMS interface and
   * check that for each command only this command's fit blag is set
   * in the pulse counter flags register.
   */
  bool TestPulseCountersBits();

  /**
   * Write various values into CSRB2 (command bus),
   * read back through TMB's result register, and compare
   */
  bool TestCommandBus();


  // holds CCB and TMB pointers
  CCB * ccb_;
  TMB * tmb_;

  /// where to store the results output
  std::ostream * out_ ;

  /// test label -> test procedure association
  std::map<std::string, TestProcedure> testProcedures_ ;

  /// test label -> test result association
  std::map<std::string, int> testResults_ ;
};


}} // namespaces

#endif
