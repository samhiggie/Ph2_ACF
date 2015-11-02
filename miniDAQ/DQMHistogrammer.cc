/*!
        \file                DQMHistogrammer.h
        \brief               base class to create and fill monitoring histograms
        \author              Suchandra Dutta and Subir Sarkar
        \version             1.0
        \date                13/10/15
        Support :            mail to : Suchandra.Dutta@cern.ch, Subir.Sarkar@cern.ch
*/

#include "TROOT.h"
#include "TH1.h"
#include "TFile.h"
#include "TString.h"
#include "TDirectory.h"
#include "TTree.h"

#include "../Utils/Event.h"

#include "DQMHistogrammer.h"

DQMHistogrammer::DQMHistogrammer(bool addTree) 
  : addTree_(addTree)
{
}

DQMHistogrammer::~DQMHistogrammer() {
  if (dut0HitProfH_) delete dut0HitProfH_;
  if (dut1HitProfH_) delete dut1HitProfH_;
  if (sensCorrH_) delete sensCorrH_;
  if (l1AcceptH_) delete l1AcceptH_;
  if (tdcCounterH_) delete tdcCounterH_;

  for ( auto& imap : cbcHMap_ ) {
    CBCHistos& cbc_h = imap.second;
    delete cbc_h.errBitH;
    delete cbc_h.nStubsH;
    delete cbc_h.evenChnOccuH;
    delete cbc_h.oddChnOccuH;
  }
  if (addTree_ && tree_) delete tree_;
}
void DQMHistogrammer::bookHistos(const Ph2_HwInterface::EventMap& evmap) {
  dut0HitProfH_ = new TH1I( "evenSensor_hitprofile", "Even Sensor Hitmap", 1016, 0.5, 1016.5 );
  dut1HitProfH_ = new TH1I( "oddSensor_hitprofile", "Odd Sensor Hitmap", 1016, 0.5, 1016.5 );

  sensCorrH_ = new TH1I( "sensorHitcorr", "Sensor Hit Correlation", 4, 0.5, 4.5 );
  sensCorrH_->GetXaxis()->SetBinLabel(1, "No hits");
  sensCorrH_->GetXaxis()->SetBinLabel(2, "Even & !Odd");
  sensCorrH_->GetXaxis()->SetBinLabel(3, "Odd & !Even");
  sensCorrH_->GetXaxis()->SetBinLabel(4, "Even & Odd");
  
  l1AcceptH_= new TH1I( "l1A", "L1A Counter", 10000, 0, 10000 );
  tdcCounterH_= new TH1I( "tdc", "TDC counter", 256, -0.5, 255.5 );

  for ( auto const& it : evmap ) {
    uint32_t feId = it.first;
    for ( auto const& jt : it.second ) {
      uint32_t cbcId = jt.first;
      // create histogram name tags
      std::stringstream ss;
      ss << "fed" << feId << "cbc" << cbcId;
      std::string key = ss.str();
      CBCHistos cbc_h;
      cbc_h.errBitH = new TH1I( TString( "errbit"+key ), TString("Error bit ("+key+")"), 3, -0.5, 2.5 );
      cbc_h.nStubsH = new TH1I( TString( "nstubs"+key ), TString("No. Of Stubs ("+key+")"), 21, -0.5, 20.5 );
      cbc_h.evenChnOccuH = new TH1I( TString( "chOccupancy_even"+key ), TString("Even Channel Occupancy ("+key+")"), 127, 0.5, 127.5 );
      cbc_h.oddChnOccuH = new TH1I( TString( "chOccupancy_odd"+key ), TString("Odd Channel Occupancy ("+key+")"), 127, 0.5, 127.5 );

      cbcHMap_.insert({key, cbc_h});
    }
  }
  // optionally add a Tree 
  if (addTree_) {
    tree_ = new TTree("sensorHitTree", "sensorHitTree from RAW files");

    sensorNhitsEven_ = new std::vector<int>();
    tree_->Branch("dutEven_channel", "std::vector<int>", &sensorNhitsEven_);

    sensorNhitsOdd_ = new std::vector<int>();
    tree_->Branch("dutOdd_channel", "std::vector<int>", &sensorNhitsOdd_);
  }
}  
void DQMHistogrammer::fillHistos(const std::vector<Event*>& event_list) {
  for ( const auto& ev : event_list ) {
    if (addTree_) {
      sensorNhitsEven_->clear();
      sensorNhitsOdd_->clear();
    }
    tdcCounterH_->Fill( ev->GetTDC() );
    l1AcceptH_->Fill( ev->GetEventCount() );

    const EventMap& evmap = ev->GetEventMap();
    for ( auto const& it : evmap ) {
      int even_hits = 0;	
      int odd_hits = 0;	
      
      uint32_t feId = it.first;
      for ( auto const& jt : it.second ) {
	uint32_t cbcId = jt.first;
	std::stringstream ss;
	ss << "fed" << feId << "cbc" << cbcId;
	std::string key = ss.str();
	
	std::map<std::string, CBCHistos>::iterator iCBCH = cbcHMap_.find(key);
	if (iCBCH != cbcHMap_.end()) {
	  CBCHistos& cbc_h = iCBCH->second;
	  if (cbc_h.errBitH) cbc_h.errBitH->Fill( ev->Error( feId, cbcId ));
	  if (cbc_h.nStubsH) cbc_h.nStubsH->Fill( std::stoi( ev->StubBitString( feId, cbcId ), nullptr, 10));

	  // channel data
	  const std::vector<bool>& dataVec = ev->DataBitVector( feId, cbcId );
	  for ( unsigned int ch = 0; ch < dataVec.size(); ++ch ) {
	    if ( dataVec[ch] ) {
	      int ichan = ch / 2 + 1;					
	      int hitpos = 127*cbcId + ichan;
	      if ( ch % 2 == 0 ) {
		if (cbc_h.evenChnOccuH) cbc_h.evenChnOccuH->Fill( ichan );
		if (dut0HitProfH_) dut0HitProfH_->Fill( hitpos );
                if (addTree_) sensorNhitsEven_->push_back(hitpos);
		++even_hits;
	      } else {
		if (cbc_h.oddChnOccuH) cbc_h.oddChnOccuH->Fill( ichan );
		if (dut1HitProfH_) dut1HitProfH_->Fill( hitpos);
                if (addTree_) sensorNhitsOdd_->push_back(hitpos);
		++odd_hits;
	      }
	    }
	  }
	}
      }
      if (!even_hits && !odd_hits) sensCorrH_->Fill(1);
      else if (!even_hits && odd_hits) sensCorrH_->Fill(2);
      else if (even_hits && !odd_hits) sensCorrH_->Fill(3);
      else if (even_hits && odd_hits) sensCorrH_->Fill(4);  
    }
    if (addTree_) tree_->Fill();
  }
}
void DQMHistogrammer::saveHistos(const std::string& out_file){
  TFile* fout = TFile::Open( out_file.c_str(), "RECREATE" );
  for ( auto& imap : cbcHMap_ ) {
    CBCHistos& cbc_h = imap.second;
    fout->mkdir( imap.first.c_str() );
    fout->cd( imap.first.c_str() );
    cbc_h.errBitH->Write();
    cbc_h.nStubsH->Write();
    cbc_h.evenChnOccuH->Write();
    cbc_h.oddChnOccuH->Write();
  }
  
  fout->cd();
  if (l1AcceptH_) l1AcceptH_->Write();
  if (tdcCounterH_) tdcCounterH_->Write();
  fout->mkdir("Sensor");
  fout->cd("Sensor");
  if (dut0HitProfH_) dut0HitProfH_->Write();
  if (dut1HitProfH_) dut1HitProfH_->Write();
  if (sensCorrH_) sensCorrH_->Write();

  fout->Close();

  // Optionally save the tree
  if (addTree_) {
    TFile* fout2 = TFile::Open( "Rawtuple_" + TString(out_file.c_str()), "RECREATE" );
    fout2->cd();
    tree_->Write();
    fout2->Close();
  }
}
void DQMHistogrammer::resetHistos() {
  if (dut0HitProfH_) dut0HitProfH_->Reset();
  if (dut1HitProfH_) dut1HitProfH_->Reset();
  if (sensCorrH_) sensCorrH_->Reset();
  if (l1AcceptH_) l1AcceptH_->Reset();
  if (tdcCounterH_) tdcCounterH_->Reset();

  for ( auto& imap : cbcHMap_ ) {
    CBCHistos& cbc_h = imap.second;
    cbc_h.errBitH->Reset();
    cbc_h.nStubsH->Reset();
    cbc_h.evenChnOccuH->Reset();
    cbc_h.oddChnOccuH->Reset();
  }
  // Optionally reset the tree
  if (addTree_) tree_->Reset();
}
