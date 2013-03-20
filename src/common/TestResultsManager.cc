/*
 * $Id: $
 */

// class header
#include "emu/pc/TestResultsManager.h"

namespace emu { namespace pc {

using std::endl;
using std::cout;
using std::string;
using std::map;
using std::pair;
using std::multimap;
using std::vector;

void TestResultsManager::processFile(string f_name)
{
  if(TestResultsManagerCallTrace) cout << __func__ << " " << f_name << endl;
  boost::regex regEx0("(.*)(CCBBackplaneTests_Board)(\\w+)(_)(\\d+)(\\.log)");
  boost::regex regEx1("(.*)(CCBBackplaneTests_Board)(\\w+)(_)(\\d{4}-\\d{2}-\\d{2}_\\d{2}-\\d{2}-\\d{2})(\\.log)");
  boost::smatch matches0;
  boost::smatch matches1;
  if(boost::regex_match(f_name, matches0, regEx0))
  {
    boardLabel_ = matches0[3];
    boardLabels_.insert(boardLabel_);
    time_ = timeToString(atoi(matches0[5].str().c_str()));
    file_.open(f_name.c_str());
  }
  else if(boost::regex_match(f_name, matches1, regEx1))
  {
    boardLabel_ = matches1[3];
    boardLabels_.insert(boardLabel_);
    time_ = matches1[5];
    file_.open(f_name.c_str());
  }
  else
    return;

  while(!file_.eof())
  {
    processLine();
  }

  file_.close();
  file_.clear();
}

void TestResultsManager::processDirectory(const std::string dir_name)
{
  if(TestResultsManagerCallTrace) cout << __func__ << " " << dir_name << endl;
  namespace fs = boost::filesystem;
  fs::path someDir(dir_name);
  fs::directory_iterator end_iter;

  if(fs::exists(someDir) && fs::is_directory(someDir))
  {
    currentPath_ = someDir.string();
    for(fs::directory_iterator dir_iter(someDir) ; dir_iter != end_iter ; ++dir_iter)
    {
      fs::path file(*dir_iter);
      if(!fs::is_directory(file))
        if(loadedFiles_.find(file.leaf()) == loadedFiles_.end())
        {
          processFile(file.string());
          loadedFiles_.insert(file.leaf());
        }
    }
    sortTable();
    std::cout << resultsTable_;
  }

}

void TestResultsManager::initializeKeys()
{
  if(TestResultsManagerCallTrace) cout << __func__ << endl;
  Key key;

  string label = "LogBegin";
  key.regEx = "(TMB-CCB Backplane Tests ME\\+)(\\d*)(/)(\\d*)(/)(\\d*)( output:)";
  //key.identifiers = "jdjdjdj";
  key.identifiers = "0101010";
  keyLabels_.insert(label);
  keys_.insert(pair<string, Key>(label, key));

  label = "TestBegin";
  key.regEx = "(Test with label\\s*)(\\w*)(\\s*...\\s*start)";
  //key.identifiers = "jdj";
  key.identifiers = "010";
  keyLabels_.insert(label);
  keys_.insert(pair<string, Key>(label, key));

  label = "TestResult";
  key.regEx = "(Test with label\\s*)(\\w*)(\\s*status\\s*...\\s*->\\s*)(PASS|FAIL)(!*)";
  //key.identifiers = "jdjd";
  key.identifiers = "0101";
  keyLabels_.insert(label);
  keys_.insert(pair<string, Key>(label, key));

  label = "CompareValues";
  key.regEx = "(CompareValues:\\s*)(.*)(\\s*->\\s*)(PASS|FAIL)(!*\\s*:\\s*)(\\w*)(\\s*)(==|!=)(\\s*)(\\w*)";
  //key.identifiers = "jdjdjdjdjd";
  key.identifiers = "0101010101";
  keyLabels_.insert(label);
  keys_.insert(pair<string, Key>(label, key));
}

void TestResultsManager::insertTestResult(std::string & boardLabel, std::string & testLabel, std::string & time, std::string testResult)
{
  if(TestResultsManagerCallTrace) cout << __func__ << endl;
  std::string r[4];
  r[0] = boardLabel;
  r[1] = testLabel;
  r[2] = time;
  r[3] = testResult;

  resultsTable_.addRow(&r[0], &r[4]);
}


void TestResultsManager::processLine()
{
  //if(TestResultsManagerCallTrace) cout << __func__ << endl;
  vector<string> data_list;

  char line[LINE_BUFFER_LENGTH];
  file_.getline(line, LINE_BUFFER_LENGTH);
  string key_label;

  std::_Rb_tree_const_iterator<std::string> it = keyLabels_.begin();
  for(unsigned int i=0; i<keyLabels_.size(); ++i)
  {
    key_label = *it;
    Key * key = & (keys_[key_label]);
    boost::regex regexPattern((*key).regEx);
    boost::smatch matches;
    string s(line); //must be constructed this way. cannot use string(line) in place of s below, will create strange errors in sub matches

    if(boost::regex_match(s, matches, regexPattern))
    {
      for(unsigned int j=1; j<matches.size(); ++j)
      {
        if((*key).identifiers[j-1] == DATA_ID)
        {
          data_list.push_back(utils::shaveOffBlanks(matches[j].str()));
        }
      }
      break;
    }
    else if(i == keyLabels_.size()-1)
    {
      key_label = "None";
    }

    ++it;
  }

  if(key_label == "LogBegin")
  {
    //boardLabel_ = "2012";
    //time_ = time(NULL);
  }
  else if(key_label == "TestBegin")
  {
    testLabel_ = data_list[0];
    testLabels_.insert(testLabel_);
  }
  else if(key_label == "CompareValues")
  {
  }
  else if(key_label == "TestResult")
  {
    insertTestResult(boardLabel_, testLabel_, time_, data_list[1]);
  }
}

void TestResultsManager::sortTable()
{
  if(TestResultsManagerCallTrace) cout << __func__ << endl;
  resultsTable_.sortByColumn("TestTime", true);
  resultsTable_.sortByColumn("TestLabel");
  resultsTable_.sortByColumn("BoardLabel");
}

std::set<std::string> TestResultsManager::getBoardLabels()
{
  return boardLabels_;
}
std::set<std::string> TestResultsManager::getTestLabels()
{
  return testLabels_;
}


BasicTable<std::string>::row_iterator TestResultsManager::getBegin(std::string column_label, std::string query, row_iterator begin, row_iterator end )
{
  row_iterator it = begin;
  while(it!=end)
  {
    if(it[column_label] == query)
      break;
    else ++it;
  }

  return it;
}
BasicTable<std::string>::row_iterator TestResultsManager::getEnd(std::string column_label, std::string query, row_iterator begin, row_iterator end)
{
  row_iterator it = end;
  while(begin!=it)
  {
    --it;
    if(it[column_label] == query)
      return ++it;
  }

  return end;
}
BasicTable<std::string>::row_iterator TestResultsManager::begin()
{
  return resultsTable_.begin();
}
BasicTable<std::string>::row_iterator TestResultsManager::end()
{
  return resultsTable_.end();
}

void TestResultsManager::setBoardLabel(std::string board_label) {boardLabel_ = board_label;}
std::string TestResultsManager::getBoardLabel() {return boardLabel_;}
void TestResultsManager::setTestLabel(std::string test_label) {testLabel_ = test_label;}
std::string TestResultsManager::getTestLabel() {return testLabel_;}
void TestResultsManager::setTime(time_t time) {time_ = timeToString(time);}
void TestResultsManager::setTime(std::string time) {time_ = time;}
std::string TestResultsManager::getTime() {return time_;}

}} // namespaces
