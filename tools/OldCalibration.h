/*!
 *
 * \file Calibration.h
 * \brief Calibration class, calibration of the hardware
 * \author Lorenzo BIDEGAIN
 * \date 13/08/14
 *
 * Support : no support!
 *
 */

#ifndef Calibration_h__
#define Calibration_h__

#include "Tool.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/CommonVisitors.h"
#include "Channel.h"

#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TFitResultPtr.h"
#include "TStyle.h"

#include <time.h>
#include <map>
// #include <system>

#define READBACK true

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

/*!
 * \class Calibration
 * \brief Read/Write Cbc's registers on a file
 */
class Calibration : public Tool
{

  public:
	// Default C'tor
	Calibration();

	// D'Tor
	~Calibration();

	void InitialiseTestGroup();

	void VplusScan();
	void OffsetScan();

	void SaveResults();

  private:
	TestGroupMap fTestGroupMap;
	TestGroupGraphMap fTestGroupGraphMap;
	std::vector<uint8_t> fVplusValues;
	std::map<Cbc*, TCanvas*> fCbcCanvasMap;

  private:
	void ConstructTestGroup( uint8_t pShelveId, uint8_t pBeId, uint8_t pFeId, uint8_t pCbcId );

	void FitVplusVcth( BeBoard* pBoard, uint8_t pTargetVcth,  bool pDoDraw );
	void setGlobalReg( BeBoard* pBoard, const std::string& pRegName, uint8_t pRegValue );
	void initializeSCurves( BeBoard* pBoard, uint8_t pGroupId, uint8_t pValue, TString pParameter );
	void measureSCurves( BeBoard* pBoard, uint8_t pGroupId, uint32_t pEventsperVcth, uint32_t pTotalChannels, bool pHoleMode );
	void processSCurves( BeBoard* pBoard, uint8_t pGroupId, uint32_t pEventsperVcth, uint8_t pValue, TString pParameter, bool pHoleMode, bool pDoDraw );
	uint32_t fillScurveHists( BeBoard* pBoard, uint8_t pGroupId, uint8_t pVcth, const Event* pEvent );
	uint32_t ToggleTestGroup( BeBoard* pBoard, uint8_t pGroupId, bool pHoleMode, bool pEnable );
	uint32_t SetOffsetTargetBitTestGroup( BeBoard* pBoard, uint8_t pGroupId, bool pHoleMode, uint8_t pTargetBit, uint8_t pTargetVcth );
	void processSCurvesOffset( BeBoard* pBoard, uint8_t pGroupId, uint32_t pEventsperVcth, uint8_t pTargetVcth, uint8_t pTargetBit, TString pParameter, bool pHoleMode, bool pDoDraw );
	void UpdateCbcObject( BeBoard* pBoard, uint8_t pGroupId );

	// Use BeBoard reference
	void FitVplusVcth( BeBoard& pBoard, uint8_t pTargetVcth,  bool pDoDraw ) {
		FitVplusVcth( &pBoard, pTargetVcth,  pDoDraw );
	}
	void setGlobalReg( BeBoard& pBoard, const std::string& pRegName, uint8_t pRegValue ) {
		setGlobalReg( &pBoard, pRegName, pRegValue );
	}
	void initializeSCurves( BeBoard& pBoard, uint8_t pGroupId, uint8_t pValue, TString pParameter ) {
		initializeSCurves( &pBoard, pGroupId, pValue, pParameter );
	}
	void measureSCurves( BeBoard& pBoard, uint8_t pGroupId, uint32_t pEventsperVcth, uint32_t pTotalChannels, bool pHoleMode ) {
		measureSCurves( &pBoard, pGroupId, pEventsperVcth, pTotalChannels, pHoleMode );
	}
	void processSCurves( BeBoard& pBoard, uint8_t pGroupId, uint32_t pEventsperVcth, uint8_t pValue, TString pParameter, bool pHoleMode, bool pDoDraw ) {
		processSCurves( &pBoard, pGroupId, pEventsperVcth, pValue, pParameter, pHoleMode, pDoDraw );
	}
	uint32_t fillScurveHists( BeBoard& pBoard, uint8_t pGroupId, uint8_t pVcth, const Event* pEvent ) {
		return fillScurveHists( &pBoard, pGroupId, pVcth, pEvent );
	}
	uint32_t ToggleTestGroup( BeBoard& pBoard, uint8_t pGroupId, bool pHoleMode, bool pEnable ) {
		return ToggleTestGroup( &pBoard, pGroupId, pHoleMode, pEnable );
	}
	uint32_t SetOffsetTargetBitTestGroup( BeBoard& pBoard, uint8_t pGroupId, bool pHoleMode, uint8_t pTargetBit, uint8_t pTargetVcth ) {
		return SetOffsetTargetBitTestGroup( &pBoard, pGroupId, pHoleMode, pTargetBit, pTargetVcth );
	}
	void processSCurvesOffset( BeBoard& pBoard, uint8_t pGroupId, uint32_t pEventsperVcth, uint8_t pTargetVcth, uint8_t pTargetBit, TString pParameter, bool pHoleMode, bool pDoDraw ) {
		processSCurvesOffset( &pBoard, pGroupId, pEventsperVcth, pTargetVcth, pTargetBit, pParameter, pHoleMode, pDoDraw );
	}
	void UpdateCbcObject( BeBoard& pBoard, uint8_t pGroupId ) {
		UpdateCbcObject( &pBoard, pGroupId );
	}
};

#endif
