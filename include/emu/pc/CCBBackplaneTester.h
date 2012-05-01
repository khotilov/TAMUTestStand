#ifndef emu_pc_CCBBackplaneTester_h
#define emu_pc_CCBBackplaneTester_h

#include "emu/pc/TestWorkerBase.h"


namespace emu { namespace pc {


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

private:

  /// helper copying method for internal use by the copy c-tor and the assignment operator
  void CopyFrom(const CCBBackplaneTester &other);

  /// register all the test procedures
  void RegisterTestProcedures();


  // Actual test procedures below...
  // They are implemented as private, and should be accessed only through the interface offered by TestWorkerBase.

  /// dummy test
  int TestDummy() {return 0;}

  /**
   * Write to DataBus and check that it is cleared after L1Reset signal is sent.
   * Also check that the total counter and the counter flag bits are cleared by L1Reset.
   */
  int TestL1Reset();

  /**
   * Write to DataBus and check that it is cleared after HardReset signal is sent.
   */
  int TestTMBHardReset();

  /**
   * Send various pulse triggering commands through VMS interface and check that
   * for each command only this command's fit blag is set in the pulse counter flags register.
   * Also check the counter values against expected  numbers.
   */
  int TestPulseCounters();

  /**
   * Write various values into CSRB2 (command bus), read back through TMB's result register, and compare
   */
  int TestCommandBus();

  /**
   * Test CCB_reserved[0:3] connections
   * CCB_reserved0 - trivial test (no real way to control it right now)
   * CCB_reserved1 - trivial test (no real way to control it right now)
   * CCB_reserved2 - write through CSRB6, read through RR0
   * CCB_reserved3 - write through CSRB6, read through RR0
   */
  int TestCCBReserved();

  /**
   * Test TMB_reserved_out[0:2] connections (CSRB6[9:7]) by reading them back from RR bits [15:13]
   */
  int TestTMBReservedOut();

  /**
   * Test DMB_reserved_out[0:4] connections (CSRB6[14:10]) by reading them back from CSRB11 bits [7:3]
   */
  int TestDMBReservedOut();

  /**
   * Write various values into CSRB3 (CCB data bus),
   * read back through TMB's result register, and compare.
   *
   * Also, the TMB command that reads the data bus value (bits [7:0] out of 12 data bits),
   * sets one of 4 more bits (bits [11:8]) if status of some clocks is not good.
   * Test checks if these clock statuses are zero.
   */
  int TestDataBus();
  
  /**
   * Read the value of ccb_clock40_enable (or ccb_rx0) multiple times.
   * Check that we see sufficient variety in the values.
   */
  int TestCCBClock40();

  /**
  * Test DMB_reserved_in [2:0] via loopback board,
  * bits [1:0] are tested by writing into bits [4:3] of DMB_reserved_out,
  * bit 2 is tested by writing into CCB_reserved2
  */
  int TestDMBReservedInLoopback();
};


}} // namespaces

#endif
