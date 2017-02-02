/*!

        \file                   BiasSweep.h
        \brief                  Class to sweep one of the CBC2/3 biases and perform an analog measurement via the AnalogMux and Ke2110 DMM
        \author                 Georg AUZINGER
        \version                1.0
        \date                   31/10/16
        Support :               mail to : georg.auzinger@SPAMNOT.cern.ch

 */


#ifndef __STUBSWEEP_H__
#define __STUBWEEP_H__
#include "Tool.h"
#include <map>
#include <vector>

#include "TProfile.h"
#include "TGraph.h"
#include "TObject.h"
#include "TAxis.h"
#include "TTree.h"
#include "TString.h"
#include "Utils/CommonVisitors.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

class StubSweep : public Tool
{
  public:
    StubSweep();
    ~StubSweep();
    void Initialize();

    // added for debugging - S.
    // ******
    // *******
    void SweepStubs (uint32_t pNEvents = 1 );



  private:
    TCanvas* fSweepCanvas;

    ChipType fType;
    //settings
    uint8_t fDelay;
    uint8_t fReadBackAttempts;

    // methods to fill/update histograms
    void updateHists ( std::string pHistname );
    void fillStubBendHist ( Cbc* pCbc, std::vector<uint8_t> pChannelPair, uint8_t pStubBend );
    void fillStubSweepHist ( Cbc* pCbc, std::vector<uint8_t> pChannelPair, uint8_t pStubPosition );

    // method to configure test pulse on the CBC
    void configureTestPulse (Cbc* pCbc, uint8_t pPulseState);

    // method to mask all channels on the CBC
    void maskAllChannels (Cbc* pCbc);
    // method to return the position of the first stub in a CBC event
    uint8_t getStubPosition (std::vector<Event*> pEvents, uint32_t pFeId, uint32_t pCbcId, uint32_t pNEvents);

    /*!
    * \brief return mask for a given channel
    * \param pTestGroup: the  channel number [ between 1 and 254 ]
    */
    uint8_t getChanelMask ( Cbc* pCbc, uint8_t pChannel );

    /*!
    * \brief find the channels of a test group
    * \param pTestGroup: the number of the test group
    * \return the channels in the pTestGroup
    */
    std::vector<uint8_t> findChannelsInTestGroup ( uint8_t pTestGroup );
    /*!
    * \brief reverse the byte
    * \param n:the number to be reversed
    * \return the reversed number
    */
    uint8_t reverse ( uint8_t n )
    {
        // Reverse the top and bottom nibble then swap them.
        return ( fLookup[n & 0xF] << 4 ) | fLookup[n >> 4];
    }

    /*!
    * \brief reverse the endianess before writing in to the register
    * \param pDelay: the actual delay
    * \param pGroup: the actual group number
    * \return the reversed endianness
    */
    uint8_t to_reg ( uint8_t pDelay, uint8_t pGroup )
    {

        uint8_t cValue = ( ( reverse ( pDelay ) ) & 0xF8 ) |
                         ( ( reverse ( pGroup ) ) >> 5 );

        //std::cout << std::bitset<8>( cValue ) << " cGroup " << +pGroup << " " << std::bitset<8>( pGroup ) << " pDelay " << +pDelay << " " << std::bitset<8>( pDelay ) << std::endl;
        return cValue;
    }
    unsigned char fLookup[16] =
    {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
    }; /*!< Lookup table for reverce the endianness */

    std::map<double, uint8_t> fWindowOffsetMapCBC3 =
    {
        {0.0, 0x00 }, { 0.5, 0x01}, { 1.0, 0x02}, { 1.5, 0x03}, { 2.0, 0x04}, { 2.5, 0x05}, { 3.0, 0x06},
        {-0.5, 0x0f}, {-1.0, 0x0e}, {-1.5, 0x0d}, {-2.0, 0x0c}, {-2.5, 0x0b}, {-3.0, 0x0a}
        // { -3.0 , 0x06}, {-2.5, 0x05 } , {-2.0 , 0x04} , {-1.5, 0x03} , {-1.0, 0x02} , {-0.5,0x01} , {0,0x00},
        // { 3.0  , 0x0a} , {2.5 , 0x0b} , {2.0 , 0x0c } , {1.5,0x0d}, {1.0,0x0e}  , {0.5, 0x0f}
    };

    void setCorrelationWinodwOffsets ( Cbc* pCbc, double pOffsetR1, double pOffsetR2, double pOffsetR3, double pOffsetR4 );

    std::map<uint8_t, std::string> fChannelMaskMapCBC2 =
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
    };

    std::map<uint8_t, std::string> fChannelMaskMapCBC3 =
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
    };


    void writeObjects();

};




#endif
