/*
 * $Id: $
 */

// class header
#include "emu/pc/ConfigurablePCrates.h"

// Emu includes
#include "emu/db/PCConfigHierarchy.h"
#include "emu/db/XMLReadWriter.h"
#include "emu/db/ConfigTree.h"
#include "emu/pc/EmuEndcapConfigWrapper.h"
#include "emu/pc/EmuEndcap.h"
#include "emu/pc/Crate.h"
#include "emu/pc/Chamber.h"
#include "emu/pc/DAQMB.h"
#include "emu/pc/TMB.h"
#include "emu/pc/CCB.h"
#include "emu/pc/MPC.h"
#include "emu/pc/ALCTController.h"
#include "emu/pc/RAT.h"
#include "emu/pc/VMECC.h"
#include "emu/pc/DDU.h"
#include "emu/utils/String.h"


// xdaq includes
#include "xdaq/WebApplication.h"

// system includes
#include <iostream>


namespace emu {  namespace pc {

using std::endl;
using std::cout;
using std::string;


ConfigurablePCrates::ConfigurablePCrates(xdaq::WebApplication * app)
: app_(app)
{
  clear();
}


void ConfigurablePCrates::clear()
{
  endcap_.reset();

  crates_.clear();
  thisCrate_ = 0;
  thisVME_ = 0;
  thisCCB_ = 0;
  thisMPC_ = 0;
  thisRAT_ = 0;
  thisALCT_ = 0;

  chambers_.clear();
  tmbs_.clear();
  dmbs_.clear();
  ddus_.clear();

  thisChamber_ = 0;
  thisTMB_ = 0;

  thisCrateN_ = -1;
  thisTMBN_ = -1;
}


ConfigurablePCrates::~ConfigurablePCrates() {}


std::vector<Crate*> & ConfigurablePCrates::crates()
{
  return crates_;
}

Crate * ConfigurablePCrates::crate()
{
  return thisCrate_;
}

std::vector<Chamber*> & ConfigurablePCrates::chambers()
{
  return chambers_;
}

VMECC * ConfigurablePCrates::vme()
{
  return thisVME_;
}


CCB * ConfigurablePCrates::ccb()
{
  return thisCCB_;
}

ALCTController * ConfigurablePCrates::alct()
{
  return thisALCT_;
}

RAT * ConfigurablePCrates::rat()
{
  return thisRAT_;
}

MPC * ConfigurablePCrates::mpc()
{
  return thisMPC_;
}

std::vector<TMB*> & ConfigurablePCrates::tmbs()
{
  return tmbs_;
}

std::vector<DAQMB*> & ConfigurablePCrates::dmbs()
{
  return dmbs_;
}

std::vector<DDU*> & ConfigurablePCrates::ddus()
{
  return ddus_;
}

int ConfigurablePCrates::crateN()
{
  return thisCrateN_;
}

Chamber* ConfigurablePCrates::chamber()
{
  return thisChamber_;
}

TMB* ConfigurablePCrates::tmb()
{
  return thisTMB_;
}

int ConfigurablePCrates::tmbN()
{
  return thisTMBN_;
}


bool ConfigurablePCrates::Configure(const std::string & xml_filename, const std::string & tstore_filename, bool force_configure)
{
  if (endcap_ != 0)
  {
    if(force_configure) // reload configuration
    {
      clear();
    }
    else // do nothing
    {
      return true;
    }
  }

  LOG4CPLUS_INFO(app_->getApplicationLogger(), "ConfigurablePCrates loading XML config...");

  emu::db::PCConfigHierarchy h(tstore_filename);
  emu::db::XMLReadWriter xml(&h, xml_filename);
  xml.read(2000000);
  emu::db::ConfigTree config_tree(xml.configTables());

  endcap_.reset( emu::pc::EmuEndcapConfigWrapper(&config_tree).getConfiguredEndcap() );

  if (!endcap_) return false;
  endcap_->NotInDCS();

  crates_ = endcap_->crates();
  if (crates_.size() <= 0) return false;

  // current crate is the 1st one
  SetCurrentCrate(0);

  // current TMB is the 1st one in the 1st crate
  SetCurrentTMB(0);

  LOG4CPLUS_INFO(app_->getApplicationLogger(), "ConfigurablePCrates Done");

  return true;
}


bool ConfigurablePCrates::SetCurrentCrate(int cr)
{
  if (crates_.size() <= 0)
  {
    cout << "ConfigurablePCrates::SetCurrentCrate: no crates configured" << endl;
    return false;
  }

  if (crates_.size() < (size_t)cr+1)
  {
    cout << "ConfigurablePCrates::SetCurrentCrate: "
        "crate index "<<cr<<" is larger then number of configured crates "<< crates_.size() << endl;
    return false;
  }

  thisCrate_ = crates_[cr];
  thisCrateN_ = cr;

  chambers_ = thisCrate_->chambers();
  thisCCB_ = thisCrate_->ccb();
  thisMPC_ = thisCrate_->mpc();
  tmbs_ = thisCrate_->tmbs();
  dmbs_ = thisCrate_->daqmbs();
  ddus_ = thisCrate_->ddus();

  // current TMB reset to the 1st one in the crate
  SetCurrentTMB(0);

  return true;
}


bool ConfigurablePCrates::SetCurrentTMB(int t)
{
  if (!thisCrate_)
  {
    cout << "ConfigurablePCrates::SetCurrentTMB: curent crate is not set" << endl;
    return false;
  }

  thisTMB_ = tmbs_[t];

  thisChamber_ = thisCrate_->GetChamber(thisTMB_->GetTmbSlot());

  thisTMBN_ = t;

  cout << "Current TMB was set:  slot " << thisTMB_->GetTmbSlot() << "  index " << t << "  in crate #" << thisCrateN_ << endl;

  return true;
}


void ConfigurablePCrates::useTMBInSlot(int slot)
{
  if (slot <= 0 && thisTMB_ != NULL) return; // keep the previous thisTMB_

  for (size_t k = 0; k < tmbs_.size(); ++k)
  {
    if (tmbs_[k]->GetTmbSlot() == slot)
    {
      thisTMB_ = tmbs_[k];

      thisChamber_ = thisCrate_->GetChamber(slot);

      thisTMBN_ = k;

      cout << "Current TMB was set:  slot "<< slot << "  index "<< k << "  in crate #" << thisCrateN_ << endl;

      return;
    }
  }

  // don't have this specified slot
  XCEPT_RAISE(xcept::Exception,
     string("Misconfiguration: could not assign TMB with slot ") +
       emu::utils::stringFrom(slot) );
}



}}  // namespaces
