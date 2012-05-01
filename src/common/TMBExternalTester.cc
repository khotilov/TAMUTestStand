/*
 * $Id: $
 */

// class header
#include "emu/pc/TMBExternalTester.h"

// Emu includes
#include "emu/pc/TMB.h"
#include "emu/pc/CCB.h"
#include "emu/pc/CCBCommands.h"
#include "emu/pc/TestUtils.h"

// system includes
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <unistd.h> // for sleep()
#include <boost/bind.hpp>


namespace emu { namespace pc {

using std::endl;
using std::cout;
using std::string;
using std::set;
using std::hex;
using std::dec;


TMBExternalTester::TMBExternalTester()
{
  RegisterTestProcedures();
}


TMBExternalTester::~TMBExternalTester(){}


TMBExternalTester::TMBExternalTester(const TMBExternalTester &other)
{
  CopyFrom(other);
}


TMBExternalTester & TMBExternalTester::operator=(const TMBExternalTester &rhs)
{
  CopyFrom(rhs);
  return *this;
}


void TMBExternalTester::CopyFrom(const TMBExternalTester &other)
{
  // the real need for the user defined copy c-tor and assignment comes from here:
  // we need to make sure that the test procedures are properly bound to this object.
  RegisterTestProcedures();
}


void TMBExternalTester::RegisterTestProcedures()
{
  cout<<__PRETTY_FUNCTION__<<endl;
  RegisterTheTest("Blah", boost::bind( &TMBExternalTester::TestBlah, this));
}


////////////////////////////////////////////////////
// Actual tests:
////////////////////////////////////////////////////

int TMBExternalTester::TestBlah()
{
  string test = TestLabelFromProcedureName(__func__);
  bool ok = true;

  int errcode = (ok == false);
  MessageOK(cout, test + " result", errcode);
  return errcode;
}



}} // namespaces
