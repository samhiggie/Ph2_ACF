/*!

        \file                  PulseShape.h
        \brief                 class to do reconstruct the pulse shape of the CBC
        \author              Andrea Massetti & Ali Imran
        \version                1.0
        \date                   20/01/15
        Support :               mail to : georg.auzinger@cern.ch

 */

#ifndef PULSESHAPE_H__
#define PULSESHAPE_H__

#include "Tool.h"
#include "Channel.h"
#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/CommonVisitors.h"
#include "TFile.h"
#include "TString.h"
#include "TMultiGraph.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2F.h"
#include <math.h>

using namespace Ph2_System;
using namespace Ph2_HwInterface;
using namespace Ph2_HwDescription;

/*!
 * \class PulseShape
 * \brief Class to reconstruct pulse shape
 */
typedef std::map<Cbc*, std::vector<Channel*> > ChannelMap;

class PulseShape : public Tool
{
  public:
    /*!
    * \Initialize the istogram
    */
    void Initialize();

    /*!
    * \scan the Vcth with the correspondent delay
    * \param pDelay: initialize the hist whith the pDelay
    */
    void ScanVcth ( uint32_t pDelay );

    /*!
    * \Scan the test pulse delay
    * \param pStepSize: scan the test pulse delay with steps of : pStepSize
    */
    void ScanTestPulseDelay ( uint8_t pStepSize );

  private:

    /*!
    * \brief find the channels of a test group
    * \param pTestGroup: the number of the test group
    * \return the channels in the pTestGroup
    */
    std::vector<uint32_t> findChannelsInTestGroup ( uint32_t pTestGroup );

    /*!
    * \brief parse the xml settings
    */
    void parseSettings();

    /*!
    * \brief set the system test pulse
    * \param pTPAmplitude: the amplitude of the test pulse
    */
    void setSystemTestPulse ( uint8_t pTPAmplitude );

    /*!
    * \brief update the Histogram
    * \param pHistName: the name of the Hist
    * \param pFinal: true if is the last updateHists to be done
    */
    void updateHists ( std::string pHistName, bool pFinal );

    uint32_t fillVcthHist ( BeBoard* pBoard, Event* pEvent, uint32_t pVcth );
    /*!
    * \brief convert the delay before concat to the  test group number
    * \param pDelay: the actual dealy
    */
    void setDelayAndTesGroup ( uint32_t pDelay );

    /*!
    * \brief enable the test group
    */
    void toggleTestGroup (bool pEnable);

    /*!
    * \brief fit the graph with the fitting function
    */
    void fitGraph ( int pLow );

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

    ChannelMap fChannelMap;/*!< Map Cbc vs chennels number */
    bool fFitHist;
    uint32_t fNevents; /*!< Number of events */
    uint32_t fHoleMode;/*!< Check if is in hole mode */
    uint32_t fNCbc;/*!< Number of CBCs */
    uint8_t fVplus;/*!< Postamp  bias voltage */
    uint8_t fTestGroup; /*!< Number of the test group */
    uint8_t fTPAmplitude; /*!< Test pulse Amplitude */
    uint32_t fDelayAfterPulse ; // Delay after test pulse
    uint32_t fChannel; /*!< channel number */
    uint8_t fOffset; /*!< Offset value for the channel */
    uint32_t fStepSize; /*!< Step size */
    std::vector<uint32_t> fChannelVector;  /*!< Channels in the test group */
    ChipType fType;

    unsigned char fLookup[16] =
    {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
    }; /*!< Lookup table for reverce the endianness */

    const std::string getDelAfterTPString (BoardType pBoardType)
    {

        if (pBoardType == BoardType::GLIB || pBoardType == BoardType::CTA) return "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE";
        else if (pBoardType == BoardType::ICGLIB || pBoardType == BoardType::ICFC7) return "cbc_daq_ctrl.commissioning_cycle.test_pulse_count";
        else return "not recognized";
    }

};

/*!
* \fitting function
* \param x: the amplitude of the fitting function
* \param par: array with the parameters of the fitting function
* \return the point of the fitting function
*/
double pulseshape ( double* x, double* par );

#endif
