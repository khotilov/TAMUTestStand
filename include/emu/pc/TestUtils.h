/** file TestUtils.h
 *
 * Helper functions for tester classes
 *
 * $Id:$
 */

#ifndef emu_pc_TestUtils_h
#define emu_pc_TestUtils_h

#include <iostream>
#include <string>

namespace emu { namespace pc {

/// Compare integer values
/// test if "testval" is equivalent to the expected value: "compareval"
/// return depends on if you wanted them to be "equal"
bool CompareValues(std::ostream & out, const std::string &test, float testval, float compareval, float tolerance, bool print_pass = false);

/// Compare float values with some tolerance
bool CompareValues(std::ostream & out, const std::string &test, int testval, int compareval, bool equal, bool print_pass = false);

/// Formatted test result printout
void MessageOK(std::ostream & out, const std::string &test, int errcode);

/// Convert unix time to formatted string
std::string timeToString(time_t t);

/// Conversion from text to integer (assuming text displays in hexadecimal)
int atoi_hex (const std::string & text);


}} // namespaces

#endif
