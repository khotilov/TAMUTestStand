#ifndef emu_pc_CCBBackplaneTester_h
#define emu_pc_CCBBackplaneTester_h

#include "emu/pc/TestWorkerBase.h"


namespace emu { namespace pc {

class TMB;
class CCB;


/** class CCBBackplaneTester
 *
 * Implements actual test procedures for the TMB testing using CCB and backplane.
 *
 * \author Vadim Khotilovich
 */
class CCBBackplaneTester : public TestWorkerBase
{
public:

  // not responsible for deleting pointers
  CCBBackplaneTester();

  /// copy c-tor (allows, e.g., proper storing objects of this class in std::vectors, etc.)
  CCBBackplaneTester(const CCBBackplaneTester &);

  /// custom assignment operator is also needed
  CCBBackplaneTester & operator=(const CCBBackplaneTester &);

  virtual ~CCBBackplaneTester();

  void SetTMB(TMB * tmb) {tmb_ = tmb;}
  void SetCCB(CCB * ccb) {ccb_ = ccb;}

  /// issue HardReset
  void Reset();

private:

  /// helper copying method for internal use by the copy c-tor and the assignment operator
  void CopyFrom(const CCBBackplaneTester &other);

  /// register all the test procedures
  void RegisterTestProcedures();

  /// implementation of the base class method
  void PrepareHWForTest();

  // actual test procedures

  /// dummy test
  bool TestDummy() {return true;}

  /**
   * Send various pulse triggering commands through VMS interface and
   * check that for each command only this command's fit blag is set
   * in the pulse counter flags register.
   * Also check the counter values against expected  numbers.
   */
  bool TestPulseCounters();

  /**
   * Write various values into CSRB2 (command bus),
   * read back through TMB's result register, and compare
   */
  bool TestCommandBus();


  // holds CCB and TMB pointers
  CCB * ccb_;
  TMB * tmb_;
};


}} // namespaces

#endif
