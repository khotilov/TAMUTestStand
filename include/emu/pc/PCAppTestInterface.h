/*
 * \author Vadim Khotilovich
 * $Id:$
 */

#ifndef _Emu_PC_TestAppWrapper_h_
#define _Emu_PC_TestAppWrapper_h_

#include "emu/pc/Crate.h"
#include "emu/pc/Chamber.h"
#include "emu/pc/TMB.h"

#include "xgi/Input.h"
#include "xgi/Output.h"
#include "xdaq/ApplicationDescriptor.h"


namespace emu { namespace pc {

/** \class PCAppTestInterface
 *
 * An interface for implementing partial polymorphism for PCrate Applications that do testing.
 * It covers only limited necessary set of methods.
 *
 */
class PCAppTestInterface
{
public:
  virtual ~PCAppTestInterface() {}

  virtual void MainPage(xgi::Input * in, xgi::Output * out) = 0;
  virtual void MyHeader(xgi::Input * in, xgi::Output * out, const std::string & title ) = 0;

  virtual xdaq::ApplicationDescriptor* getApplicationDescriptor() = 0;

  virtual Chamber * getChamber(int tmb) = 0;
  virtual Crate * getCrate() = 0;
  virtual TMB * getTMB(int tmb) = 0;
  virtual CCB * getCCB() = 0;
  virtual int getCrateN() = 0;
};


}} // namespaces

#endif
