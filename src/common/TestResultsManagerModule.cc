/*
 * TestResultsManagerModule.cc
 *
 *  Created on: Aug 1, 2012
 *      Author: Austin Schneider
 */

#include "emu/pc/TestResultsManagerModule.h"

// Emu includes
#include "emu/utils/Cgi.h"

// XDAQ includes
#include "cgicc/Cgicc.h"
#include "cgicc/HTMLClasses.h"
#include "xgi/Utils.h"
#include "xdaq/WebApplication.h"

// system includes
#include <string>
#include <vector>
#include <set>

namespace emu { namespace pc {

using std::endl;
using std::cout;
using std::string;
using std::map;
using std::vector;
using std::set;

TestResultsManagerModule::TestResultsManagerModule(xdaq::WebApplication *app)
: app_(app)
, trm_()
, pageMode_(0)
{
}

void TestResultsManagerModule::TestResultsManagerPage(xgi::Input * in, xgi::Output * out )
{
  using cgicc::tr;
  using cgicc::form;
  using cgicc::input;

  cgicc::Cgicc cgi(in);

  string urn = app_->getApplicationDescriptor()->getURN();

  emu::utils::headerXdaq(out, app_,"Test Results Manager");

  *out << "<table cellpadding=\"2\" cellspacing=\"2\" border=\"0\" style=\"width: 100%; font-family: arial;\">" << endl;
  *out << "<tbody>" << endl;
  *out << "<tr>" << endl;
  *out << "<td width=\"50%\">" << endl;

  *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;") << endl;
  *out << cgicc::legend("Results Manager Controls:").set("style", "color:blue") <<endl;

  ///////////////////////////////////////////
  // Results Manager Controls:

  *out << form().set("method","GET").set("action", "/" + urn + "/SetPageMode" ) << endl;
  *out << input().set("type","submit").set("value", "Test Results Home").set("style", "color:blue") << endl;
  *out << input().set("type", "hidden").set("value", "0").set("name", "mode") << endl;
  *out << form() << endl;
  *out << cgicc::br() << endl;
  *out << form().set("method","GET").set("action", "/" + urn + "/ProcessLogDirectory" ) << endl;
  *out << input().set("type", "text").set("name", "path").set("size", "100").set("value", trm_.getCurrentPath()) << endl;
  *out << input().set("type","submit").set("value", "Process Log Directory").set("style", "color:blue") << endl;
  *out << form() << endl;

  //*out << cgicc::table().set("border","1");
  *out << cgicc::fieldset() << endl;


  *out << cgicc::fieldset().set("style","font-size: 11pt; font-family: arial;") << endl;
  ///////////////////////////////////////////
  // Results Table:

  if(pageMode_ == 0)
    BoardListTable(in, out);
  else if(pageMode_ == 1)
    BoardTestSummaryTable(in, out);
  else if(pageMode_ == 2)
    TestDetailsTable(in, out);

  *out << cgicc::fieldset() << endl;

  *out << "</td>" << endl;

  *out << "</tr>" << endl;
  *out << "</tbody>" << endl;
  *out << "</table>" << endl;


  emu::utils::footer(out);
}

void TestResultsManagerModule::BoardListTable(xgi::Input * in, xgi::Output * out )
{
  using cgicc::tr;
  using cgicc::td;
  using cgicc::table;
  using cgicc::form;
  using cgicc::input;

  typedef TestResultsManager::row_iterator row_iterator;

  string urn = app_->getApplicationDescriptor()->getURN();

  // section label
  *out << cgicc::legend("Boards:").set("style", "color:blue") <<endl;

  // results table
  *out << table().set("border", "1") << endl;
  *out << tr() << endl;

  // column headers
  *out << td() << "Board" << td() << endl;
  *out << td() << "Last Test Time" << td() << endl;
  *out << td() << "Status" << td() << endl;


  *out << tr() << endl;

  set<string> board_labels = trm_.getBoardLabels();
  set<string> test_labels = trm_.getTestLabels();

  // iterate over board labels (each board label corresponds to a single board)
  for(set<string>::iterator i = board_labels.begin(); i != board_labels.end(); ++i)
  {
    string time = "";
    int fail = false;
    row_iterator board_begin = trm_.getBegin("BoardLabel", *i, trm_.begin(), trm_.end());
    row_iterator board_end = trm_.getEnd("BoardLabel", *i, trm_.begin(), trm_.end());

    // iterate over test labels (each test label corresponds to a test type)
    // to find the last time the board was tested
    for(set<string>::iterator j = test_labels.begin(); j != test_labels.end(); ++j)
    {
      // gets the iterator of the most recent testingInstance
      row_iterator k = trm_.getBegin("TestLabel", *j, board_begin, board_end);
      if(k == trm_.getEnd("TestLabel", *j, board_begin, board_end))
        fail |= -1;
      else
      {
        if(k["TestTime"] > time)
          time = k["TestTime"];
        if(k["TestResult"]!="PASS")
          fail |= true;
      }
    }
    *out << tr() << endl;

    // button linking to test summary of the board
    *out << td() << endl;
    *out << form().set("method","GET").set("action", "/" + urn + "/SetPageMode" ) << endl;
    *out << input().set("type","submit").set("value", *i).set("style", "color:blue") << endl;
    *out << input().set("type", "hidden").set("value", "1").set("name", "mode") << endl;
    *out << input().set("type", "hidden").set("value", *i).set("name", "board_label") << endl;
    *out << form() << endl;
    *out << td() << endl;
    *out << td() << time << td() << endl;

    // board status (based on last instance of each test type)
    *out << td();
    if(!fail)
      *out << "GOOD";
    else if(fail == 1)
      *out << "BAD";
    else if(fail < 0)
      *out << "UNTESTED";
    *out << td() << endl;

    *out << tr() << endl;
  }

  *out << table() << endl;
}

void TestResultsManagerModule::BoardTestSummaryTable(xgi::Input * in, xgi::Output * out)
{
  using cgicc::tr;
  using cgicc::td;
  using cgicc::table;
  using cgicc::form;
  using cgicc::input;

  typedef TestResultsManager::row_iterator row_iterator;

  row_iterator board_begin = trm_.getBegin("BoardLabel", trm_.getBoardLabel(), trm_.begin(), trm_.end());
  row_iterator board_end = trm_.getEnd("BoardLabel", trm_.getBoardLabel(), trm_.begin(), trm_.end());

  string urn = app_->getApplicationDescriptor()->getURN();
  // section label
  *out << cgicc::legend("Board " + trm_.getBoardLabel() + ": Test Summary:").set("style", "color:blue") <<endl;

  // results table
  *out << table().set("border", "1") << endl;
  *out << tr() << endl;

  // column headers
  *out << td() << "Test" << td() << endl;
  *out << td() << "Last Test Time" << td() << endl;
  *out << td() << "Last Test Result" << td() << endl;


  *out << tr() << endl;

  set<string> test_labels = trm_.getTestLabels();
  // iterate over test labels (types)
  for(set<string>::iterator j = test_labels.begin(); j != test_labels.end(); ++j)
  {
    row_iterator test_begin = trm_.getBegin("TestLabel", *j, board_begin, board_end);
    row_iterator test_end = trm_.getEnd("TestLabel", *j, board_begin, board_end);

    if(test_begin!=test_end)
    {
      *out << tr() << endl;

      // Test Label (link to test type page)
      *out << td() << endl;
      *out << form().set("method","GET").set("action", "/" + urn + "/SetPageMode" ) << endl;
      *out << input().set("type","submit").set("value", *j).set("style", "color:blue") << endl;
      *out << input().set("type", "hidden").set("value", "2").set("name", "mode") << endl;
      *out << input().set("type", "hidden").set("value", *j).set("name", "test_label") << endl;
      *out << form() << endl;
      *out << td() << endl;

      // Test Time
      *out << td() << test_begin["TestTime"] << td() << endl;

      // Test Result
        *out << td() << test_begin["TestResult"] << td() << endl;

      *out << tr() << endl;
    }
  }

  *out << table() << endl;
}

void TestResultsManagerModule::TestDetailsTable(xgi::Input * in, xgi::Output * out)
{
  using cgicc::tr;
  using cgicc::td;
  using cgicc::table;
  using cgicc::form;
  using cgicc::input;

  typedef TestResultsManager::row_iterator row_iterator;

  string urn = app_->getApplicationDescriptor()->getURN();

  *out << cgicc::legend("Board " + trm_.getBoardLabel() + ": " + trm_.getTestLabel() + ":").set("style", "color:blue") <<endl;

  *out << table().set("border", "1") << endl;
  *out << tr() << endl;

  *out << td() << "Test Time" << td() << endl;
  *out << td() << "Test Result" << td() << endl;


  *out << tr() << endl;

  row_iterator board_begin = trm_.getBegin("BoardLabel", trm_.getBoardLabel(), trm_.begin(), trm_.end());
  row_iterator board_end = trm_.getEnd("BoardLabel", trm_.getBoardLabel(), trm_.begin(), trm_.end());

  row_iterator test_begin = trm_.getBegin("TestLabel", trm_.getTestLabel(), board_begin, board_end);
  row_iterator test_end = trm_.getEnd("TestLabel", trm_.getTestLabel(), board_begin, board_end);

  for(row_iterator k = test_begin; k != test_end; ++k)
  {
    *out << tr() << endl;

    // Test Time
    *out << td() << k["TestTime"] << td() << endl;

    // Test Result
    *out << td() << k["TestResult"] << td() << endl;

    *out << tr() << endl;
  }

  *out << table() << endl;
}

void TestResultsManagerModule::ProcessLogDirectory(xgi::Input * in, xgi::Output * out)
{
  cgicc::Cgicc cgi(in);
  cgicc::form_iterator name = cgi.getElement("path");

  boost::filesystem::path p;

  if(name != cgi.getElements().end())
  {
    p = cgi["path"]->getValue();
    cout << __func__ << ":  path " << p.string() << endl;
    trm_.processDirectory(p.string());
  }

  this->TestResultsManagerPage(in,out);
}

void TestResultsManagerModule::SetPageMode(xgi::Input * in, xgi::Output * out)
{
  cgicc::Cgicc cgi(in);
  cgicc::form_iterator name = cgi.getElement("tmb");

  string s = "";
  int mode = 0;

  name = cgi.getElement("mode");
  if(name != cgi.getElements().end())
  {
    s = cgi["mode"]->getValue();
    mode = atoi(s.c_str());
  }
  pageMode_ = mode;

  name = cgi.getElement("board_label");
  if(name != cgi.getElements().end())
  {
    s = cgi["board_label"]->getValue();
    trm_.setBoardLabel(s);
  }

  name = cgi.getElement("test_label");
  if(name != cgi.getElements().end())
  {
    s = cgi["test_label"]->getValue();
    trm_.setTestLabel(s);
  }

  name = cgi.getElement("time");
  if(name != cgi.getElements().end())
  {
    s = cgi["time"]->getValue();
    trm_.setTime(atoi(s.c_str()));
  }



  this->TestResultsManagerPage(in,out);
}

} } //Namespaces
