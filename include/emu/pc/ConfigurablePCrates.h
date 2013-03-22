#ifndef _ConfigurablePCrates_h_
#define _ConfigurablePCrates_h_


#include "toolbox/lang/Class.h"

#include <vector>
#include <boost/shared_ptr.hpp>


namespace xdaq { class WebApplication; }


namespace emu { namespace pc {

class EmuEndcap;
class Crate;
class Chamber;
class VMECC;
class ALCTController;
class RAT;
class CCB;
class MPC;
class DAQMB;
class TMB;
class DDU;

/** \class ConfigurablePCrates
 *
 * Encapsulation of EmuEndcap consisting of peripheral crates and everything within them.
 *
 * At this moment, the only configuration option implemented here is from XML file.
 * TODO: add ability to configure from DB.
 *
 * \author Vadim Khotilovich
 *
 */
class ConfigurablePCrates : public virtual toolbox::lang::Class
{
public:

  ConfigurablePCrates(xdaq::WebApplication * app);

  ~ConfigurablePCrates();

private:

  /// forbid copying
  ConfigurablePCrates(const ConfigurablePCrates &);

  /// forbid assignment
  ConfigurablePCrates & operator=(const ConfigurablePCrates &);

public:

  /**
   * Loads configuration from xml file.
   *
   * Sets the current crate to the 1st crate, the current TMB to the 1st TMB in the 1st crate.
   *
   * \param xml_filename   xml file location
   * \param tstore_filename   tstore configuration file location (needed for reading the system schema)
   * \param force_configure   reload configuration if true, do nothing is false and configuration was already loaded
   */
  bool Configure(const std::string & xml_filename, const std::string & tstore_filename, bool force_configure = false);

  /// set the current crate index and also set current TMB to the lowest slot# with TMB
  bool SetCurrentCrate(int c);

  /// set the current TMB (or chamber) index in the current crate 
  /// Index starts from 0, and 0 means "set current the lowest slot# with TMB"
  bool SetCurrentTMB(int t);


  /**
   * This method allows to assign a TMB in a specific slot for this crate.
   * By default, in the constructor, the first available TMB with the lowest
   * slot number is used. Also, if slot<=0, the assignment doesn't change.
   * Throws when tmb was not successfully assigned.
   */
  void useTMBInSlot(int slot);


  /// vector of all crates
  std::vector<Crate*> & crates();

  /// current crate
  Crate * crate();

  /// vector of chambers in current crate
  std::vector<Chamber*> & chambers();

  /// VME in current crate
  VMECC * vme();

  /// CCB in current crate
  CCB * ccb();

  /// ALCTController in current crate
  ALCTController * alct();

  /// RAT in current crate
  RAT * rat();

  /// MPC in current crate
  MPC * mpc();

  /// TMBs in current crate
  std::vector<TMB*> & tmbs();

  /// DMBs in current crate
  std::vector<DAQMB*> & dmbs();

  /// DDUs in current crate
  std::vector<DDU*> & ddus();

  /// index of the current crate
  int crateN();


  /// current chamber in current crate
  Chamber* chamber();

  /// current TMB in current crate
  TMB* tmb();

  /// index of current TMB in current crate
  int tmbN();

private:

  void clear();

  xdaq::WebApplication * app_;

  boost::shared_ptr<EmuEndcap> endcap_;

  std::vector<Crate*> crates_;
  Crate *thisCrate_;
  VMECC *thisVME_;
  CCB *thisCCB_;
  ALCTController *thisALCT_ ;
  RAT *thisRAT_;
  MPC *thisMPC_;
  std::vector<Chamber*> chambers_;
  std::vector<TMB*> tmbs_;
  std::vector<DAQMB*> dmbs_;
  std::vector<DDU*> ddus_;

  Chamber * thisChamber_;
  TMB * thisTMB_;

  int thisCrateN_;
  int thisTMBN_;
};

}} // namespaces

#endif
