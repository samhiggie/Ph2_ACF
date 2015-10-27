/*!
        \file                DQMHistogrammer.h
        \brief               base class to create and fill monitoring histograms
        \author              Suchandra Dutta and Subir Sarkar
        \version             1.0
        \date                13/10/15
        Support :            mail to : Suchandra.Dutta@cern.ch, Subir.Sarkar@cern.ch

*/

#ifndef __DQMHISTOGRAMMER_H__
#define __DQMHISTOGRAMMER_H__

#include "../tools/Tool.h"

#include <vector>
#include <string>
#include <map>

class TH1I;
class TTree;
/*!
 * \class DQMHistogrammer
 * \brief Class to create and fill monitoring histograms
 */
class DQMHistogrammer : public Tool {

  public:
  /*! 
   * constructor 
   */
  DQMHistogrammer(bool addTree = false);
  
  /*! 
   * destructor 
   */
  virtual ~DQMHistogrammer();

  /*!
   * Book histograms
   */
  void bookHistos(const Ph2_HwInterface::EventMap& evmap);
  /*!
   * Fill histogram
   */
  void fillHistos(const std::vector<Event*>& event_list);
  void saveHistos(const std::string& out_file);
  void resetHistos();

 private:

  bool addTree_;

  TTree* tree_;
  std::vector<int>* sensorNhitsEven_;
  std::vector<int>* sensorNhitsOdd_;

  struct CBCHistos {
    TH1I* errBitH;
    TH1I* nStubsH;
    TH1I* evenChnOccuH;
    TH1I* oddChnOccuH;
  };
  std::map< std::string, CBCHistos > cbcHMap_;
  
  TH1I* dut0HitProfH_;
  TH1I* dut1HitProfH_;
  TH1I* sensCorrH_;
  TH1I* l1AcceptH_;
  TH1I* tdcCounterH_;
};
#endif
