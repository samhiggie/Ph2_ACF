/*

    \file                          Definition.h
    \brief                         Definition File, listing the registers
    \author                        Nicolas PIERRE
    \version                       1.0
    \date                          07/06/14
    Support :                      mail to : nico.pierre@icloud.com

 */
#ifndef _DEFINITION_H__
#define _DEFINITION_H__

#include <iostream>



//-----------------------------------------------------------------------------
//Glib Config Files

#define XML_DESCRIPTION_FILE_2CBC "settings/ICDescription.xml"
#define XML_DESCRIPTION_FILE_4CBC "settings/HWDescription_2CBC.xml"
#define XML_DESCRIPTION_FILE_8CBC "settings/HWDescription_8CBC.xml"
#define XML_DESCRIPTION_FILE_16CBC "settings/Beamtest_Nov15.xml"


//Time out for stack writing
#define TIME_OUT         5


//------------------------------------------------------------------------------
#define NCHANNELS                   254
//Events

//CBC2
//in uint32_t words
#define CBC_EVENT_SIZE_32   9 // 9 32bit words per CBC
#define EVENT_HEADER_TDC_SIZE_32    6 // total of 6 32 bit words for HEADER + TDC
#define EVENT_HEADER_SIZE_32    5  // 5 words for the header

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
#define OFFSET_ERROR              0
#define WIDTH_ERROR               2
#define OFFSET_PIPELINE_ADDRESS   2       //OFFSET_ERROR + WIDTH_ERROR
#define WIDTH_PIPELINE_ADDRESS    8
#define OFFSET_CBCDATA            2+8     //OFFSET_PIPELINE_ADDRESS + WIDTH_PIPELINE_ADDRESS
#define WIDTH_CBCDATA             254     //NCHANNELS
#define OFFSET_GLIBFLAG           10+254  //OFFSET_CBCDATA + WIDTH_CBCDATA
#define WIDTH_GLIBFLAG            12
#define OFFSET_CBCSTUBDATA        264+23  //LAST BIT
#define IC_OFFSET_CBCSTUBDATA        276  //BIT 12
#define WIDTH_CBCSTUBDATA         12

//CBC3
//in uint32_t words
#define CBC_EVENT_SIZE_32_CBC3   11 // 11 32bit words per CBC
#define EVENT_HEADER_TDC_SIZE_32_CBC3    3 // total of 6 32 bit words for HEADER + TDC
#define EVENT_HEADER_SIZE_32_CBC3    3  // 5 words for the header

//Event
//#define OFFSET_BUNCH               8
//#define WIDTH_BUNCH                24
//#define OFFSET_ORBIT               1*32+8
//#define WIDTH_ORBIT                24
//#define OFFSET_LUMI                2*32+8
//#define WIDTH_LUMI                 24
#define OFFSET_EVENT_COUNT_CBC3         2*32+3
#define WIDTH_EVENT_COUNT_CBC3          29

//Cbc Event
#define OFFSET_EVENT_COUNT_CBC_CBC3     2*32+4
#define WIDTH_EVENT_COUNT_CBC_CBC3      9
#define OFFSET_ERROR_CBC3              2*32+22
#define WIDTH_ERROR_CBC3               2
#define OFFSET_PIPELINE_ADDRESS_CBC3   2*32+13       //OFFSET_ERROR + WIDTH_ERROR
#define WIDTH_PIPELINE_ADDRESS_CBC3    9
#define OFFSET_CBCDATA_CBC3            2*32+4     //OFFSET_PIPELINE_ADDRESS + WIDTH_PIPELINE_ADDRESS
#define WIDTH_CBCDATA_CBC3             254     //NCHANNELS
#define OFFSET_GLIBFLAG_CBC3           10+254  //OFFSET_CBCDATA + WIDTH_CBCDATA
#define WIDTH_GLIBFLAG_CBC3            12
#define OFFSET_CBCSTUBDATA_CBC3        264+23  //LAST BIT
#define WIDTH_CBCSTUBDATA         12
//------------------------------------------------------------------------------

enum class BoardType {GLIB, ICGLIB, CTA, ICFC7, CBC3FC7, SUPERVISOR};
enum class ChipType {UNDEFINED = 0, CBC2, CBC3};

std::map<uint8_t, std::string> ChannelMaskMapCBC2 =
{
    { 0, "MaskChannelFrom008downto001" },
    { 1, "MaskChannelFrom016downto009" },
    { 2, "MaskChannelFrom024downto017" },
    { 3, "MaskChannelFrom032downto025" },
    { 4, "MaskChannelFrom040downto033" },
    { 5, "MaskChannelFrom048downto041" },
    { 6, "MaskChannelFrom056downto049" },
    { 7, "MaskChannelFrom064downto057" },
    { 8, "MaskChannelFrom072downto065" },
    { 9, "MaskChannelFrom080downto073" },
    {10, "MaskChannelFrom088downto081" },
    {11, "MaskChannelFrom096downto089" },
    {12, "MaskChannelFrom104downto097" },
    {13, "MaskChannelFrom112downto105" },
    {14, "MaskChannelFrom120downto113" },
    {15, "MaskChannelFrom128downto121" },
    {16, "MaskChannelFrom136downto129" },
    {17, "MaskChannelFrom144downto137" },
    {18, "MaskChannelFrom152downto145" },
    {19, "MaskChannelFrom160downto153" },
    {20, "MaskChannelFrom168downto161" },
    {21, "MaskChannelFrom176downto169" },
    {22, "MaskChannelFrom184downto177" },
    {23, "MaskChannelFrom192downto185" },
    {24, "MaskChannelFrom200downto193" },
    {25, "MaskChannelFrom208downto201" },
    {26, "MaskChannelFrom216downto209" },
    {27, "MaskChannelFrom224downto217" },
    {28, "MaskChannelFrom232downto225" },
    {29, "MaskChannelFrom240downto233" },
    {30, "MaskChannelFrom248downto241" },
    {31, "MaskChannelFrom254downto249" }
}

std::map<uint8_t, std::string> ChannelMaskMapCBC3 =
{
    { 0, "MaskChannel-008-to-001" },
    { 1, "MaskChannel-016-to-009" },
    { 2, "MaskChannel-024-to-017" },
    { 3, "MaskChannel-032-to-025" },
    { 4, "MaskChannel-040-to-033" },
    { 5, "MaskChannel-048-to-041" },
    { 6, "MaskChannel-056-to-049" },
    { 7, "MaskChannel-064-to-057" },
    { 8, "MaskChannel-072-to-065" },
    { 9, "MaskChannel-080-to-073" },
    {10, "MaskChannel-088-to-081" },
    {11, "MaskChannel-096-to-089" },
    {12, "MaskChannel-104-to-097" },
    {13, "MaskChannel-112-to-105" },
    {14, "MaskChannel-120-to-113" },
    {15, "MaskChannel-128-to-121" },
    {16, "MaskChannel-136-to-129" },
    {17, "MaskChannel-144-to-137" },
    {18, "MaskChannel-152-to-145" },
    {19, "MaskChannel-160-to-153" },
    {20, "MaskChannel-168-to-161" },
    {21, "MaskChannel-176-to-169" },
    {22, "MaskChannel-184-to-177" },
    {23, "MaskChannel-192-to-185" },
    {24, "MaskChannel-200-to-193" },
    {25, "MaskChannel-208-to-201" },
    {26, "MaskChannel-216-to-209" },
    {27, "MaskChannel-224-to-217" },
    {28, "MaskChannel-232-to-225" },
    {29, "MaskChannel-240-to-233" },
    {30, "MaskChannel-248-to-241" },
    {31, "MaskChannel-254-to-249" }
}
#endif
