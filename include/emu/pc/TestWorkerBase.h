#ifndef emu_pc_TestWorkerBase_h
#define emu_pc_TestWorkerBase_h

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <boost/function.hpp>

namespace emu { namespace pc {


/** class TestWorkerBase
 *
 * A base class that contains hardware-independent testing functionality like
 *  - registering a test procedur
 *  - running the test(s)
 *  - keeping log of test output
 * which should be reused by its subclasses.
 *
 * Notes:
 *  - subclasses have to define the abstract methods;
 *  - subclasses have to implement copy c-tor and assignment operator which
 *    should properly take care of re-binding their test procedures to new objects
 *
 * \author Vadim Khotilovich
 */
class TestWorkerBase
{
public:

  // not responsible for deleting pointers
  TestWorkerBase();

  virtual ~TestWorkerBase();

  /// returns labels of available registered tests
  std::vector<std::string> GetTestLabels();

  /// Getter for the result of a test with specific label.
  /// Note: the results values are first initialized to -1 in the constructor.
  /// Running the tests through RunTest would set the corresponding result values to either 0 (not pass) or 1 (pass).
  int GetTestResult(const std::string &test);

  /// setter for the result of a test with specific label
  void SetTestResult(const std::string &test, int result);

  /// Run a test with specific label.
  /// A special case of a label is "All", which would cause running of all registered tests.
  bool RunTest(const std::string &test);

  /// Allows to set the results output destination.
  /// Initially, the internal out_ output is set to std::cout in the constructor.
  virtual void RedirectOutput(std::ostream * output);

  /// reference accessor to the output stream
  std::ostream & out() { return *out_; }

protected:

  /// defines the signature for test procedures
  typedef boost::function<bool ()> TestProcedure;

  /// Register a new test procedure with a given label
  /// @param test is a test's label (note: it cannot be "All" which is reserved for running all tests)
  /// @param proc is a binding for a test procedure
  /// If a procedure with a given label was registered before, it would only reset the functional binding.
  void RegisterTheTest(const std::string &test, TestProcedure proc);

  /// Register the collection of test procedures by calling RegisterTheTest for each specific test.
  /// Has to be implemented by every subclass!
  virtual void RegisterTestProcedures() = 0;

  /// Hardware preparation procedure that has to be performed before every test.
  /// Has to be implemented by every subclass!
  virtual void PrepareHWForTest() = 0;

private:

  /// where to store the output with results
  std::ostream * out_ ;

  /// keep _ordered_ list of test labels
  std::vector<std::string> testLabels_;

  /// test label -> test procedure association
  std::map<std::string, TestProcedure> testProcedures_ ;

  /// test label -> test result association
  std::map<std::string, int> testResults_ ;
};


}} // namespaces

#endif
