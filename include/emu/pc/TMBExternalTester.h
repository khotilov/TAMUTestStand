#ifndef emu_pc_TMBExternalTester_h
#define emu_pc_TMBExternalTester_h

#include "emu/pc/TestWorkerBase.h"


namespace emu { namespace pc {


/** class TMBExternalTester
 *
 * Implements actual test procedures for the TMB testing using CCB and backplane.
 *
 * \author Vadim Khotilovich
 */
class TMBExternalTester : public TestWorkerBase
{
public:

  // not responsible for deleting pointers
  TMBExternalTester();

  /// copy c-tor (allows, e.g., proper storing objects of this class in std::vectors, etc.)
  TMBExternalTester(const TMBExternalTester &);

  /// custom assignment operator is also needed
  TMBExternalTester & operator=(const TMBExternalTester &);

  virtual ~TMBExternalTester();

private:

  /// helper copying method for internal use by the copy c-tor and the assignment operator
  void CopyFrom(const TMBExternalTester &other);

  /// register all the test procedures
  void RegisterTestProcedures();


  // Actual test procedures below...
  // They are implemented as private, and should be accessed only through the interface offered by TestWorkerBase.

  /**
   * Blah
   */
  int TestBlah();
  
};


}} // namespaces

#endif
