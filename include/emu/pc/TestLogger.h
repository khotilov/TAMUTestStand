/*
 * TestLogger.h
 *
 *  Created on: Mar 5, 2013
 *      Author: Austin Schneider
 */

#ifndef TESTLOGGER_H_
#define TESTLOGGER_H_

// Emu includes
#include "emu/pc/BasicTable.h"

// system includes
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

namespace emu { namespace pc {

class TestLogger
{
public:
  TestLogger();
  TestLogger(std::string);
  void openFile(std::string);
  void startTest(std::string);
  void endTest(int);
  void reportError(int);
  void beginLogging();
  void endLogging();
  void setBoard(std::string);




private:
  bool testing;
  bool logging;
  std::string currentBoard;
  std::string currentTest;
  std::fstream log;




};


} } //Namespaces


#endif /* TESTLOGGER_H_ */
