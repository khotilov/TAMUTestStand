#ifndef _Emu_PC_TMBTestManager_h_
#define _Emu_PC_TMBTestManager_h_


#include "emu/pc/TestWorkerBase.h"

#include <vector>
#include <map>
#include <sstream>
#include <boost/shared_ptr.hpp>

#include "emu/pc/ConfigurablePCrates.h"
#include "emu/pc/Crate.h"
#include "emu/pc/TMB.h"
#include "emu/pc/CCB.h"


namespace emu { namespace pc {


/** \class TMBTestManager
 *
 * Provides the means to manages groups of TMB tests for TMBs in the current crate.
 * The tests have to be implementations of the TestWorkerBase base tester class.
 * The test groups are registered inside of the Init() method.
 *
 * \author Vadim Khotilovich
 *
 */
class TMBTestManager
{
public:

  /// Constructor
  /// Note: destructor is trivial, and copy c-tor & assignment operator are forbidden
  TMBTestManager();

  /// Initialize tests for all TMBs in the "current" crate ("current" crate has to be already set externally).
  /// The pointer to ConfigurablePCrates data is managed externally
  void Init(ConfigurablePCrates * sys);

  /// \return the labels of test groups
  std::vector<std::string> GetTestGroupLabels() const;

  /// \return the tester object that is responsible for a specific \c test_group label.
  /// If \c tmb number is -1, the "current" tmb in the "current" crate is assumed.
  boost::shared_ptr<TestWorkerBase> GetTester(const std::string &test_group, int tmb = -1);

  /// Access the output of tests for a particular #tmb
  /// If \c tmb number is -1, the "current" tmb in the "current" crate is assumed.
  std::ostringstream & GetTestOutput(std::string test_group, int tmb = -1);

  /// Label of the board being tested
  std::string boardLabel;

private:

  /// forbid copying
  TMBTestManager(const TMBTestManager &);

  /// forbid assignment
  TMBTestManager & operator=(const TMBTestManager &);

  ///
  template <typename T>
  void RegisterTestGroup(const std::string &test_group);

  /// the system under testing (not owned by this class!)
  ConfigurablePCrates * sys_;

  /// Keeps _ordered_ list of test group labels
  std::vector<std::string> testGroupLabels_;

  /// Container of tests runner classes for tmbs in a particular crate
  /// The map key is labels of test groups.
  /// The vector is over all TMBs in the current crate.
  std::map< std::string, std::vector<boost::shared_ptr<TestWorkerBase> > > tests_;

  /// Keeps tests outputs for each tmb in a current crate
  //std::vector<std::ostringstream> testOutputs_;
  std::map<std::string, std::ostringstream * > testOutputs_;
  //std::ostringstream testOutputs_[10];
};


template <typename T>
void TMBTestManager::RegisterTestGroup(const std::string &test_group)
{
  std::cout<<__PRETTY_FUNCTION__<< ": " << test_group <<std::endl;

  if (tests_.find(test_group) == tests_.end())
  {
    testGroupLabels_.push_back(test_group);
    std::vector<boost::shared_ptr<TestWorkerBase> > empty_vec;
    tests_[test_group] = empty_vec;

    testOutputs_.insert(std::pair<std::string, std::ostringstream * >(test_group, new std::ostringstream[10]));
  }
  else
  {
    // shared_ptr should take care of proper destruction
    tests_[test_group].clear();
  }

  std::vector< TMB * > tmbs = sys_->crate()->tmbs();
  CCB * ccb = sys_->crate()->ccb();

  for (size_t i = 0; i < tmbs.size(); i++)
  {
    boost::shared_ptr<TestWorkerBase> tmp(new T);
    tmp->SetTMB(tmbs[i]);
    tmp->SetCCB(ccb);
    tmp->RedirectOutput(&testOutputs_[test_group][i]);
    tests_[test_group].push_back(tmp);

    //if (i >= testOutputs_.size())
    //{
    //  testOutputs_.resize(i+1);
    //}
  }

}

}} // namespaces


#endif
