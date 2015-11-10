/*

    \file                          Definition.h
    \brief                         Definition File, listing the registers
    \author                        Nicolas PIERRE
    \version                       1.0
    \date                          07/06/14
    Support :                      mail to : nico.pierre@icloud.com

 */

//------------------------------------------------------------------------------
//uHal Connection File

// #define UHAL_CONNECTION_FILE "file://settings/connections.xml"

//-----------------------------------------------------------------------------
//Glib Config Files

#define XML_DESCRIPTION_FILE_2CBC "settings/HWDescription_2CBC.xml"
#define XML_DESCRIPTION_FILE_8CBC "settings/HWDescription_8CBC.xml"
#define XML_DESCRIPTION_FILE_16CBC "settings/Beamtest_Nov15.xml"

//------------------------------------------------------------------------------
//Cbc Config Files

// #define DEFAULT_FILE   "settings/default_file_Hole.txt"
// #define FE0CBC0HOLE    "settings/FE0CBC0Hole.txt"
// #define FE0CBC1        "settings/FE0CBC1.txt"
// #define FE0CBC1HOLE    "settings/FE0CBC1Hole.txt"

//---------------REGISTERS------------------------------------------------------
//------------------------------------------------------------------------------
//I2C Parameters

#define I2C_CTRL_ENABLE   0x000009F4
#define I2C_CTRL_DISABLE  0
#define I2C_STROBE        1
#define I2C_M16B          0
#define I2C_MEM           1
#define I2C_WRITE_ADDR    0x09
#define I2C_READ_ADDR     0x06

#define I2C_SLAVE         0x5B
#define I2C_COMMAND       "user_wb_ttc_fmc_regs.cbc_reg_i2c_command"
#define I2C_REPLY         "user_wb_ttc_fmc_regs.cbc_reg_i2c_reply"
#define I2C_SETTINGS      "user_wb_ttc_fmc_regs.cbc_reg_i2c_settings"

#define MAX_NB_LOOP       70

//------------------------------------------------------------------------------
//Board infos

#define BOARD_TYPE          "board_id"
#define FW_VERSION_MAJOR    "firm_id.firmware_major"
#define FW_VERSION_MINOR    "firm_id.firmware_minor"
#define FW_VERSION_BUILD    "firm_id.firmware_build"
#define FMC1_PRESENT        "status.fmc1_present"
#define FMC2_PRESENT        "status.fmc2_present"

// User FW infos


//FMC infos
#define FMC_USER_BOARD_ID "user_wb_ttc_fmc_regs.user_board_id"
#define FMC_USER_SYS_ID   "user_wb_ttc_fmc_regs.user_sys_id"
#define FMC_USER_VERSION  "user_wb_ttc_fmc_regs.user_version"

//------------------------------------------------------------------------------
//GLIB Useful Registers

//SRAM

//SRAM IDs
#define SRAM1             "sram1"
#define SRAM2             "sram2"
// needed to avoid IPBUS bug
#define SRAM1_256         "sram1_256"
#define SRAM2_256         "sram2_256"

//SRAM user logic
#define SRAM1_USR_LOGIC   "ctrl_sram.sram1_user_logic"
#define SRAM2_USR_LOGIC   "ctrl_sram.sram2_user_logic"

//SRAM End Readout
#define SRAM1_END_READOUT "user_wb_ttc_fmc_regs.pc_commands.SRAM1_end_readout"
#define SRAM2_END_READOUT "user_wb_ttc_fmc_regs.pc_commands.SRAM2_end_readout"

//SRAM Full flags, reset when their end readout is 1
#define SRAM1_FULL        "user_wb_ttc_fmc_regs.flags.SRAM1_full"
#define SRAM2_FULL        "user_wb_ttc_fmc_regs.flags.SRAM2_full"

//GLIB

//GLIBS settings
#define FAKE_DATA         "user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_GENE"
#define EXT_TRG           "user_wb_ttc_fmc_regs.pc_commands.TRIGGER_SEL"

//Hybrid
#define HYBRID_TYPE      "hybrid_type"
#define HYBRID_VERSION   "user_wb_ttc_fmc_regs.new.hybrid_version"

//NB FE
#define NB_FE         "nb_FE"

//------------------------------------------------------------------------------
//CBC

//CBC expectation
#define CBC_EXPECTED         "CBC_expected"

//CBC DATA PACKET NUMBER
#define CBC_PACKET_NB        "user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_PACKET_NUMBER"

//CBC TEST PULSE VALID
#define CBC_TEST_PULSE_VALID "COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID"

//CBC Data generator
#define CBC_DATA_GENE        "user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_GENE"

//CBC_TRIGGER_ONE_SHOT
#define CBC_TRIGGER_1SHOT    "user_wb_ttc_fmc_regs.cbc_acquisition.CBC_TRIGGER_ONE_SHOT"

//CBC stubdata latency adjustement
#define CBC_STUB_LATENCY     "cbc_stubdata_latency_adjust"
#define CBC_STUB_LATENCY_FE1 "cbc_stubdata_latency_adjust_fe1"
#define CBC_STUB_LATENCY_FE2 "cbc_stubdata_latency_adjust_fe2"

//CBC I2C command acknoledgement
#define CBC_I2C_CMD_ACK      "cbc_i2c_cmd_ack"
#define CBC_I2C_CMD_ACK_FE1  "cbc_i2c_cmd_ack_fe1"
#define CBC_I2C_CMD_ACK_FE2  "cbc_i2c_cmd_ack_fe2"

//CBC I2C command request
#define CBC_I2C_CMD_RQ       "cbc_i2c_cmd_rq"
#define CBC_I2C_CMD_RQ_FE1   "cbc_i2c_cmd_rq_fe1"
#define CBC_I2C_CMD_RQ_FE2   "cbc_i2c_cmd_rq_fe2"

//CBC Hard Reset
#define CBC_HARD_RESET       "cbc_hard_reset"
#define CBC_HARD_RESET_FE1   "cbc_hard_reset_fe1"
#define CBC_HARD_RESET_FE2   "cbc_hard_reset_fe2"

//CBC Fast Reset
#define CBC_FAST_RESET       "cbc_fast_reset"
#define CBC_FAST_RESET_FE1   "cbc_fast_reset_fe1"
#define CBC_FAST_RESET_FE2   "cbc_fast_reset_fe2"

//Enable Cbc

//FE0
#define ENABLE_FE0_CBC0          "user_wb_ttc_fmc_regs.FE0.CBC0"
#define ENABLE_FE0_CBC1          "user_wb_ttc_fmc_regs.FE0.CBC1"
#define ENABLE_FE0_CBC2          "user_wb_ttc_fmc_regs.FE0.CBC2"
#define ENABLE_FE0_CBC3          "user_wb_ttc_fmc_regs.FE0.CBC3"
#define ENABLE_FE0_CBC4          "user_wb_ttc_fmc_regs.FE0.CBC4"
#define ENABLE_FE0_CBC5          "user_wb_ttc_fmc_regs.FE0.CBC5"
#define ENABLE_FE0_CBC6          "user_wb_ttc_fmc_regs.FE0.CBC6"
#define ENABLE_FE0_CBC7          "user_wb_ttc_fmc_regs.FE0.CBC7"
#define ENABLE_FE0_CBC8          "user_wb_ttc_fmc_regs.FE0.CBC8"
#define ENABLE_FE0_CBC9          "user_wb_ttc_fmc_regs.FE0.CBC9"
#define ENABLE_FE0_CBC10         "user_wb_ttc_fmc_regs.FE0.CBC10"
#define ENABLE_FE0_CBC11         "user_wb_ttc_fmc_regs.FE0.CBC11"
#define ENABLE_FE0_CBC12         "user_wb_ttc_fmc_regs.FE0.CBC12"
#define ENABLE_FE0_CBC13         "user_wb_ttc_fmc_regs.FE0.CBC13"
#define ENABLE_FE0_CBC14         "user_wb_ttc_fmc_regs.FE0.CBC14"
#define ENABLE_FE0_CBC15         "user_wb_ttc_fmc_regs.FE0.CBC15"
#define CBC_FE0_ENABLED          "user_wb_ttc_fmc_regs.FE0.enabled"

//FE1
#define ENABLE_FE1_CBC0          "user_wb_ttc_fmc_regs.FE1.CBC0"
#define ENABLE_FE1_CBC1          "user_wb_ttc_fmc_regs.FE1.CBC1"
#define ENABLE_FE1_CBC2          "user_wb_ttc_fmc_regs.FE1.CBC2"
#define ENABLE_FE1_CBC3          "user_wb_ttc_fmc_regs.FE1.CBC3"
#define ENABLE_FE1_CBC4          "user_wb_ttc_fmc_regs.FE1.CBC4"
#define ENABLE_FE1_CBC5          "user_wb_ttc_fmc_regs.FE1.CBC5"
#define ENABLE_FE1_CBC6          "user_wb_ttc_fmc_regs.FE1.CBC6"
#define ENABLE_FE1_CBC7          "user_wb_ttc_fmc_regs.FE1.CBC7"
#define ENABLE_FE1_CBC8          "user_wb_ttc_fmc_regs.FE1.CBC8"
#define ENABLE_FE1_CBC9          "user_wb_ttc_fmc_regs.FE1.CBC9"
#define ENABLE_FE1_CBC10         "user_wb_ttc_fmc_regs.FE1.CBC10"
#define ENABLE_FE1_CBC11         "user_wb_ttc_fmc_regs.FE1.CBC11"
#define ENABLE_FE1_CBC12         "user_wb_ttc_fmc_regs.FE1.CBC12"
#define ENABLE_FE1_CBC13         "user_wb_ttc_fmc_regs.FE1.CBC13"
#define ENABLE_FE1_CBC14         "user_wb_ttc_fmc_regs.FE1.CBC14"
#define ENABLE_FE1_CBC15         "user_wb_ttc_fmc_regs.FE1.CBC15"
#define CBC_FE1_ENABLED          "user_wb_ttc_fmc_regs.FE1.enabled"

//------------------------------------------------------------------------------
//Delays

//Delay after fast reset
#define DELAY_AF_FAST_RESET "COMMISSIONNING_MODE_DELAY_AFTER_FAST_RESET"

//Delay after L1A
#define DELAY_AF_L1A        "COMMISSIONNING_MODE_DELAY_AFTER_L1A"

//Delay after test pulse
#define DELAY_AF_TEST_PULSE "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE"

//------------------------------------------------------------------------------
//Triggers

//Break Trigger
#define BREAK_TRIGGER    "break_trigger"

//Internal Trigger freq
#define INT_TRIGGER_FREQ "user_wb_ttc_fmc_regs.pc_commands.INT_TRIGGER_FREQ"

//Trigger Select
#define TRIGGER_SELECT   "user_wb_ttc_fmc_regs.pc_commands.TRIGGER_SEL"

// Threshold for DIO5
#define TRIGGER_THRESHOLD "user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_trig_in"

// Trigger Edge
#define TRIGGER_EDGE "user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_edge"

//input termination
#define TRIGGER_IN_TERMINATE "user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_50ohms"

// Output termination
#define TRIGGER_OUT_TERMINATE "user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_out_50ohms"

// Lemo 2 signal
#define LEMO2_SIGNAL "user_wb_ttc_fmc_regs.dio5.fmcdio5_lemo2_sig_sel"

//------------------------------------------------------------------------------
//Clock

// Clock select
#define CLK_SELECT  "user_wb_ttc_fmc_regs.dio5.clk_mux_sel"

// Clock threshold
#define CLK_THRESHOLD "user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_clk_in"

// Input termination
#define CLK_IN_TERMINATE "user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_in_50ohms"

// Output termination
#define CLK_OUT_TERMINATE "user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_out_50ohms"

//------------------------------------------------------------------------------
//Backpressure

#define BACKPRESSURE_POLARITY "user_wb_ttc_fmc_regs.dio5.fmcdio5_backpressure_out_polar"

//------------------------------------------------------------------------------
//Others

//PC Config OK
#define PC_CONFIG_OK    "user_wb_ttc_fmc_regs.pc_commands.PC_config_ok"

//Spurious frame
#define SPURIOUS_FRAME  "user_wb_ttc_fmc_regs.pc_commands.SPURIOUS_FRAME"

//Force BG0 Start
#define FORCE_BG0_START "user_wb_ttc_fmc_regs.pc_commands2.force_BG0_start"

//CMD_START_VALID Flag
#define CMD_START_VALID "user_wb_ttc_fmc_regs.status_flags.CMD_START_VALID"

//FE expectation
#define FE_EXPECTED     "FE_expected"

//RQ
#define RQ              "COMMISSIONNING_MODE_RQ"

// Test Pulse in commissioning mode
#define ENABLE_TP		"COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID"

//Acquisition mode
#define ACQ_MODE        "user_wb_ttc_fmc_regs.pc_commands.ACQ_MODE"

//Clock shift
#define CLOCK_SHIFT     "user_wb_ttc_fmc_regs.pc_commands2.clock_shift"

//Negative logic
#define NEG_LOGIC_CBC   "user_wb_ttc_fmc_regs.pc_commands2.negative_logic_CBC"
#define NEG_LOGIC_STTS  "user_wb_ttc_fmc_regs.pc_commands2.negative_logic_sTTS"

//Polarity
#define POLARITY_TLU       "user_wb_ttc_fmc_regs.pc_commands2.polarity_tlu"

//Time out for stack writing
#define TIME_OUT         5


//------------------------------------------------------------------------------
//Events

//in uint32_t words
#define CBC_EVENT_SIZE_32   9 // 9 32bit words per CBC
#define EVENT_HEADER_TDC_SIZE_32    6 // total of 6 32 bit words for HEADER + TDC
#define EVENT_HEADER_SIZE_32    5  // 5 words for the header

//in chars
#define CBC_EVENT_SIZE_CHAR 9 * 4
#define EVENT_HEADER_TDC_SIZE_CHAR  6 * 4
#define EVENT_HEADER_SIZE_CHAR    5 * 4


//Event
#define OFFSET_BUNCH               8
#define WIDTH_BUNCH                24
#define OFFSET_ORBIT               1*32+8
#define WIDTH_ORBIT                24
#define OFFSET_LUMI                2*32+8
#define WIDTH_LUMI                 24
#define OFFSET_EVENT_COUNT         3*32+8
#define WIDTH_EVENT_COUNT          24
#define OFFSET_EVENT_COUNT_CBC     4*32+8
#define WIDTH_EVENT_COUNT_CBC      3*8

//Cbc Event
#define NCHANNELS                   254
#define OFFSET_ERROR              0
#define WIDTH_ERROR               2
#define OFFSET_PIPELINE_ADDRESS   2       //OFFSET_ERROR + WIDTH_ERROR
#define WIDTH_PIPELINE_ADDRESS    8
#define OFFSET_CBCDATA            2+8     //OFFSET_PIPELINE_ADDRESS + WIDTH_PIPELINE_ADDRESS
#define WIDTH_CBCDATA             254     //NCHANNELS
#define OFFSET_GLIBFLAG           10+254  //OFFSET_CBCDATA + WIDTH_CBCDATA
#define WIDTH_GLIBFLAG            12
#define OFFSET_CBCSTUBDATA        264+23  //LAST BIT
#define WIDTH_CBCSTUBDATA         12
//------------------------------------------------------------------------------
