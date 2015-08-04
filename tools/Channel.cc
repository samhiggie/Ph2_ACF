#include "Channel.h"
#include "TMath.h"
#include <cmath>



Channel::Channel( uint8_t pBeId, uint8_t pFeId, uint8_t pCbcId, uint8_t pChannelId ) :
	fBeId( pBeId ),
	fFeId( pFeId ),
	fCbcId( pCbcId ),
	fChannelId( pChannelId )
{
}


Channel::~Channel()
{
}

double Channel::getPedestal() const
{
	if (fFitted) {
		if ( fFit != nullptr )
			return fabs( fFit->GetParameter( 0 ) );
		else return -1;
	} else {
		if( fDerivative != nullptr)
			return fabs( fDerivative->GetMean() );
		else return -1;
	}	
}

double Channel::getNoise() const
{

	if (fFitted) {
		if ( fFit != nullptr )
			return fabs( fFit->GetParameter( 1 ) );
		else return -1;
	} else {
		if( fDerivative != nullptr)
			return fabs( fDerivative->GetRMS() );
		else return -1;
	}
}

void Channel::setOffset( uint8_t pOffset )
{
	fOffset = pOffset;
}

void Channel::initializeHist( uint8_t pValue, TString pParameter )
{

	TString histname;
	TString fitname;
	TString graphname;

	pParameter += Form( "%d", pValue );
	histname = Form( "Scurve_Be%d_Fe%d_Cbc%d_Channel%d", fBeId, fFeId, fCbcId, fChannelId );
	histname += pParameter;
	fitname = Form( "Fit_Be%d_Fe%d_Cbc%d_Channel%d", fBeId, fFeId, fCbcId, fChannelId );
	fitname += pParameter;
	graphname = Form( "fDerivative_Be%d_Fe%d_Cbc%d_Channel%d", fBeId, fFeId, fCbcId, fChannelId );	
	graphname += pParameter;


	fScurve = dynamic_cast<TH1F*>( gROOT->FindObject( histname ));
	if ( fScurve ) delete fScurve;
	fScurve = new TH1F( histname, Form( "Scurve_Be%d_Fe%d_Cbc%d_Channel%d", fBeId, fFeId, fCbcId, fChannelId ), 256, -0.5, 255.5 );

	fDerivative = dynamic_cast<TH1F*>( gROOT->FindObject( graphname ));
	if ( fDerivative ) delete fDerivative;
	fDerivative = new TH1F( graphname, Form( "Derivative_Scurve_Be%d_Fe%d_Cbc%d_Channel%d", fBeId, fFeId, fCbcId, fChannelId ), 255, 0, 255 );	

	fScurve->SetMarkerStyle( 7 );
	fScurve->SetMarkerSize( 2 );

	fFit = dynamic_cast< TF1* >( gROOT->FindObject( fitname ));
	if ( fFit ) delete fFit;
	// TF1 *f1=gROOT->GetFunction("myfunc");
	fFit = new TF1( fitname, MyErf, 0, 255, 2 );
}

void Channel::fillHist( uint8_t pVcth )
{
	fScurve->Fill( float( pVcth ) );
}

void Channel::fitHist( uint32_t pEventsperVcth, bool pHole, uint8_t pValue, TString pParameter, TFile* pResultfile )
{
	fFitted = true;

	if ( fScurve != nullptr && fFit != nullptr )
	{

		// Normalize first
		// fScurve->Sumw2();
		fScurve->Scale( 1 / double_t( pEventsperVcth ) );

		// Get first non 0 and first 1
		double cFirstNon0( 0 );
		double cFirst1( 0 );

		// Not Hole Mode
		if ( !pHole )
		{
			for ( Int_t cBin = 1; cBin <= fScurve->GetNbinsX(); cBin++ )
			{
				double cContent = fScurve->GetBinContent( cBin );
				if ( !cFirstNon0 )
				{
					if ( cContent ) cFirstNon0 = fScurve->GetBinCenter( cBin );
				}
				else if ( cContent == 1 )
				{
					cFirst1 = fScurve->GetBinCenter( cBin );
					break;
				}
			}
		}
		// Hole mode
		else
		{
			for ( Int_t cBin = fScurve->GetNbinsX(); cBin >= 1; cBin-- )
			{
				double cContent = fScurve->GetBinContent( cBin );
				if ( !cFirstNon0 )
				{
					if ( cContent ) cFirstNon0 = fScurve->GetBinCenter( cBin );
				}
				else if ( cContent == 1 )
				{
					cFirst1 = fScurve->GetBinCenter( cBin );
					break;
				}
			}
		}

		// Get rough midpoint & width
		double cMid = ( cFirst1 + cFirstNon0 ) * 0.5;
		double cWidth = ( cFirst1 - cFirstNon0 ) * 0.5;

		fFit->SetParameter( 0, cMid );
		fFit->SetParameter( 1, cWidth );

		// Fit
		fScurve->Fit( fFit, "RNQ+" );

		// Eventually add TFitResultPointer
		// create a Directory in the file for the current Offset and save the channel Data
		TString cDirName;
		cDirName = pParameter + Form( "%d", pValue );
		TDirectory* cDir = dynamic_cast< TDirectory* >( gROOT->FindObject( cDirName ) );
		if ( !cDir ) cDir = pResultfile->mkdir( cDirName );
		pResultfile->cd( cDirName );

		fScurve->SetDirectory( cDir );
		fFit->Write(fFit->GetName(), TObject::kOverwrite);
		// pResultfile->Flush();

		pResultfile->cd();
	}
	else std::cout << "Historgram Empty for Fe " << fFeId << " Cbc " << fCbcId << " Channel " << fChannelId << std::endl;


}


void Channel::differentiateHist( uint32_t pEventsperVcth, bool pHole, uint8_t pValue, TString pParameter, TFile* pResultfile )
{
	fFitted = false;

	if ( fScurve != nullptr && fFit != nullptr )
	{

		// Normalize first
		// fScurve->Sumw2();
		fScurve->Scale( 1 / double_t( pEventsperVcth ) );

		// Histogram of Differences

		double_t cPrev = fScurve->GetBinContent(fScurve->GetBin(-0.5));
		double_t cDiff;
		double_t cCurrent;
		bool cActive; // indicates existence of data points
		int cStep = 1;
		
		double cValue = 0;
		if (pHole) {
			cActive = false;	
			while (0 <= cValue && cValue<= 255){		
				cCurrent = fScurve->GetBinContent(fScurve->GetBin(cValue));
				cDiff = cPrev - cCurrent;
				if (cPrev == 1) cActive = true; // sampling begins 
				if (cActive) fDerivative->SetBinContent(fDerivative->GetBin(cValue-0.5), cDiff);
				cPrev = cCurrent;
				cValue +=cStep;
			}
		} else {
			cActive = true;
			while (-0.5 <= cValue && cValue<= 255.5){		
				cCurrent = fScurve->GetBinContent(fScurve->GetBin(cValue));
				cDiff = cCurrent - cPrev;
				if (cDiff == -1) cActive = false; // sampling ends 
				if (cActive) fDerivative->SetBinContent(fDerivative->GetBin(cValue-0.5), cDiff);
				cPrev = cCurrent;
				cValue +=cStep;
			}
		}

		// Eventually add TDerivativeResultPointer
		// create a Directory in the file for the current Offset and save the channel Data
		TString cDirName;
		cDirName = pParameter + Form( "%d", pValue );
		TDirectory* cDir = dynamic_cast< TDirectory* >( gROOT->FindObject( cDirName ) );
		if ( !cDir ) cDir = pResultfile->mkdir( cDirName );
		pResultfile->cd( cDirName );

		fScurve->SetDirectory( cDir );
		fDerivative->Write(fDerivative->GetName(), TObject::kOverwrite);
		// pResultfile->Flush();

		pResultfile->cd();
	}
	else std::cout << "Historgram Empty for Fe " << fFeId << " Cbc " << fCbcId << " Channel " << fChannelId << std::endl;


}


void Channel::resetHist()
{

	// fScurve = nullptr;
	// fFit = nullptr;

}


TestGroup::TestGroup( uint8_t pShelveId, uint8_t pBeId, uint8_t pFeId, uint8_t pCbcId, uint8_t pGroupId ) :
	fShelveId( pShelveId ),
	fBeId( pBeId ),
	fFeId( pFeId ),
	fCbcId( pCbcId ),
	fGroupId( pGroupId )
{
}


TestGroupGraph::TestGroupGraph()
{
	fVplusVcthGraph = nullptr;
}


TestGroupGraph::TestGroupGraph( uint8_t pBeId, uint8_t pFeId, uint8_t pCbcId, uint8_t pGroupId )
{
	TString graphname = Form( "VplusVcthGraph_Fe%d_Cbc%d_Group%d", pFeId, pCbcId, pGroupId );
	fVplusVcthGraph = dynamic_cast<TGraphErrors*>( gROOT->FindObject( graphname ));
	if ( fVplusVcthGraph ) delete fVplusVcthGraph;
	fVplusVcthGraph = new TGraphErrors();
	fVplusVcthGraph->SetName( graphname );
}

void TestGroupGraph::FillVplusVcthGraph( uint8_t& pVplus, double pPedestal, double pNoise )
{

	if ( fVplusVcthGraph != nullptr )
	{
		fVplusVcthGraph->SetPoint( fVplusVcthGraph->GetN(), pPedestal, pVplus );
		fVplusVcthGraph->SetPointError( fVplusVcthGraph->GetN() - 1, pNoise, 0 );
	}
}



