#ifndef _Emu_PC_CCBCommands_h_
#define _Emu_PC_CCBCommands_h_

#include <stdint.h>


namespace emu { namespace pc {

// Forward declarations
class CCB;


// control & status registers
const int CCB_CSRA1 = 0x00;
const int CCB_CSRA2_STATUS = 0x02;
const int CCB_CSRA3_STATUS = 0x04;

const int CCB_CSRB1 = 0x20;
const int CCB_CSRB2_COMMAND_BUS = 0x22;
const int CCB_CSRB3_DATA_BUS = 0x24;
const int CCB_CSRB6 = 0x2A;
const int CCB_CSRB11 = 0x34;


// VME interface command codes (p16 of CCB doc)
const int CCB_VME_L1RESET = 0x50;
const int CCB_VME_BC0 = 0x52;
const int CCB_VME_L1ACC = 0x54;
const int CCB_VME_HARD_RESET = 0x60;
const int CCB_VME_TMB_HARD_RESET = 0x62;
const int CCB_VME_ALCT_HARD_RESET = 0x66;
const int CCB_VME_SOFT_RESET = 0x6A; // TMB, DMB, MPC
const int CCB_VME_TMB_SOFT_RESET = 0x6C; // TMB
const int CCB_VME_ALCT_ADB_PULSE_SYNC = 0x82;
const int CCB_VME_ALCT_ADB_PULSE_ASYNC = 0x84;
const int CCB_VME_CLCT_EXT_TRIGGER = 0x86;
const int CCB_VME_ALCT_EXT_TRIGGER = 0x88;
const int CCB_VME_DMB_CFEB_CALIB0 = 0x8A;
const int CCB_VME_DMB_CFEB_CALIB1 = 0x8C;
const int CCB_VME_DMB_CFEB_CALIB2 = 0x8E;

// Dummy command code for TMB_reserved0
// Note: it CANNOT be written to VME, use it only for internal bookkeeping
const int CCB_VME_TMB_RESERVED0 = 0xBB;


// custom TMB testing command codes to write into the command bus
const int CCB_COM_RR0 = 0x00;
const int CCB_COM_RR_LOAD_CCB_RX0 = 0xC0;
const int CCB_COM_RR_READ = 0xCC;
const int CCB_COM_RR_LOAD_COUNTER = 0xCD;
const int CCB_COM_RR_LOAD_DATA_BUS = 0xCE;
const int CCB_COM_RR_LOAD_COUNTERS_FLAG = 0xCF;


// VME commands that correspond to pulse counter flags that they trigger.
// Array index corresponds to a bit number.
const int PULSE_IN_COMMANDS[] =
    {
     CCB_VME_BC0, // 0x52;
     CCB_VME_L1ACC, // 0x54;
     CCB_VME_TMB_SOFT_RESET, // 0x6C; // TMB
     CCB_VME_CLCT_EXT_TRIGGER, // 0x86;
     CCB_VME_ALCT_EXT_TRIGGER, // 0x88;
     CCB_VME_DMB_CFEB_CALIB0, // 0x8A;
     CCB_VME_DMB_CFEB_CALIB1, // 0x8C;
     CCB_VME_DMB_CFEB_CALIB2,  // 0x8E;
     CCB_VME_ALCT_HARD_RESET, // 0x66;
     CCB_VME_ALCT_ADB_PULSE_SYNC, // 0x82;
     CCB_VME_ALCT_ADB_PULSE_ASYNC, // 0x84;
     CCB_VME_TMB_RESERVED0
    };
const int LENGTH_PULSE_IN_COMMANDS = sizeof(PULSE_IN_COMMANDS)/sizeof(PULSE_IN_COMMANDS[0]);


//// Special bits that are set by TMB in RR after issuing the CCB_COM_RR0 command on command bus.
//// Note that the bit shifts include the command field width (8 bits)
union RR0Bits
{
  uint32_t r; // value of the TMB result register after the CCB_COM_RR0 command

  struct {
    unsigned int command : 8; // 1st 8 bits
    unsigned int CCB_reserved0 : 1; //#8
    unsigned int CCB_reserved1 : 1;
    unsigned int CCB_reserved2 : 1;
    unsigned int CCB_reserved3 : 1;
    unsigned int TMB_reserved0 : 1; // #12
    unsigned int unused : 3;
    unsigned int DMB_reserved_in0 : 1; // #16
    unsigned int DMB_reserved_in1 : 1; //
    unsigned int DMB_reserved_in2 : 1; //
    unsigned int DMB_L1A_release : 1; // #19
  };
};


/// Accessors to the bit fields of CSRB6
union CSRB6Bits
{
  unsigned int r; // value of the CSRB6 register

  struct {
    unsigned int CCB_reserved2 : 1; // #0
    unsigned int CCB_reserved3 : 1;
    unsigned int TMB_reserved0 : 1; // #2
    unsigned int MPC_reserved0 : 1; // #3
    unsigned int MPC_reserved1 : 1; //
    unsigned int DMB_reserved0 : 1; // #5
    unsigned int DMB_reserved1 : 1; //
    unsigned int TMB_reserved_out0 : 1; // #7
    unsigned int TMB_reserved_out1 : 1; //
    unsigned int TMB_reserved_out2 : 1; //
    unsigned int DMB_reserved_out0 : 1; // #10
    unsigned int DMB_reserved_out1 : 1; //
    unsigned int DMB_reserved_out2 : 1; //
    unsigned int DMB_reserved_out3 : 1; //
    unsigned int DMB_reserved_out4 : 1; //
  };
};


/// Accessors to the bit fields of CSRB11
union CSRB11Bits
{
  unsigned int r; // value of the CSRB11 register

  struct {
    unsigned int DMB_reserved_in0 : 1; // #0
    unsigned int DMB_reserved_in1 : 1; //
    unsigned int DMB_reserved_in2 : 1; //
    unsigned int TMB_reserved_in0 : 1; // #3
    unsigned int TMB_reserved_in1 : 1; //
    unsigned int TMB_reserved_in2 : 1; //
    unsigned int TMB_reserved_in3 : 1; //
    unsigned int TMB_reserved_in4 : 1; //
  };
};


/// check if command triggers a single 25 ns pulse
bool is25nsPulseCommand(const int command);

/// check if command triggers a single 500 ns pulse
bool is500nsPulseCommand(const int command);

/// check if command triggers a finite pulse
bool isFinitePulseCommand(const int command);




/// Set backplane to be controlled by FPGA
void SetFPGAMode(CCB* ccb);

/// perform WriteRegister n times
void NTimesWriteRegister(CCB* ccb, int n, int reg, int value);

/// Write five DMB_reserved_out bits
void Write5ReservedBits(CCB* ccb, int &value);

/// read five TMB_reserved_in bits
int Read5ReservedBits(CCB* ccb);

/// Write TMB_reserved0 bit
void WriteTMBReserved0Bit(CCB* ccb, int value);


/// load TMB result register using load_command, and read in the value
uint32_t LoadAndReadResutRegister(CCB* ccb, int tmb_slot, int load_command);

/// extract command code from TMB result register value
uint32_t ResutRegisterCommand(uint32_t rr);

/// extract the data from TMB result register value
uint32_t ResultRegisterData(uint32_t rr);

}} // namespaces


#endif
