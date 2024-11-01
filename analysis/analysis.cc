#include "RootInterface.h"
#include "DRsimInterface.h"
#include "functions.h"

#include "TROOT.h"
#include "TStyle.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TPaveStats.h"
#include "TString.h"
#include "TLorentzVector.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH2Poly.h"

#include <iostream>
#include <string>
#include "Riostream.h"

int main(int argc, char* argv[]) {
  TString filename = argv[1];
  float low = std::stof(argv[2]);
  float high = std::stof(argv[3]);

  gStyle->SetOptFit(1);

  RootInterface<DRsimInterface::DRsimEventData>* drInterface = new RootInterface<DRsimInterface::DRsimEventData>(std::string(filename)+".root", true);
  drInterface->set("DRsim","DRsimEventData");
  //drInterface->GetChain("DRsim");

  TH1F* tEdep = new TH1F("totEdep",";MeV;Evt",100,low*1000.,high*1000.);
  tEdep->Sumw2(); tEdep->SetLineColor(kRed); tEdep->SetLineWidth(2);

  TH1F* tEdep_MCP    = new TH1F("totEdep_MCP",";MeV;Evt",1000,0,high*1000.);
  TH2F* tEdep_MCP_2D = new TH2F("Edep_MCP_2D",";;",      8, 0, 8, 8, 0, 8);
  TH2F* tEdep_MCP_2D_percent = new TH2F("Edep_MCP_2D_percent",";;",      8, 0, 8, 8, 0, 8);

  TH1F* tEdep_Towers[529];
  TString histoIDs;
  for (int i=0; i<529; i++) {
    histoIDs = std::to_string(i) + "_Tower_Edep";
    tEdep_Towers[i] = new TH1F(histoIDs, ";MeV;Evt", 1000, 0., high*1000.);
  }

  TH1F* tP_leak = new TH1F("Pleak",";MeV;Evt",100,0.,1000.*high);
  tP_leak->Sumw2(); tP_leak->SetLineWidth(2);
  TH1F* tP_leak_nu = new TH1F("Pleak_nu",";MeV;Evt",100,0.,1000.*high);
  tP_leak_nu->Sumw2(); tP_leak_nu->SetLineWidth(2);

  unsigned int entries = drInterface->entries();
  while (drInterface->numEvt() < entries) {
    if (drInterface->numEvt() % 500 == 0) printf("Analyzing %dth event ...\n", drInterface->numEvt());

    DRsimInterface::DRsimEventData drEvt;
    drInterface->read(drEvt);

    float Edep = 0.; float Edep_MCP = 0.;
    float Edep_Towers[529] = {0};
    int moduleNum = 0;

    for (auto edepItr = drEvt.Edeps.begin(); edepItr != drEvt.Edeps.end(); ++edepItr) {
      auto edep = *edepItr;
      Edep += edep.Edep;

      moduleNum = edep.ModuleNum;
      Edep_Towers[moduleNum] += edep.Edep;
    }

    tEdep->Fill(Edep);

    for (int i=0; i<529; i++) {
      tEdep_Towers[i]->Fill(Edep_Towers[i]);
    }

    for (int i=8; i<16; i++) {
      for (int j=8; j<16; j++) {
        Edep_MCP += Edep_Towers[i*23+j];
        tEdep_MCP_2D->Fill((j-8)+0.5, (i-8)+0.5, Edep_Towers[i*23+j] / entries);
        tEdep_MCP_2D_percent->Fill((j-8)+0.5, (i-8)+0.5, Edep_Towers[i*23+j] / (entries*1000));
      }
    }

    tEdep_MCP->Fill(Edep_MCP);

    float Pleak = 0.;
    float Eleak_nu = 0.;
    for (auto leak : drEvt.leaks) {
      TLorentzVector leak4vec;
      leak4vec.SetPxPyPzE(leak.px,leak.py,leak.pz,leak.E);
      if ( std::abs(leak.pdgId)==12 || std::abs(leak.pdgId)==14 || std::abs(leak.pdgId)==16 ) {
        Eleak_nu += leak4vec.P();
      } else {
        Pleak += leak4vec.P();
      }
    }
    tP_leak->Fill(Pleak);
    tP_leak_nu->Fill(Eleak_nu);
  } // event loop

  TCanvas* c = new TCanvas("c","");

  c->cd();

  tEdep->Draw("Hist"); c->SaveAs(filename+"_Total_Edep.pdf");
  tEdep_MCP->Draw("Hist"); c->SaveAs(filename+"_MCP_Edep.pdf");

  std::ofstream ofstream1;

  ofstream1.open(filename+"_Edep_MCP.csv", std::ios::out | std::ios::app);
  ofstream1 << "Total Edep  "   << tEdep->GetMean() << std::endl;
  ofstream1 << "MCP   Edep  "   << tEdep_MCP->GetMean() << std::endl;
  for (int i=8; i<16; i++) {
    for (int j=8; j<16; j++) {
      ofstream1 << "Cell "<< i-7 << "x" << j-7 << "   "<< tEdep_Towers[i*23+j]->GetMean() << std::endl;
      //tEdep_Towers[i*23+j]->Draw("Hist");
      //c->SaveAs(filename+"_Edep_Cell_"+std::to_string((i-8)*8 + (j-7))+".pdf");
    }
  }

  c->SetLogy(1);
  tP_leak->Draw("Hist"); c->SaveAs(filename+"_Pleak.pdf");
  tP_leak_nu->Draw("Hist"); c->SaveAs(filename+"_Pleak_nu.pdf");
  c->SetLogy(0);

  gStyle->SetPaintTextFormat("4.1f");
  c->cd();
  c->SetCanvasSize(1200, 1000);
  c->SetRightMargin(0.2);
  c->SetLeftMargin(0.15);
  tEdep_MCP_2D->SetMarkerSize(0.9);

  tEdep_MCP_2D->Draw("COL0Z text");
  tEdep_MCP_2D->SetStats(0);
  c->SaveAs(filename+"_Edep2D_wtext.pdf");

  c->SetLogz(1);
  tEdep_MCP_2D->Draw("COL0Z TEXT"); 
  tEdep_MCP_2D->SetStats(0);
  c->SaveAs(filename+"_Edep2D_Log.pdf"); 
  c->SetLogz(0);

  gStyle->SetPaintTextFormat("2.1f %%");
  tEdep_MCP_2D_percent->SetMarkerSize(0.9);

  tEdep_MCP_2D_percent->Draw("COL0Z text");
  tEdep_MCP_2D_percent->SetStats(0);
  c->SaveAs(filename+"_Edep2D_percent_wtext.pdf");

  c->SetLogz(1);
  tEdep_MCP_2D_percent->Draw("COL0Z TEXT"); 
  tEdep_MCP_2D_percent->SetStats(0);
  c->SaveAs(filename+"_Edep2D_percent_Log.pdf"); 
  c->SetLogz(0);
}
