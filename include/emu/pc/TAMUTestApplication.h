#ifndef _Emu_PC_TAMUTestApplication_h_
#define _Emu_PC_TAMUTestApplication_h_

#include "emu/pc/EmuPeripheralCrateBase.h"

#include "emu/pc/ConfigurablePCrates.h"
#include "emu/pc/TMBTestModule.h"
#include "emu/pc/CCBBackplaneUtilsModule.h"
#include "emu/pc/CCBBackplaneTestModule.h"
#include "emu/pc/TestResultsManagerModule.h"

#include "xdaq/WebApplication.h"


namespace emu { namespace pc {


/** \class TAMUTestApplication
 * main web GUI Application class for the TAMU TMB test stand
 */
class TAMUTestApplication: public xdaq::WebApplication
{
public:

  XDAQ_INSTANTIATOR();

  /// constructor
  TAMUTestApplication(xdaq::ApplicationStub * s);

  /// home-page of the application
  void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  /// webpage to select/upload different config file
  void ChooseConfigXMLPage(xgi::Input * in, xgi::Output * out );

  /// webpage that displays contents of the current crate
  void CrateContentsPage(xgi::Input * in, xgi::Output * out );

private:

  /// load/reload configuration from XML file
  bool ParseXML();

  /// allows to select different crate as current from a drop-down list
  void CrateSelection(xgi::Input * in, xgi::Output * out );

  /// set different config file from a location on server
  void SetConfFile(xgi::Input * in, xgi::Output * out );

  /// upload different config file
  void UploadConfFile(xgi::Input * in, xgi::Output * out );

  /// Output logging into a file
  void LogOutput(xgi::Input * in, xgi::Output * out );


  /// the EMU system
  ConfigurablePCrates teststand_;

  /// the TMB testing module
  TMBTestModule tmbTestModule_;

  /// the TMB testing module through CCB & backplane
  CCBBackplaneTestModule ccbBackplaneTestModule_;

  /// the collection of CCB & backplane utility commands
  CCBBackplaneUtilsModule ccbBackplaneUtilsModule_;

  /// the test results manager module
  TestResultsManagerModule testResultsManagerModule_;

  // some more members to keep local parameters and state
  xdata::String xmlFile_;
  xdata::String tstoreFile_;
  xdata::Integer tmbSlot_;

  int parsed_;
};

}} // namespaces

#endif
