#include <iostream>
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "../include/TBEvent.h"
#include "../include/shashlik.h"

const int MAXADC=4095;

//void drawChannelMap(TH2I*& hModU, TH2I*& hModD, TH2I*& hChanU, TH2I*& hChanD){
void drawChannelMap(TCanvas*& can) {

  Mapper *mapper=Mapper::Instance();
  TH2I *hModU=new TH2I("hModU_label","Modules UpSteam",4,0.5,4.5,4,0.5,4.5);
  TH2I *hModD=new TH2I("hModD_label","Modules DownStream",4,0.5,4.5,4,0.5,4.5);
  TH2I *hChanU=new TH2I("hChanU_label","Channels UpStream",8,0.5,4.5,8,0.5,4.5);
  TH2I *hChanD=new TH2I("hChanD_label","Channels DownStream",8,0.5,4.5,8,0.5,4.5);

  for (int i=1; i<=NMODULES; i++){
    int x,y;
    mapper->ModuleXY(i,x,y);
    hModU->Fill(x,y,-i);
    hModD->Fill(x,y,i);
  }
  can->cd(1);
  hModD->SetMarkerSize(3);
  hModD->Draw("text same");
  can->cd(2);
  hModU->SetMarkerSize(3);
  hModU->Draw("text same");

  for (int i=0; i<NPADECHANNELS/2; i++){
    // downstream channels
    int channelID=FIBERMAP[i*4];
    int fiberID=FIBERMAP[i*4+1];
    float x,y;
    // cout << i<<" "<< channelID << " " << fiberID << " ";
    mapper->FiberXY(fiberID, x, y);
    // cout << x << " " << y << endl;
    hChanD->Fill(x,y,channelID);
    // upstream channels
    channelID=FIBERMAP[i*4+2];
    fiberID=FIBERMAP[i*4+3];

    hChanU->Fill(x,y,channelID);
  }
 

  can->cd(3);
  hChanD->SetMarkerSize(1.5);
  hChanD->Draw("text same");
  can->cd(4);
  hChanU->SetMarkerSize(1.5);
  hChanU->Draw("text same");
}

// inputs data file and event in file to display (default is to integrate all)
void calDisplay(TString fdat, int ndisplay=0){

  gStyle->SetOptStat(0);

  TFile *f = new TFile(fdat);
  if (f->IsZombie()){
    cout << "Cannot open file: " << fdat << endl;
    return;
  }

  Bool_t singleEvent=ndisplay>0;
  Mapper *mapper=Mapper::Instance();
  TH2F *hModU=new TH2F("hModU","Modules UpSteam RO",4,0.5,4.5,4,0.5,4.5);
  TH2F *hModD=new TH2F("hModD","Modules DownStream RO",4,0.5,4.5,4,0.5,4.5);
  TH2F *hChanU=new TH2F("hChanU","Channels UpStream RO",8,0.5,4.5,8,0.5,4.5);
  TH2F *hChanD=new TH2F("hChanD","Channels DownStream RO",8,0.5,4.5,8,0.5,4.5);

  hModU->SetMinimum(0);
  hModD->SetMinimum(0);
  hChanU->SetMinimum(0);
  hChanU->SetMinimum(0);

  if (singleEvent){
    hModU->SetMaximum(MAXADC*4);
    hModD->SetMaximum(MAXADC*4);
    hChanU->SetMaximum(MAXADC);
    hChanU->SetMaximum(MAXADC);
  }
  
  TH2F * hModD_time = (TH2F*)hModU->Clone("hModD_time");
  TH2F * hModU_time = (TH2F*)hModU->Clone("hModU_time");
  TH2F * hChanD_time = (TH2F*)hModU->Clone("hChanD_time");
  TH2F * hChanU_time = (TH2F*)hModU->Clone("hChanU_time");

  TBEvent *event = new TBEvent();
  TTree *t1041 = (TTree*)f->Get("t1041");
  TBranch *bevent = t1041->GetBranch("tbevent");
  bevent->SetAddress(&event);

  Int_t start=0; Int_t end=t1041->GetEntries();
  if (singleEvent) {
    start=ndisplay-1;
    end=ndisplay;
  }
  for (Int_t i=start; i<end; i++) {
    t1041->GetEntry(i);
    for (Int_t j = 0; j < event->NPadeChan(); j++){
      PadeChannel pch = event->GetPadeChan(j);

      // loop over ADC samples
      UShort_t * wform = pch.GetWform();
      UShort_t max = 0;
      Int_t maxTime = 0;

      // find the value to plot (just using the peak sample value now)
      bool overMax = false;
      for (Int_t k = 0; k < event->GetPadeChan(j).__SAMPLES(); k++){
	if(wform[k] > MAXADC) {
	  overMax = true;
	  break;
	}
	if (wform[k] > 120 && wform[k] > max) {
	  max = wform[k];
	  maxTime = k;
	}
      }

      if(overMax) continue;
      
      ///////////////////////////////////////////////////////////////
      int module,fiber;
      mapper->Pade2Fiber(pch.GetBoardID(), pch.GetChannelID(), module, fiber);
      int xm,ym;
      mapper->ModuleXY(module,xm,ym);
      if (module<0) {
	hModU->Fill(xm, ym, max);
	hModU_time->Fill(xm, ym, maxTime);
      }
      else {
	hModD->Fill(xm, ym, max);
	hModD_time->Fill(xm, ym, maxTime);
      }
      
      float xf,yf;
      int fiberID=module*100+fiber;
      mapper->FiberXY(fiberID, xf, yf);
      if (module<0) {
	hChanU->Fill(xf, yf, max);
	hChanU_time->Fill(xf, yf, maxTime);
      }
      else {
	hChanD->Fill(xf, yf, max);
	hChanD_time->Fill(xf, yf, maxTime);
      }

    }
  }

  hModD->Scale(1./hModD->GetEntries());
  hModU->Scale(1./hModU->GetEntries());
  hChanD->Scale(1./hChanD->GetEntries());
  hChanU->Scale(1./hChanU->GetEntries());
  
  hModD_time->Scale(1./hModD_time->GetEntries());
  hModU_time->Scale(1./hModU_time->GetEntries());
  hChanD_time->Scale(1./hChanD_time->GetEntries());
  hChanU_time->Scale(1./hChanU_time->GetEntries());


  TCanvas *c1=new TCanvas("c1","Average Peak Height",800,800);
  c1->Divide(2,2);

  c1->cd(1);
  hModD->Draw("colz");
  c1->cd(2);
  hModU->Draw("colz");
  c1->cd(3)->SetGrid();
  hChanD->Draw("colz");
  c1->cd(4)->SetGrid();
  hChanU->Draw("colz");
  drawChannelMap(c1);

  return;   // skip timing

  TCanvas * c2 = new TCanvas("c2", "Average Peak Timing", 800, 800);
  c2->Divide(2, 2);

  c2->cd(1);
  hModD_time->Draw("colz");
  c2->cd(2);
  hModU_time->Draw("colz");
  c2->cd(3)->SetGrid();
  hChanD_time->Draw("colz");
  c2->cd(4)->SetGrid();
  hChanU_time->Draw("colz");
  drawChannelMap(c2);
  c2->Update();
}