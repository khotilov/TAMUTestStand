#ifndef _Emu_PC_CCBCommands_h_
#define _Emu_PC_CCBCommands_h_

#include <stdint.h>
#include "emu/pc/ResultRegisterSerializer.h"


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

// TMB command codes related to loopback testing
  // TMB command codes related to DMB loopback
const int CCB_COM_RR_LOAD_DMBLOOP_ERROR_COUNT = 0xD0;
const int CCB_COM_RR_LOAD_DMBLOOP1_STAT = 0xD1;
const int CCB_COM_RR_LOAD_DMBLOOP2_STAT = 0xD2;
const int CCB_COM_RR_LOAD_DMBLOOP3_STAT = 0xD3;
//const int CCB_COM_RR_LOAD_DMBLOOP4_STAT = 0xD4;
//const int CCB_COM_RR_LOAD_DMBLOOP5_STAT = 0xD5;
  // TMB command codes related to RPC loopback
const int CCB_COM_RR_LOAD_RPCLOOP_ERROR_COUNT = 0xD8;
const int CCB_COM_RR_LOAD_RPCLOOP1_STAT = 0xD9;
const int CCB_COM_RR_LOAD_RPCLOOP2_STAT = 0xDA;
const int CCB_COM_RR_LOAD_RPCLOOP3_STAT = 0xDB;
const int CCB_COM_RR_LOAD_RPCLOOP4_STAT = 0xDC;
const int CCB_COM_RR_LOAD_RPCLOOP5_STAT = 0xDD;
const int CCB_COM_RR_LOAD_RPCLOOP6_STAT = 0xDE;
const int CCB_COM_RR_LOAD_RPCLOOP7_STAT = 0xDF;
  // TMB command codes related to skewclear cable test
const int CCB_COM_RR_LOAD_CABLE_STAT = 0xE0;
const int CCB_COM_RR_LOAD_CABLE1_ERROR_COUNT = 0xE1;
const int CCB_COM_RR_LOAD_CABLE2_ERROR_COUNT = 0xE2;
const int CCB_COM_RR_LOAD_CABLE3_ERROR_COUNT = 0xE3;
const int CCB_COM_RR_LOAD_CABLE4_ERROR_COUNT = 0xE4;
const int CCB_COM_RR_LOAD_CABLE5_ERROR_COUNT = 0xE5;
  // TMB command codes related to fiber test
const int CCB_COM_RR_LOAD_FIBER_STAT = 0xF0;
const int CCB_COM_RR_LOAD_FIBER1_ERROR_COUNT = 0xF1;
const int CCB_COM_RR_LOAD_FIBER2_ERROR_COUNT = 0xF2;
const int CCB_COM_RR_LOAD_FIBER3_ERROR_COUNT = 0xF3;
const int CCB_COM_RR_LOAD_FIBER4_ERROR_COUNT = 0xF4;
const int CCB_COM_RR_LOAD_FIBER5_ERROR_COUNT = 0xF5;
const int CCB_COM_RR_LOAD_FIBER6_ERROR_COUNT = 0xF6;
const int CCB_COM_RR_LOAD_FIBER7_ERROR_COUNT = 0xF7;
  // TMB command codes related to test counts
const int CCB_COM_RR_LOAD_CABLE_TEST_COUNT = 0xFC;
const int CCB_COM_RR_LOAD_LOOP_TEST_COUNT = 0xFD;
//const int CCB_COM_RR_LOAD_UNUSED_TEST_COUNT = 0xFE;
const int CCB_COM_RR_LOAD_FIBER_TEST_COUNT = 0xFF;
  // TMB commands for starting and stopping tests
const int CCB_COM_START_TRIG_ALL = 0x18;
const int CCB_COM_START_TRIG_LOOP = 0x19;
const int CCB_COM_START_TRIG_CABLE = 0x1A;
const int CCB_COM_START_TRIG_FIBER = 0x1B;
const int CCB_COM_STOP_TRIG_ALL = 0x1C;
const int CCB_COM_STOP_TRIG_LOOP = 0x1D;
const int CCB_COM_STOP_TRIG_CABLE = 0x1E;
const int CCB_COM_STOP_TRIG_FIBER = 0x1F;

const int TEST_COUNT_COMMANDS[] =
    {
     CCB_COM_RR_LOAD_LOOP_TEST_COUNT,
     CCB_COM_RR_LOAD_CABLE_TEST_COUNT,
     CCB_COM_RR_LOAD_FIBER_TEST_COUNT
    };
const int LENGTH_TEST_COUNT_COMMANDS = sizeof(TEST_COUNT_COMMANDS)/sizeof(TEST_COUNT_COMMANDS[0]);

// TMB command array for DMBLOOP testing
const int DMBLOOP_COMMANDS[] =
    {
     CCB_COM_RR_LOAD_DMBLOOP_ERROR_COUNT,
     CCB_COM_RR_LOAD_DMBLOOP1_STAT,
     CCB_COM_RR_LOAD_DMBLOOP2_STAT,
     CCB_COM_RR_LOAD_DMBLOOP3_STAT
     //CCB_COM_RR_LOAD_DMBLOOP4_STAT
     //CCB_COM_RR_LOAD_DMBLOOP5_STAT
    };
const int LENGTH_DMBLOOP_COMMANDS = sizeof(DMBLOOP_COMMANDS)/sizeof(DMBLOOP_COMMANDS[0]);

// TMB command array for RPCLOOP testing
const int RPCLOOP_COMMANDS[] =
    {
     CCB_COM_RR_LOAD_RPCLOOP_ERROR_COUNT,
     CCB_COM_RR_LOAD_RPCLOOP1_STAT,
     CCB_COM_RR_LOAD_RPCLOOP2_STAT,
     CCB_COM_RR_LOAD_RPCLOOP3_STAT,
     CCB_COM_RR_LOAD_RPCLOOP4_STAT,
     CCB_COM_RR_LOAD_RPCLOOP5_STAT,
     CCB_COM_RR_LOAD_RPCLOOP6_STAT,
     CCB_COM_RR_LOAD_RPCLOOP7_STAT
    };
const int LENGTH_RPCLOOP_COMMANDS = sizeof(RPCLOOP_COMMANDS)/sizeof(RPCLOOP_COMMANDS[0]);

// TMB command array for CABLE testing
const int CABLE_COMMANDS[] =
    {
     CCB_COM_RR_LOAD_CABLE_STAT,
     CCB_COM_RR_LOAD_CABLE1_ERROR_COUNT,
     CCB_COM_RR_LOAD_CABLE2_ERROR_COUNT,
     CCB_COM_RR_LOAD_CABLE3_ERROR_COUNT,
     CCB_COM_RR_LOAD_CABLE4_ERROR_COUNT,
     CCB_COM_RR_LOAD_CABLE5_ERROR_COUNT
    };
const int LENGTH_CABLE_COMMANDS = sizeof(CABLE_COMMANDS)/sizeof(CABLE_COMMANDS[0]);

// TMB command array for FIBER testing
const int FIBER_COMMANDS[] =
    {
     CCB_COM_RR_LOAD_FIBER_STAT,
     CCB_COM_RR_LOAD_FIBER1_ERROR_COUNT,
     CCB_COM_RR_LOAD_FIBER2_ERROR_COUNT,
     CCB_COM_RR_LOAD_FIBER3_ERROR_COUNT,
     CCB_COM_RR_LOAD_FIBER4_ERROR_COUNT,
     CCB_COM_RR_LOAD_FIBER5_ERROR_COUNT,
     CCB_COM_RR_LOAD_FIBER6_ERROR_COUNT,
     CCB_COM_RR_LOAD_FIBER7_ERROR_COUNT
    };
const int LENGTH_FIBER_COMMANDS = sizeof(FIBER_COMMANDS)/sizeof(FIBER_COMMANDS[0]);

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
//// Note that the bit shifts include the command field width (8 bits = TMB_RR_COMMAND_WIDTH)
union RR0Bits
{
  uint32_t r; // value of the TMB result register after the CCB_COM_RR0 command

  struct {
    unsigned int command : TMB_RR_COMMAND_WIDTH; // 1st 8 bits
    unsigned int CCB_reserved0 : 1; //#8
    unsigned int CCB_reserved1 : 1;
    unsigned int CCB_reserved2 : 1;
    unsigned int CCB_reserved3 : 1;
    unsigned int TMB_reserved0 : 1; // #12
    unsigned int TMB_reserved_out : 3; // #13
    unsigned int DMB_reserved_in : 3; // #16
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
    unsigned int TMB_reserved_out : 3; // #9:7
    unsigned int DMB_reserved_out : 5; // #14:10
  };
};


/// Accessors to the bit fields of CSRB11
union CSRB11Bits
{
  unsigned int r; // value of the CSRB11 register

  struct {
    unsigned int DMB_reserved_in : 3; // #2:0
    unsigned int TMB_reserved_in : 5; // #7:3
  };
};

// Accessors to parts of DMB and RPC ERROR_COUNT command returns
union LoopErrorCount
{
  unsigned int r;

  struct{
    unsigned int command : TMB_RR_COMMAND_WIDTH; // #7:0
    unsigned int error_count : 11; // #18:8
    unsigned int overflow : 1; // 19
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
uint32_t ResultRegisterCommand(uint32_t rr);

/// extract the data from TMB result register value
uint32_t ResultRegisterData(uint32_t rr);

}} // namespaces


#endif
