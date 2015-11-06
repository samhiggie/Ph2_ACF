#include <Riostream.h>

#include <TROOT.h>
#include <TDirectory.h>
#include <TDirectoryFile.h>
#include <TFile.h>
#include <TKey.h>
#include <TH1F.h>
#include <TF1.h>
#include <TH2F.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TString.h>
#include <TPaveText.h>
#include <TText.h>
#include <TLegend.h>
#include <TMath.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>


void plot_all(TString fname, Int_t iBoard, Int_t iFnd, Int_t iCBC);
void plot_SCurve(TFile* file, Int_t iBoard, Int_t iFnd, Int_t iCBC, TString type);
void plot_Vplus(TFile* file, Int_t iFnd, Int_t iCBC);
void plot_validation(TFile* file, Int_t iFnd, Int_t iCBC);
/*void plot_pedestal(TFile* file, Int_t iBoard, Int_t iFnd, Int_t iCBC);*/

void draw_SCurve(TDirectoryFile* dir, Int_t iBoard, Int_t iFnd, Int_t iCBC, TString xtit, Int_t ival);
void set_style(TH1F* th, TF1* f1, int mcol, int lcol);
TString createName(Int_t iBoard, Int_t iFnd, Int_t iCBC, Int_t iChan, TString type);

//
// -- Plot all relevant histograms and fuctions for a given CBC chip 
//
void plot_all(TString fname, Int_t iBoard, Int_t iFnd, Int_t iCBC) {
  TFile* file = new TFile(fname);
  if (!file || !file->IsOpen()) return;

  plot_SCurve(file, iBoard, iFnd, iCBC, "Vplus");
  plot_Vplus(file, iFnd, iCBC);
  plot_SCurve(file, iBoard, iFnd, iCBC, "Offset");
  /*plot_validation(file, iFnd, iCBC);*/
  /*plot_SCurve(file, iBoard, iFnd, iCBC, "Final");*/
}

//
// -- Plot S-curves for a given type (vplus value or offset value) for a given CBC chip
//
void plot_SCurve(TFile* file, Int_t iBoard, Int_t iFnd, Int_t iCBC, TString type) {

  if (!file || !file->IsOpen()) return;

  TString xtit;
  if (type.Contains("Vplus") || type.Contains("OffsetBits")) xtit = "VCth"; 
  else xtit = "Offset";

  TIter next(file->GetListOfKeys());
  TKey* key;

  TCanvas* tc = new TCanvas(type, type, 800, 600);
  tc->cd();

  TText txt;
  int ival = 0;
  float xpos;
  float ypos;

  while ( (key = dynamic_cast<TKey*>(next())) ) {
    TObject *obj = key->ReadObj();
    TString dname = obj->GetName();
    if (obj->InheritsFrom("TDirectory") && dname.Contains(type)) {  
      TDirectoryFile* td = dynamic_cast<TDirectoryFile*>(obj);
      draw_SCurve(td, iBoard, iFnd, iCBC, xtit, ival);

      xpos = 0.72;
      ypos = 0.7 - ival * 0.15;
      txt.SetTextColor(ival+2);
      txt.DrawTextNDC(xpos, ypos, dname);
      ival++; 
    }
  }
  tc->Update();
}
//
// -- Plot Vplus vs VCth histograms for a given CBC chip
//
void plot_Vplus(TFile* file, Int_t iFnd, Int_t iCBC) {  
  if (!file || !file->IsOpen()) return;
  TString name = "VplusVcthGraph_";
  name += "Fe";
  name += iFnd;
  name += "_Cbc";
  name += iCBC;
  TGraphErrors* tg =  dynamic_cast<TGraphErrors*> (file->Get(name));
  if (tg) {
    TCanvas* tc = new TCanvas("VPlusVsTCTH", "VPlusVsTCTH",700, 600);
    tc->cd();
    tg->Draw("AP");
    tg->GetXaxis()->SetTitle("VPlus");
    tg->GetYaxis()->SetTitle("VCth");
    tc->Update();
  }  
}
//
// -- Plot the validation histograms for a given CBC chip
//
void plot_validation(TFile* file, Int_t iFnd, Int_t iCBC) {  
  if (!file || !file->IsOpen()) return;
  TString name = "Validation_";
  name += "Fe";
  name += iFnd;
  name += "_Cbc";
  name += iCBC;
  TH1F* th =  dynamic_cast<TH1F*> (file->Get(name));
  if (th) {
    TCanvas* tc = new TCanvas("Validation", "Validation",700, 600);
    tc->cd();
    th->Draw();
    th->GetXaxis()->SetTitle("Occupancy");
    tc->Update();
  }  
}
//
// -- loop over all the Scurve histograms and the fitting functions for a given type and CBC chip and draw them
//
void draw_SCurve(TDirectoryFile* dir, Int_t iBoard, Int_t iFnd, Int_t iCBC, TString xtit, Int_t ival) {  
  TString option;
  Int_t marker_col = 1;
  Int_t line_col = ival+2;

  TString dName = dir->GetName();
  for (Int_t i = 0; i < 254; i++) {

    TString name = createName(iBoard, iFnd, iCBC, i, dName);
    TString hname = "Scurve_";
    TString fname = "Fit_";
    hname += name;
    fname += name;
    TH1F* hist = dynamic_cast<TH1F*> (dir->Get(hname));
    //TF1* func = dynamic_cast<TF1*> (dir->Get(fname));
    if ( ival == 0 && i == 0) {
      option = "P" ;
      hist->GetXaxis()->SetTitle(xtit);
      hist->GetYaxis()->SetTitle("Efficiency");
      TString htit = "SCurve (";
      htit += " Be ";
      htit += iBoard;
      htit += " Fe ";
      htit += iFnd;
      htit += " CBC ";
      htit += iCBC;
      htit += " )";
      hist->SetTitle(htit);
    } else option = "Psame";

    //set_style(hist, func, marker_col, line_col);
    hist->SetStats(0);   
    hist->DrawCopy(option);
    //func->DrawCopy("same");
  } 
}
//
// -- Set the style pf SCurve histogram and fitting function
//
void set_style(TH1F* th, TF1* f1, int mcol, int lcol) {
  if (!th || !f1) return;
  th->SetStats(0);
  th->SetMarkerStyle(20);
  th->SetMarkerSize(0.3);
  th->SetLineWidth(1);
  th->SetMarkerColor(mcol);
  th->SetLineColor(lcol);
  f1->SetLineStyle(1);
  f1->SetLineWidth(0.1);
  f1->SetLineColor(lcol);
}
//
// -- create the histogram/fuction name from Board/Front-End/CBC/Channel numbers 
//
TString createName(Int_t iBoard, Int_t iFnd, Int_t iCBC, Int_t iChan, TString type){
  TString name = "Be";
  name +=  iBoard;
  name += "_Fe";
  name +=  iFnd;
  name += "_Cbc";
  name += iCBC;
  name += "_Channel";
  name += iChan;
  name += type;
  return name;
} 
