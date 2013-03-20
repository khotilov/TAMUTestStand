/*
 * $Id: $
 */

// class header
#include "emu/pc/TestUtils.h"


// system includes
#include <iomanip>
#include <cmath>
#include <sstream>

namespace emu { namespace pc {

using std::endl;
using std::string;
using std::hex;
using std::dec;



bool CompareValues(std::ostream &out,
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
      out << "CompareValues:  " << test << " -> PASS! : " << hex << testval << pass_relation << compareval << dec << endl;
    }
    return true;
  }
  else
  {
    out << "CompareValues:  " << test << " -> FAIL! : " << hex << testval << fail_relation << compareval << dec << endl;
    return false;
  }
}


bool CompareValues(std::ostream &out,
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
    out << "CompareValues tolerance:  " << test << " -> FAIL! " << test << " : expected = " << compareval << ", returned = " << testval << " outside of tolerance " << fractolerance << endl;
    return false;
  }
  else
  {
    if(print_pass)
    {
      out << "CompareValues tolerance:  " << test << " -> PASS! " << test << " : expected = " << compareval << ", returned = " << testval << " inside of tolerance " << fractolerance << endl;
    }
    return true;
  }
}


void MessageOK(std::ostream &out,
               const std::string &test,
               int errcode)
{
  out << test;
  if (errcode == 0)     out << " -> PASS" << endl;
  else if (errcode < 0) out << " -> INACTIVE" << endl;
  else                  out << " -> FAIL" << endl;
  return;
}

std::string timeToString(time_t t)
{
  using namespace std;

  struct tm tm;
  localtime_r(&t, &tm);

  std::string gap0 = "-";
  std::string gap1 = "_";
  std::string gap2 = gap0;

  std::stringstream ss;
  ss << setfill('0') << setw(4) << tm.tm_year+1900 << gap0
     << setfill('0') << setw(2) << tm.tm_mon+1     << gap0
     << setfill('0') << setw(2) << tm.tm_mday      << gap1
     << setfill('0') << setw(2) << tm.tm_hour      << gap2
     << setfill('0') << setw(2) << tm.tm_min       << gap2
     << setfill('0') << setw(2) << tm.tm_sec;

  return ss.str();
}

int atoi_hex (const std::string & text)
{
  unsigned int x;
  std::stringstream ss;
  ss << std::hex << text;
  ss >> x;
  return x;
};


}} // namespaces
