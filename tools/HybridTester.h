/*!

        \file                   HybridTester.h
        \brief                 class for validating x*CBC2 hybrids
        \author              Alireza KOKABI & Georg AUZINGER
        \version                1.0
        \date                   24/10/14
        Support :               mail to : georg.auzinger@cern.ch

 */

#ifndef HybridTester_h__
#define HybridTester_h__
#define CH_DIAGNOSIS_DECISION_TH 80 // decision threshold value for channels diagnosis, expressed in % from 0 to 100

#include "../HWDescription/Module.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../System/SystemController.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/Antenna.h"
#include "../System/SystemController.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLine.h"
#include "Channel.h"
#include <fstream>
#include <time.h>
#include <sstream>
#include <vector>

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

typedef std::map< int, std::vector<uint8_t> >  TestGroupChannelMap;
typedef std::map<Cbc*,  std::map< uint8_t, uint8_t > > CalibratedOffsetMap;
typedef std::vector<std::pair< std::string, uint8_t> > RegisterVector;

/*!
 * \class HybridTester
 * \brief Class to test x*CBC2 Hybrids
 */
class HybridTester : public Antenna, public SystemController
{
  public:
	// Default C'tor
	HybridTester() {
		for ( int cGid = -1; cGid < 8; cGid++ ) {
			std::vector<uint8_t> cTempChannelVec;
			if ( cGid > -1 ) {
				for ( int cIdx = 0; cIdx < 16; cIdx++ ) {
					
					int cTemp1 = cIdx * 16 + cGid * 2;
					int cTemp2 = cTemp1 + 1;
					if ( cTemp1 < 254 ) cTempChannelVec.push_back( cTemp1 );
					if ( cTemp2 < 254 )  cTempChannelVec.push_back( cTemp2 );
					// here need to still fill the calibrated offset Map
					/*
					uint8_t cTemp1 = cIdx + cGid * 16;
					if ( cTemp1 < 254 ) cTempChannelVec.push_back( cTemp1 );
					*/
					
				}
			}
			else {
				for ( int cIdx = 0; cIdx < 254; cIdx++ )
					cTempChannelVec.push_back( cIdx );
			}
			fTestGroupChannelMap[cGid] = cTempChannelVec;
		}
	
	// here we need to read the initial offset values and fill it in fOffsetMap
	}

	// Default D'tor
	~HybridTester() {}
	/*!
	* \brief Initialize the Histograms and Canvasses for CMD line applications
	* \param pThresholdScan :  bool flag to initialize the additional canvas for the Threshold scan
	*/
	void Initialize( bool pThresholdScan );
	/*!
	* \brief Initialize the Histograms and Canvasses for GUI applications
	* \param pThresholdScan :  bool flag to initialize the additional canvas for the Threshold scan
	* \param pCanvasVector: vector of TCanvas* to be passed by the GUI to draw on
	*/
	void InitializeGUI( bool pThresholdScan, const std::vector<TCanvas*>& pCanvasVector );
	/*!
	* \brief Test CBC registers by writing complimentary bit patterns (0x55, 0xAA)
	*/
	void TestRegisters();
	/*!
	* \brief Scan the thresholds to identify the threshold with no noise occupancy
	*/
	void ScanThreshold();
	/*!
	* \brief Measure the single strip efficiency
	*/
	void Measure();
	/*!
	* \brief private method that configures SPI interface between CP2130 and slave analog switch
	*/
	void ConfigureSpiSlave(usb_dev_handle* pUsbHandle, uint8_t pSlaveChipSelectId);
	/*!
	* \brief private method that switches on given channel of last analog switch for which SPI interface was configured
	*/
	void TurnOnAnalogSwitchChannel(usb_dev_handle* pUsbHandle, uint8_t pSwichChannelId);
	/*!
	* \brief private method that checks channels malfunction based on occupancy histograms, produces output report in .txt format
	*/
	void TestChannels();
	/*!
	* \brief overload method that checks channels malfunction based on occupancy histograms, produces output report in .txt format, does not rely on shared arrays
	*/
	void TestChannels(double pTopHistogram[], int pTopHistogramSize, double pBottomHistogram[], int pBottomHistogramSize, double pDecisionThreshold);
	/*!
	* \brief Save the results to the file created with SystemController::InitializeResultFile
	*/
	void SaveResults();
	
	void ConstructTestGroup( uint8_t pShelveId, uint8_t pBeId, uint8_t pFeId, uint8_t pCbcId );


  private:
	uint32_t fNCbc;   /*!< Number of CBCs in the Setup */
	TCanvas* fDataCanvas;   /*!<Canvas to output single-strip efficiency */
	TH1F* fHistTop;   /*!< Histogram for top pads */
	TH1F* fHistBottom;   /*!< Histogram for bottom pads */
	bool fThresholdScan; /*!< Flag for SCurve Canvas */
	TestGroupChannelMap fTestGroupChannelMap; /*!< Map of Channels for groupwise measuring of efficiency */
	CalibratedOffsetMap fOffsetMap;
	TCanvas* fSCurveCanvas;   /*!< Canvas for threshold scan */
	std::map<Cbc*, TH1F*> fSCurveMap;  /*!< Histograms for SCurve */
	std::map<Cbc*, TF1*> fFitMap;   /*!< fits for SCurve*/
	double fTopHistogramMerged[255] = {0};
	double fBottomHistogramMerged[255] = {0};
	uint32_t fTotalEvents;

	double fDecisionThreshold = 10.0;   /*!< Decision Threshold for channels occupancy based tests, values from 1 to 100 as % */

	//double fChannelDiagnosisThreshold;
	/*!
	* \brief private method that sets offset for a particular test group
	*/
	void setOffset( uint8_t pOffset, int  pTGrpId );
	void setOffset( int pTGrpId );
	
	
	void InitializeHists();
	/*!
	* \brief private method to periodically update the output graphs
	*/
	void UpdateHists() {
		fDataCanvas->cd( 1 );
		fHistTop->Draw();
		fDataCanvas->cd( 2 );
		fHistBottom->Draw();
		fDataCanvas->Update();
	}

	// To measure the occupancy per Cbc
	uint32_t fillSCurves( BeBoard* pBoard,  const Event* pEvent, uint8_t pValue );
	void updateSCurveCanvas( BeBoard* pBoard );
	void processSCurves( uint32_t pEventsperVcth );

	const std::string currentDateTime() {
   		time_t now = time(0);
   		struct tm  tstruct;
    		char buf[80];
   		tstruct = *localtime(&now);
    		strftime(buf, sizeof(buf), "%Y-%m-%d_at_%H-%M-%S", &tstruct);
   		return buf;
	}




std::string DecimalToBinaryString(int a)
{
    std::string binary = "";
    int mask = 1;
    for(int i = 0; i < 31; i++)
    {
        if((mask&a) >= 1)
            binary = "1"+binary;
        else
            binary = "0"+binary;
        mask<<=1;
    }
    return binary;
}

std::string CharToBinaryString(int a)
{
    std::string binary = "";
    int mask = 1;
    for(int i = 0; i < 7; i++)
    {
        if((mask&a) >= 1)
            binary = "1"+binary;
        else
            binary = "0"+binary;
        mask<<=1;
    }
    return binary;
}

void print_buffer(char* buf, int8_t buf_size)
{
    for(uint8_t i=0; i<buf_size; i++)
    {
        std::cout<<buf[i]+0<<" ";
    }
    std::cout<<std::endl;
}

std::string int_vector_to_string(std::vector<int> int_vector)
{
    std::string output_string = "";
    for (std::vector<int>::iterator it = int_vector.begin(); it != int_vector.end(); ++it)
    {
        output_string += patch::to_string(*it) + "; ";
    }
    return output_string;
}

std::string double_vector_to_string(std::vector<double> int_vector)
{
    std::string output_string = "";
    for (std::vector<double>::iterator it = int_vector.begin(); it != int_vector.end(); ++it)
    {
        output_string += patch::to_string(*it) + "; ";
    }
    return output_string;
}

	

};


#endif


