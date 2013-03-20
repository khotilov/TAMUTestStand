/*
 * TestLogger.cc
 *
 *  Created on: Mar 7, 2013
 *      Author: Austin Schneider
 */

// class header
#include "emu/pc/TestLogger.h"

namespace emu { namespace pc {

TestLogger::TestLogger():
    testing(false),
    logging(false),
    currentBoard(""),
    currentTest(""),
    log()
{
}
TestLogger::TestLogger(std::string fname):
    testing(false),
    logging(false),
    currentBoard(""),
    currentTest(""),
    log(fname)
{
}
void TestLogger::openFile(std::string);
void startTest(std::string);
void endTest(int);
void reportError(int);
void beginLogging();
void endLogging();
void setBoard(std::string);




} } //Namespaces
