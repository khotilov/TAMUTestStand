/*
 * $Id: $
 */

// class header
#include "emu/pc/TestUtils.h"


// system includes
#include <iomanip>
#include <cmath>

namespace emu { namespace pc {

using std::endl;
using std::string;
using std::hex;
using std::dec;



bool CompareValues(std::ostream * out,
                   const std::string &test,
                   int testval,
                   int compareval,
                   bool equal,
                   bool print_pass)
{
  string pass_relation = " == ";
  string fail_relation = " != ";
  if (!equal)
  {
    pass_relation = " != ";
    fail_relation = " == ";
  }

  if ( (  equal && testval == compareval) ||
       ( !equal && testval != compareval) )
  {
    if(print_pass)
    {
      (*out) << "CompareValues:  " << test << " -> PASS! " << test << " : " << hex << testval << pass_relation << compareval << dec << endl;
    }
    return true;
  }
  else
  {
    (*out) << "CompareValues:  " << test << " -> FAIL! " << test << " : " << hex << testval << fail_relation << compareval << dec << endl;
    return false;
  }
}


bool CompareValues(std::ostream * out,
                   const std::string &test,
                   float testval,
                   float compareval,
                   float tolerance,
		   bool print_pass)
{
  float err = (testval - compareval) / compareval;
  float fractolerance = tolerance * compareval;

  if (std::abs(err) > tolerance)
  {
    (*out) << "CompareValues tolerance:  " << test << " -> FAIL! " << test << " : expected = " << compareval << ", returned = " << testval << " outside of tolerance " << fractolerance << endl;
    return false;
  }
  else
  {
    if(print_pass)
    {
      (*out) << "CompareValues tolerance:  " << test << " -> PASS! " << test << " : value = " << testval << " within "<< fractolerance << " of " << compareval << endl;
    }
    return true;
  }
}


void MessageOK(std::ostream * out,
               const std::string &test,
               bool testbool)
{
  (*out) << test;
  if (testbool) (*out) << " -> PASS" << endl;
  else          (*out) << " -> FAIL <-" << endl;
  return;
}


}} // namespaces
