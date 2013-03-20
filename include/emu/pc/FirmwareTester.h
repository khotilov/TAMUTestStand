#ifndef emu_pc_FirmwareTester_h
#define emu_pc_FirmwareTester_h

#include "emu/pc/TestWorkerBase.h"
#include "emu/utils/SimpleTimer.h"

#include <map>


namespace emu { namespace pc {


/** class CCBBackplaneTester
 *
 * Implements actual test procedures for the TMB testing using CCB and backplane.
 *
 * \author Austin Schneider
 */
class FirmwareTester : public TestWorkerBase
{
public:

  // not responsible for deleting pointers
  FirmwareTester();

  /// copy c-tor (allows, e.g., proper storing objects of this class in std::vectors, etc.)
  FirmwareTester(const FirmwareTester &);

  /// custom assignment operator is also needed
  FirmwareTester & operator=(const FirmwareTester &);

  virtual ~FirmwareTester();

private:

  /// helper copying method for internal use by the copy c-tor and the assignment operator
  void CopyFrom(const FirmwareTester &other);

  /// register all the test procedures
  void RegisterTestProcedures();


  // Actual test procedures below...
  // They are implemented as private, and should be accessed only through the interface offered by TestWorkerBase.

  /// dummy test
  int TestDummy() {return 0;}

  /**
   * Check status of loopback test
   */
  int CheckStatusLoopback(int const *, int const, std::string);

  /**
   * Template for loopback tests
   */
  int TemplateTestLoopback(int const *, int const, std::string, std::string);

  /**
   * Check for errors in DMB loopback test
   * report number, type, and location of errors
   */
  int TestDMBLoopback();

  /**
   * Check for errors in RPC loopback test
   * report number, type, and location of errors
   */
  int TestRPCLoopback();

  /**
   * Check status of connector test
   */
  int CheckStatusConnector(int const *, int const, std::string, bool);

  /**
   * Template for connector tests
   */
  int TemplateTestConnector(int const, int const *, int const, std::string, std::string);

  /**
   * Check for errors in Skewclear Cable test
   * report number, type, and location of errors
   */
  int TestCableConnector();

  /**
   * Check for errors in Fiber Link test
   * report number, type, and location of errors
   */
  int TestFiberConnector();

  /**
   * Check for errors in all firmware tests
   */
  int CheckFirmwareTestStatus();


};


}} // namespaces

#endif
