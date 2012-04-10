#include "config/PackageInfo.h"

namespace EmuTAMUTestStand {
const std::string package  =  "EmuTAMUTestStand";
const std::string versions =  "11.01.00";
const std::string summary = "emu/emuDCS/TAMUTestStand";
const std::string description = "TAMU Emu test stand application library";
const std::string authors = "Vadim Khotilovich";
const std::string link = "http://cms.cern.ch";
config::PackageInfo getPackageInfo();
void checkPackageDependencies();
}

GETPACKAGEINFO(EmuTAMUTestStand);

void EmuTAMUTestStand::checkPackageDependencies() {}
