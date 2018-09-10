#include <TH1F.h>
#include <TFile.h>
#include <TTree.h>
#include "TGraphErrors.h"
#include <TBranch.h>
#include <TCanvas.h>
#include <TSpectrum.h>
#include <TF1.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

void alignment_new(int run)
{
	// Getting the file
	
	string path = "/afs/cern.ch/work/g/gmocelli/PhD/res_"+to_string(run);
	string filename = path+"/out.root";
	const char *file = filename.c_str();
	TFile *infile = new TFile(file);
	
	// Getting the TTree
	
	TTree *datatree = (TTree*)infile->Get("AlignmentValidationQC8/tree");
	
	// Getting the number of events
	
	TLeaf *evt_num = datatree->GetLeaf("ev");
	long int evt_tot_number = evt_num->GetBranch()->GetEntries();
	
	cout << "Number of events = " << evt_tot_number << endl;	
	
	// Getting the recHit and trajHit leaves
	
	TLeaf *recHx = datatree->GetLeaf("chRecHitX");
	TLeaf *recHy = datatree->GetLeaf("chRecHitY");
	TLeaf *trajHx = datatree->GetLeaf("chTrajHitX");
	TLeaf *trajHy = datatree->GetLeaf("chTrajHitY");
	TLeaf *trackPx = datatree->GetLeaf("trajPx");
	TLeaf *trackPy = datatree->GetLeaf("trajPy");
	TLeaf *trackPz = datatree->GetLeaf("trajPz");
	
	// Useful variable declaration
	
	double recHitX, recHitY;
	double trajHitX, trajHitY;
	double trajAngleX, trajAngleY;
	
	const double Zpos[10] = {22.66,26.13,45.66,49.13,68.66,72.13,91.66,95.13,114.66,118.13}; // z position of recHits
	double YposLong[8] = {72.747497,53.292499,35.486499,19.329500,4.4980001,-9.008000,-21.43950,-32.79650};
	double YposShort[8] = {0,0,0,0,0,0,0,0}; // modify this when you know where they are...
	
	bool sameCol = false;
	int hitsOK = 0;
	int hitsTOT = 0;
	int evtCol;
  	
  	// Histogram declaration
  	
  	char *histname = new char[20];
  	TH1F *resXperSC[15][8];
    for (int i_SC=0; i_SC<15; i_SC++)
    {
    	for (int i_eta=0; i_eta<8; i_eta++)
    	{
        	sprintf(histname,"resX_SC_%u/%u_eta_%u",(i_SC%5)+1,(i_SC/5)+1,i_eta+1);
        	resXperSC[i_SC][i_eta] = new TH1F(histname,"",400,-4,4);
        }
    }
    
    // Column inclination histograms
    
    TH1F *TrackAngleX_perCol[3];
    for (int i_col=0; i_col<3; i_col++)
    {
        sprintf(histname,"TrackAngleX_Column_%u",i_col+1);
        TrackAngleX_perCol[i_col] = new TH1F(histname,"",200,-2,2);
    }
    
    // Residuals per SC
	
	cout << "Starting the analysis of run " << run << endl;
	
	//for (int evt=0; evt<evt_tot_number; evt++)
	for (int evt=0; evt<500000; evt++)
	{
		if (evt%50000 == 0) cout << "Event " << evt << endl;
		//cout << "Event " << evt << endl;
		
		recHx->GetBranch()->GetEntry(evt);
		recHy->GetBranch()->GetEntry(evt);
		trajHx->GetBranch()->GetEntry(evt);
		trajHy->GetBranch()->GetEntry(evt);
		
		trackPx->GetBranch()->GetEntry(evt);
		trackPy->GetBranch()->GetEntry(evt);
		trackPz->GetBranch()->GetEntry(evt);
		
		trajAngleX = double(trackPx->GetValue())/double(trackPz->GetValue());
		trajAngleY = double(trackPy->GetValue())/double(trackPz->GetValue());
		
		hitsOK = 0; hitsTOT = 0;
		
		for (int ch_num=0; ch_num<30; ch_num++)
	 	{
	 		if (trajHx->GetValue(ch_num) != 0)
	 		{
	 			evtCol = int(ch_num/10);
	 			
	 			for (int ch_num_2=0; ch_num_2<30; ch_num_2++)
	 			{
	 				if (trajHx->GetValue(ch_num_2) != 0 && int(ch_num/10)==int(ch_num_2/10)) hitsOK++;
	 				if (trajHx->GetValue(ch_num_2) != 0) hitsTOT++;
	 			}
	 			break;
	 		}
	 	}
	 	
	 	if(hitsOK == hitsTOT) sameCol=true;
	 	if(hitsOK != hitsTOT) sameCol=false;
		
		if (sameCol == true)
		{			
			TrackAngleX_perCol[evtCol]->Fill(trajAngleX);
			
			for (int ch_num=0; ch_num<30; ch_num++)
	 		{
	 			recHitX = recHx->GetValue(ch_num);
	 			recHitY = recHy->GetValue(ch_num);
	 			trajHitX = trajHx->GetValue(ch_num);
	 			trajHitY = trajHy->GetValue(ch_num);
	 			if (recHitY==0) continue;
	 			
	 			int ieta = 999;
	 			
	 			for (int i=0; i<8; i++)
				{
					if (fabs(recHitY-YposLong[i]) < 0.1)
					{
						ieta = i;
						break;
					}
				}
	 			
	 			if (fabs(trajAngleY)<0.05 && recHitY!=0 && ieta!=999 && fabs(recHitX-trajHitX)<4.0 && fabs(recHitY-trajHitY)<20.0)
	 			{
	 				resXperSC[int(ch_num/2)][ieta]->Fill(recHitX-trajHitX);
	 			}
			}
		}
	}
	
	double resEtaY[8];
	double resEtaYError[8];
	
	char *cnvname = new char[20];
	TCanvas *cnvResX[15];
	TCanvas *cnvResCorrPlot[15];
	
	double dx[15];
	double rz[15];
	
	double columnIncl[3];
	
	for (int i_SC=0; i_SC<15; i_SC++)
    {
    	sprintf(cnvname,"cnv_SC_%u/%u",(i_SC%5)+1,(i_SC/5)+1);
    	cnvResX[i_SC] = new TCanvas(cnvname,cnvname,0,0,1000,600);
    	cnvResX[i_SC]->Divide(4,2);
    	
    	for (int i_eta=0; i_eta<8; i_eta++)
    	{
        	cnvResX[i_SC]->cd(i_eta+1);
        	resXperSC[i_SC][i_eta]->Draw();
        	TF1 *GaussFit = new TF1("GaussFit","gaus",-4,4);
        	resXperSC[i_SC][i_eta]->Fit(GaussFit,"Q");
        	GaussFit->Draw("SAME");
        	resEtaY[i_eta] = GaussFit->GetParameter(1);
        	resEtaYError[i_eta] = GaussFit->GetParError(1);
        	delete GaussFit;
    	}
    	
    	sprintf(cnvname,"cnv_resCorr_SC_%u/%u",(i_SC%5)+1,(i_SC/5)+1);
    	cnvResCorrPlot[i_SC] = new TCanvas(cnvname,cnvname,0,0,1000,600);
    	sprintf(histname,"resCorrPlot_SC_%u/%u",(i_SC%5)+1,(i_SC/5)+1);
        TGraphErrors *resCorrPlotSC = new TGraphErrors(8,YposLong,resEtaY,0,resEtaYError);
        resCorrPlotSC->SetTitle(histname);
        resCorrPlotSC->SetMarkerSize(1.5);
   		resCorrPlotSC->SetMarkerStyle(21);
    	resCorrPlotSC->Draw("ap");
    	TF1 *LinFit = new TF1("LinFit","pol1",YposLong[7]-2,YposLong[0]+2);
    	resCorrPlotSC->Fit(LinFit,"Q");
    	dx[i_SC] = LinFit->GetParameter(0);
    	rz[i_SC] = LinFit->GetParameter(1);
    	delete LinFit;
    }
    
    TCanvas *cnvTrajAng = new TCanvas("TrackAngleX","TrackAngleX",0,0,1000,600);
    cnvTrajAng->Divide(3);
    for (int i_col=0; i_col<3; i_col++)
    {
    	cnvTrajAng->cd(i_col+1);
    	TrackAngleX_perCol[i_col]->Draw();
    	TF1 *GaussFit = new TF1("GaussFit","gaus",-4,4);
        TrackAngleX_perCol[i_col]->Fit(GaussFit,"Q");
       	GaussFit->Draw("SAME");
       	columnIncl[i_col] = GaussFit->GetParameter(1);
       	delete GaussFit;
    }
    
    cout << "shiftX = [";
    for (int i_SC=0; i_SC<15; i_SC++)
    {
    	cout << dx[i_SC] << ",";
    	if (i_SC == 14) cout << dx[i_SC];
    	if (i_SC == 4 || i_SC == 9) cout << "\\" << endl;
    	if (i_SC == 14) cout << "]\n" << endl;
    }
    
    cout << "rotationZ = [";
    for (int i_SC=0; i_SC<15; i_SC++)
    {
    	cout << rz[i_SC]*180/3.14159 << ",";
    	if (i_SC == 14) cout << rz[i_SC]*180/3.14159;
    	if (i_SC == 4 || i_SC == 9) cout << "\\" << endl;
    	if (i_SC == 14) cout << "]\n" << endl;
    }
    
    for (int i_col=0; i_col<3; i_col++)
    {
    	cout << "Column " << i_col << " inclination = " << columnIncl[i_col]*180/3.14159 << " deg" << endl;
   	}
}
