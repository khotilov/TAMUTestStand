#ifndef _ResultRegisterSerializer_h_
#define _ResultRegisterSerializer_h_


namespace emu { namespace pc {

class CCB;

const int TMB_RESULT_REGISTER_WIDTH = 20;


/** \class ResultRegisterSerializer
 *
 * Allows serialized reading of the result register of TMB that is being tested through CCB.
 * Reading from a specific TMB is performed through just two bits: alct_cfg_done and tmb_cfg_done
 * which are accessible through CSRA2 & CSRA3 registers of CCB.
 *
 * \author Vadim Khotilovich
 *
 */
class ResultRegisterSerializer
{
public:

  /// initialize with a CCB and TMB slot number
  ResultRegisterSerializer(CCB *ccb, int tmb_slot);

  /// \return the result of serial read of TMB status bits
  unsigned long long int read(int length = TMB_RESULT_REGISTER_WIDTH);


private:

  /// holder of CCB
  CCB * ccb_;

  /// TMB slot
  int tmbSlot_;

  /// LUT for the TMB slot # -> sequential index association
  static int slotToIndexLUT_[22];

  // bit shifting amounts
  int shiftCsra2Alct_;
  int shiftCsra2Tmb_;
  int shiftCsra3Tmb_;
};


}} // namespaces


#endif
