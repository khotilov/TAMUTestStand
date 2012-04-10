#ifndef _Emu_PC_CCBBackplaneUtilsModule_h_
#define _Emu_PC_CCBBackplaneUtilsModule_h_


#include "toolbox/lang/Class.h"


namespace xdaq { class WebApplication; }
namespace xgi { class Input; class Output; }


namespace emu { namespace pc {

class ConfigurablePCrates;

/** \class CCBBackplaneUtilsModule
 *
 * A module with HTML interface pages & utilities for TAMU TMB testing
 * that is done using signals that go through the CCB and backpanel.
 *
 * \author Vadim Khotilovich
 *
 */
class CCBBackplaneUtilsModule : public virtual toolbox::lang::Class
{
public:

  CCBBackplaneUtilsModule(xdaq::WebApplication * app, ConfigurablePCrates * sys);

  /// the html interface page
  void CCBLowLevelUtilsPage(xgi::Input * in, xgi::Output * out );

  void ReadCCBRegister(xgi::Input * in, xgi::Output * out );
  void WriteCCBRegister(xgi::Input * in, xgi::Output * out );
  void ReadTTCRegister(xgi::Input * in, xgi::Output * out );
  void HardReset(xgi::Input * in, xgi::Output * out );
  void CCBConfig(xgi::Input * in, xgi::Output * out );
  void CCBSignals(xgi::Input * in, xgi::Output * out );

  void RunBackplaneCommand(xgi::Input * in, xgi::Output * out );

private:

  void BackplaneTestsBlock(xgi::Input * in, xgi::Output * out );

  /// the application which uses this module
  xdaq::WebApplication * app_;

  /// the system under testing
  ConfigurablePCrates * sys_;

  int CCBRegisterRead_, CCBRegisterValue_, CCBRegisterWrite_, CCBWriteValue_;
  int CCBCmdWrite_, CCBDataWrite_, CCBBitsValue_;

  int TMBSlot_;
  // use a large capacity datatype for reading in serialized TMB status:
  unsigned long long int TMBStatus_;
  
  int TMBReservedBit_;

  int Reserved5BitsWrite_, Reserved5Bits_;
};


}} // namespaces


#endif
