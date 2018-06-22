{

TString run = "001";
int drawOn=0;
int printOn=0;
int calc_eff_vfat_On=1;
//If you want to see eff vs layer or eff vs chamber without eff vs vfat then set calc_eff_vfat_On=0.
int setMinimumOn=1;
int makeTreeOn=1;
int cal_eff_from_tree_On=0;

TString srcDir = "/afs/cern.ch/work/g/gmocelli/PhD/gemcrs/src/Validation/GEMCosmicMuonStand/test";
TString savePngDir = "/afs/cern.ch/work/g/gmocelli/PhD/gemcrs/src/Validation/GEMCosmicMuonStand/test";
TString filename = "out";
TFile *f = new TFile(srcDir+"/res_1/"+filename+".root");

const int maxNchamber = 15;
const int maxNlayer = 30;
const int maxNphi = 3;
const int maxNeta = 8;
const int maxNvfat = 720;

double maxVfatEff = -1;
double minVfatEff = 100;
int vfatI[maxNlayer][maxNphi][maxNeta];
int vfatF[maxNlayer][maxNphi][maxNeta];
double vfatEff[maxNlayer][maxNphi][maxNeta];
double vfatEffError[maxNlayer][maxNphi][maxNeta];
TH1D *hvfatEff[maxNlayer];
TH2D *hvfatEff2[maxNlayer];
TH2D *hvfatEff3[maxNlayer];


TFile *fEff;
TTree *tree;
int vfat=0, layer=0, phi=0, eta=0, flr=0;
float eff=0, error=0;
if(makeTreeOn)
{
	fEff = new TFile(srcDir+"/res_1/"+filename+"_eff.root","recreate");
	tree = new TTree("tree","Tree for efficiency of vfat");
	tree->Branch("floor",&flr,"floor/I");
	tree->Branch("layer",&layer,"layer/I");
	tree->Branch("phi",&phi,"phi/I");
	tree->Branch("eta",&eta,"eta/I");
	tree->Branch("vfat",&vfat,"vfat/I");
	tree->Branch("eff",&eff,"eff/F");
	tree->Branch("error",&error,"error/F");
	//tree->Branch("",&,"");
}

f->cd("gemcrValidation");

long maxVfatNumerator = -1;
long minVfatNumerator = 10000000;
long maxVfatDenominator = -1;
long minVfatDenominator = 10000000;
TH2D *hvfatNumerator[maxNlayer];
TH2D *hvfatDenominator[maxNlayer];


int layerI[maxNlayer];
int layerF[maxNlayer];
double layerEff[maxNlayer];
double layerEffError[maxNlayer];
TH1D *hlayerEff = new TH1D("hlayerEff","Efficiency VS Layer",maxNlayer,1,maxNlayer+1);
hlayerEff->GetXaxis()->SetTitle("layer num");
hlayerEff->SetMaximum(1.005);
if(setMinimumOn) hlayerEff->SetMinimum(0.9);
TH2D *hlayerEff2 = new TH2D("hlayerEff2","Efficiency VS Layer",3,1,4, 10,1,11);
TH2D *hlayerI2 = new TH2D("hlayerI2","nRecHit per layer (denominator of efficiency)",3,1,4, 10,1,11);
TH2D *hlayerF2 = new TH2D("hlayerF2","nRecHit per layer (numerator of efficiency)",3,1,4, 10,1,11);
TH1D *hlayerHitMul[maxNlayer];

int chamberI[maxNchamber];
int chamberF[maxNchamber];
double chamberEff[maxNchamber];
double chamberEffError[maxNchamber];
TH1D *hchamberEff = new TH1D("hchamberEff","Efficiency VS Chamber",maxNchamber,1,maxNchamber+1);
hchamberEff->GetXaxis()->SetTitle("chamber num");
hchamberEff->SetMaximum(1.005);
if(setMinimumOn) hchamberEff->SetMinimum(0.9);
TH2D *hchamberEff2 = new TH2D("hchamberEff2","Efficiency VS Chamber",3,1,4, 5,1,6);
TH2D *hchamberI2 = new TH2D("hchamberI2","nRecHit per chamber (denominator of efficiency)",3,1,4, 5,1,6);
TH2D *hchamberF2 = new TH2D("hchamberF2","nRecHit per chamber (numerator of efficiency)",3,1,4, 5,1,6);
TH1D *hchamberHitMul[maxNchamber];

TF1 *func = new TF1("func","0.97",0,100); //goal line
func->SetLineColor(3);

int nch=0;
TString cutChamberI = "0";
TString cutChamberF = "0";

TString cutChamberHitMul = "0";

TCanvas *c1;
TCanvas *c2;
if(drawOn) 
{
	c1 = new TCanvas("c1","c1",700,500);
	c2 = new TCanvas("c2","c2",700,500);
	c2->SetRightMargin(0.15);
}

TString hname="";
TString htitle="";
TString png="";
//int i=0;
//for(int i=18;i<20;i++)
for(int i=0;i<maxNlayer;i++)
{
	hname = Form("hvfat_%d",i);
	htitle = Form("Efficiency VS VFAT (Chamber%d Layer%d)",nch*2+1,i%2+1);
	hvfatEff[i] = new TH1D(hname,htitle,maxNphi*maxNeta,0,maxNphi*maxNeta);
	hvfatEff[i]->GetXaxis()->SetTitle("VFAT num");
	hvfatEff[i]->SetMaximum(1.005);
	if(setMinimumOn) hvfatEff[i]->SetMinimum(0.9);

	hname = Form("hvfatEff2_%d",i);
	htitle = Form("Efficiency VS VFAT (Chamber%d Layer%d)",nch*2+1,i%2+1);
	hvfatEff2[i] = new TH2D(hname,htitle,maxNphi,1,maxNphi+1,maxNeta,1,maxNeta+1);
	hvfatEff2[i]->GetXaxis()->SetTitle("ith-#phi");
	hvfatEff2[i]->GetYaxis()->SetTitle("ith-#eta");

	hname = Form("hvfatNumerator_%d",i);
	htitle = Form("nRecHit per vfat (Chamber%d Layer%d, numerator of eff.)",nch*2+1,i%2+1);
	hvfatNumerator[i] = new TH2D(hname,htitle,maxNphi,1,maxNphi+1,maxNeta,1,maxNeta+1);
	hvfatNumerator[i]->GetXaxis()->SetTitle("ith-#phi");
	hvfatNumerator[i]->GetYaxis()->SetTitle("ith-#eta");
	hname = Form("hvfatDenominator_%d",i);
	htitle = Form("nRecHit per vfat (Chamber%d Layer%d, denominator of eff.)",nch*2+1,i%2+1);
	hvfatDenominator[i] = new TH2D(hname,htitle,maxNphi,1,maxNphi+1,maxNeta,1,maxNeta+1);
	hvfatDenominator[i]->GetXaxis()->SetTitle("ith-#phi");
	hvfatDenominator[i]->GetYaxis()->SetTitle("ith-#eta");

	if(i%2==0) 
	{
		cutChamberI = "0";
		cutChamberF = "0";

		cutChamberHitMul = "0";
	}
	TString cutLayerI = "0";
	TString cutLayerF = "0";

	TString cutLayerHitMul = "0";

        int nvfat = 0;
	for(int j=0, j2=maxNphi;j<maxNphi;j++, j2--) if(calc_eff_vfat_On)
	{
		for(int k=0, k2=maxNeta;k<maxNeta;k++, k2--)
		{
			int n1 = k; //n1 = 0,1,2,3,4,5,6,7
			int n2 = 2-j+(int)(2-i/10)*3; // n2 = 8 7 6 5 4 3 2 1 0 for run106 and run>=132
			//int n2 = j+(int)(2-i/10)*3; // n2 = 6 7 8 3 4 5 0 1 2 for 107<=run<=131
			int n3 = i%10; // n3 = 0,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9
			//int vfatId = i*100 + j*10 + k;
			TString cutI = Form("vfatI[%d][%d][%d]==1",i,j,k);
			TString cutF = cutI+"&&"+Form("vfatF[%d][%d][%d]==1",i,j,k);
			if(cal_eff_from_tree_On)
			{
				vfatI[i][j][k] = tree->GetEntries(cutI);
				vfatF[i][j][k] = tree->GetEntries(cutF);
			}
			else
			{
				vfatI[i][j][k] = hvfatHit_denominator->GetBinContent(n1+1, n2+1, n3+1);
				vfatF[i][j][k] = hvfatHit_numerator->GetBinContent(n1+1, n2+1, n3+1);
			}
			if(vfatI[i][j][k]>0)
			{
				vfatEff[i][j][k] = double(vfatF[i][j][k])/vfatI[i][j][k];
				vfatEffError[i][j][k] = sqrt(vfatEff[i][j][k]*(1-vfatEff[i][j][k])/vfatI[i][j][k]);
				cout<<"i "<<i<<", j "<<j<<", k "<<k<<", vfatF "<<vfatF[i][j][k]<<", vfatI "<<vfatI[i][j][k]<<", eff "<<vfatEff[i][j][k]<<" +- "<<vfatEffError[i][j][k]<<endl;

				if(maxVfatEff < vfatEff[i][j][k]) maxVfatEff = vfatEff[i][j][k];
				if(minVfatEff > vfatEff[i][j][k]) minVfatEff = vfatEff[i][j][k];

				if(makeTreeOn)
				{
//					layer = i;
//					phi = j;
//					eta = k;
//					n2 = 2-j+(int)(2-i/10)*3; //for 107<=run<=131
					layer = n3;
					phi = n2;
					eta = n1;
					flr = layer%10;
					eff = vfatEff[i][j][k];
					error = vfatEffError[i][j][k];
					vfat++;
					tree->Fill();
					//cout<<"n3 "<<n3<<", n2 "<<n2<<", n1 "<<n1<<", vfatF "<<vfatF[i][j][k]<<", vfatI "<<vfatI[i][j][k]<<", eff "<<vfatEff[i][j][k]<<" +- "<<vfatEffError[i][j][k]<<endl;
				}
			}

			cutLayerI += "||"+cutI;
			cutLayerF += "||("+cutF+")";

			cutLayerHitMul += Form("+vfatF[%d][%d][%d]",i,j,k);

			hvfatEff[i]->SetBinContent(nvfat+1, vfatEff[i][j][k]);
			hvfatEff[i]->SetBinError(nvfat+1, vfatEffError[i][j][k]);
//			hvfatEff2[i]->SetBinContent(j2+1,k2+1, vfatEff[i][j][k]);
//			hvfatNumerator[i]->SetBinContent(j2+1,k2+1,vfatF[i][j][k]);
//			hvfatDenominator[i]->SetBinContent(j2+1,k2+1,vfatI[i][j][k]);
			hvfatEff2[i]->SetBinContent(j+1,k+1, vfatEff[i][j][k]);
			hvfatNumerator[i]->SetBinContent(j+1,k+1,vfatF[i][j][k]);
			hvfatDenominator[i]->SetBinContent(j+1,k+1,vfatI[i][j][k]);
//			hvfatNumerator[i]->SetBinContent(j2,k+1,vfatF[i][j][k]);
//			hvfatDenominator[i]->SetBinContent(j2,k+1,vfatI[i][j][k]);

			if(maxVfatNumerator < vfatF[i][j][k]) maxVfatNumerator = vfatF[i][j][k];			
			if(minVfatNumerator > vfatF[i][j][k]) minVfatNumerator = vfatF[i][j][k];			
			if(maxVfatDenominator < vfatI[i][j][k]) maxVfatDenominator = vfatI[i][j][k];			
			if(minVfatDenominator > vfatI[i][j][k]) minVfatDenominator = vfatI[i][j][k];			
			//cout<<"i "<<i<<", j "<<j<<", k "<<k<<", vfatF "<<vfatF[i][j][k]<<", vfatI "<<vfatI[i][j][k]<<", eff "<<vfatEff[i][j][k]<<" +- "<<vfatEffError[i][j][k]<<", maxVfatNumerator "<<maxVfatNumerator<<", minVfatNumerator "<<minVfatNumerator<<", maxVfatDenominator "<<maxVfatDenominator<<", minVfatDenominator "<<minVfatDenominator<<endl;

			nvfat++;
		}
	}
	if(drawOn && calc_eff_vfat_On) 
	{
		c1->cd();
		hvfatEff[i]->Draw();
		hvfatEff[i]->Fit("pol0");
//		func->Draw("same");
		png = Form(savePngDir+"/eff_VS_vfat_of_chamber%d_layer%d.png",nch*2+1,i%2+1);
		if(printOn) c1->Print(png);

		c2->cd();
		hvfatEff2[i]->Draw("colz text");
		png = Form(savePngDir+"/eff_VS_vfat_of_chamber%d_layer%d_2D.png",nch*2+1,i%2+1);
		if(printOn) c2->Print(png);

		hvfatNumerator[i]->Draw("colz text");
		png = Form(savePngDir+"/eff_VS_vfat_of_chamber%d_layer%d_Numerator.png",nch*2+1,i%2+1);
		if(printOn) c2->Print(png);
		hvfatDenominator[i]->Draw("colz text");
		png = Form(savePngDir+"/eff_VS_vfat_of_chamber%d_layer%d_Denominator.png",nch*2+1,i%2+1);
		if(printOn) c2->Print(png);
	}
	cutLayerI = Form("vfatI[%d]==1",i);
	cutLayerF = cutLayerI + "&&" + Form("vfatF[%d]==1",i);
//	cout<<"i "<<i<<", cutLayerF "<<cutLayerF<<", cutLayerI "<<cutLayerI<<endl;
	if(cal_eff_from_tree_On)
	{
		layerI[i] = tree->GetEntries(cutLayerI);
		layerF[i] = tree->GetEntries(cutLayerF);
	}
	if(layerI[i]>0)
	{
		c1->cd();
		layerEff[i] = double(layerF[i])/layerI[i];
		layerEffError[i] = sqrt(layerEff[i]*(1-layerEff[i])/layerI[i]);
		hlayerEff->SetBinContent(i+1, layerEff[i]);
		hlayerEff->SetBinError(i+1, layerEffError[i]);

		hlayerI2->SetBinContent(i/10+1, i%10+1, layerI[i]);
		hlayerF2->SetBinContent(i/10+1, i%10+1, layerF[i]);
		hlayerEff2->SetBinContent(i/10+1, i%10+1, layerEff[i]);

		cout<<"i "<<i<<", layerF "<<layerF[i]<<", layerI "<<layerI[i]<<", eff "<<layerEff[i]<<" +- "<<layerEffError[i]<<endl;
	}

	hname = Form("hlayerHitMul_%d",i);
	htitle = Form("hit multiplicity (Chamber%d Layer%d)",nch*2+1,i%2+1);
	hlayerHitMul[i] = new TH1D(hname,htitle,10,0,10);
	hlayerHitMul[i]->GetXaxis()->SetTitle("nRecHit");
	if(cal_eff_from_tree_On) tree->Project(hname, cutLayerHitMul);

	if(drawOn)
	{
		c1->cd();
		hlayerHitMul[i]->Draw();
		png = Form(savePngDir+"/hit_multiplicity_of_chamber%d_layer%d.png",nch*2+1,i%2+1);
		if(printOn) c1->Print(png);

		int maxHitMul = 0;
		for(int nbin=0;nbin<hlayerHitMul[i]->GetNbinsX();nbin++)
		{
			int yValue = hlayerHitMul[i]->GetBinContent(nbin+1);
			if(yValue>0) maxHitMul = nbin;
		}
		cout<<"i "<<i<<", max hit multiplicity "<<maxHitMul<<endl;
	}

	cutChamberI += "||"+cutLayerI;
	cutChamberF += "||("+cutLayerF+")";

	cutChamberHitMul += "+"+cutLayerHitMul;

	if(i%2==1)
	{
//		cout<<"nch "<<nch<<", cutChamberF "<<cutChamberF<<", cutChamberI "<<cutChamberI<<endl;
		if(cal_eff_from_tree_On)
		{
			chamberI[nch] = tree->GetEntries(cutChamberI);
			chamberF[nch] = tree->GetEntries(cutChamberF);
		}
		if(chamberI[nch]>0)
		{
			chamberEff[nch] = double(chamberF[nch])/chamberI[nch];
			chamberEffError[nch] = sqrt(chamberEff[nch]*(1-chamberEff[nch])/chamberI[nch]);
			hchamberEff->SetBinContent(nch+1, chamberEff[nch]);
			hchamberEff->SetBinError(nch+1, chamberEffError[nch]);

			hchamberI2->SetBinContent(nch/5+1, nch%5+1, chamberI[nch]);
			hchamberF2->SetBinContent(nch/5+1, nch%5+1, chamberF[nch]);
			hchamberEff2->SetBinContent(nch/5+1, nch%5+1, chamberEff[nch]);

			cout<<"nch "<<nch<<", chamberF "<<chamberF[nch]<<", chamberI "<<chamberI[nch]<<", eff "<<chamberEff[nch]<<" +- "<<chamberEffError[nch]<<endl;
		}

		hname = Form("hchamberHitMul_%d",i);
		htitle = Form("hit multiplicity (Chamber%d)",nch*2+1);
		hchamberHitMul[nch] = new TH1D(hname,htitle,10,0,10);
		hchamberHitMul[nch]->GetXaxis()->SetTitle("nRecHit");
		if(cal_eff_from_tree_On) tree->Project(hname, cutChamberHitMul);

		if(drawOn)
		{
			c1->cd();
			hchamberHitMul[nch]->Draw();
			png = Form(savePngDir+"/hit_multiplicity_of_chamber%d.png",nch*2+1);
			if(printOn) c1->Print(png);

			int maxHitMul = 0;
			for(int nbin=0;nbin<hchamberHitMul[nch]->GetNbinsX();nbin++)
			{
				int yValue = hchamberHitMul[nch]->GetBinContent(nbin+1);
				if(yValue>0) maxHitMul = nbin;
			}
			cout<<"nch "<<nch<<", max hit multiplicity "<<maxHitMul<<endl;
		}

		nch++;
	}
}

cout<<endl<<"maxVfatEff "<<maxVfatEff<<", minVfatEff "<<minVfatEff<<", maxVfatNumerator "<<maxVfatNumerator<<", minVfatNumerator "<<minVfatNumerator<<", maxVfatDenominator "<<maxVfatDenominator<<", minVfatDenominator "<<minVfatDenominator<<endl;

/*
for(int i=0;i<maxNlayer;i++) if(drawOn && calc_eff_vfat_On) 
{
	c2->cd();
	hvfatEff2[i]->SetMaximum(maxVfatEff + (maxVfatEff-minVfatEff)*0.05);
	hvfatEff2[i]->SetMinimum(minVfatEff - (maxVfatEff-minVfatEff)*0.05);
	hvfatEff2[i]->Draw("colz");
	png = Form(savePngDir+"/eff_VS_vfat_of_chamber%d_layer%d_fixRange_2D.png",(i/2)*2+1,i%2+1);
	if(printOn) c2->Print(png);

	hvfatNumerator[i]->SetMaximum(maxVfatNumerator + (maxVfatNumerator-minVfatNumerator)*0.05);
	hvfatNumerator[i]->SetMinimum(minVfatNumerator - (maxVfatNumerator-minVfatNumerator)*0.05);
	hvfatNumerator[i]->Draw("colz");
	png = Form(savePngDir+"/eff_VS_vfat_of_chamber%d_layer%d_fixRange_Numerator.png",(i/2)*2+1,i%2+1);
	if(printOn) c2->Print(png);
	hvfatDenominator[i]->SetMaximum(maxVfatDenominator + (maxVfatDenominator-minVfatDenominator)*0.05);
	hvfatDenominator[i]->SetMinimum(minVfatDenominator - (maxVfatDenominator-minVfatDenominator)*0.05);
	hvfatDenominator[i]->Draw("colz");
	png = Form(savePngDir+"/eff_VS_vfat_of_chamber%d_layer%d_fixRange_Denominator.png",(i/2)*2+1,i%2+1);
	if(printOn) c2->Print(png);
}
*/

if(drawOn) 
{
	c1->cd();
	hlayerEff->Draw();
//	hlayerEff->Fit("pol0");
	png =savePngDir+"/eff_VS_layer.png";
	if(printOn) c1->Print(png);
	hchamberEff->Draw();
//	hchamberEff->Fit("pol0");
	png = savePngDir+"/eff_VS_chamber.png";
	if(printOn) c1->Print(png);

	c2->cd();
	hlayerI2->Draw("colz text");
	png =savePngDir+"/eff_VS_layer_2D_denominator.png";
	if(printOn) c2->Print(png);
	hlayerF2->Draw("colz text");
	png =savePngDir+"/eff_VS_layer_2D_numerator.png";
	if(printOn) c2->Print(png);
	hlayerEff2->Draw("colz text");
	png =savePngDir+"/eff_VS_layer_2D.png";
	if(printOn) c2->Print(png);

	hchamberI2->Draw("colz text");
	png = savePngDir+"/eff_VS_chamber_2D_denominator.png";
	if(printOn) c2->Print(png);
	hchamberF2->Draw("colz text");
	png = savePngDir+"/eff_VS_chamber_2D_numerator.png";
	if(printOn) c2->Print(png);
	hchamberEff2->Draw("colz text");
	png = savePngDir+"/eff_VS_chamber_2D.png";
	if(printOn) c2->Print(png);
}

TCanvas *c3 = new TCanvas("c3","c3",700,500);
fEff->cd();
//tree->Draw("eff>>h1(40,0.9,1)");
int nBad = tree->GetEntries("eff<0.97");
cout<<"number of bad vfats with eff<0.97 : "<<nBad<<endl;
//tree->Draw("eff>>h1(40,0.96,0.99)");
tree->Draw("eff>>h1(40,0.80.1.05)");
htitle = Form("number of vfats VS efficiency (nBadVfats : %d)",nBad);
h1->SetTitle(htitle);
h1->GetXaxis()->SetTitle("efficiency");
h1->GetYaxis()->SetTitle("number of vfats");
h1->Draw();
png = savePngDir+"/run"+run+"_number_of_vfats_VS_efficiency02.png";
c3->Print(png);
//tree->Draw("eff>>h2(40,0.9,1)","phi==2||phi==6"); //for run106 and 107<=run<=131
//tree->Draw("eff>>h2(40,0.9,1)","phi==0||phi==8"); //for run>=132
//tree->Draw("eff>>h2(40,0.9,1)","(layer==0||layer==9||layer==10||layer==19||layer==20||layer==29)&&(eta==0||eta==7)"); //for run==133
//tree->Draw("eff>>h2(40,0.9,1)","(floor==0||floor==9)&&(eta==0||eta==7)"); //for run==133
//tree->Draw("eff>>h2(40,0.96,0.99)","(floor==0||floor==9)&&(eta==0||eta==7)"); //for run==133
//tree->Draw("eff>>h2(40,0.80,1.05)","(floor==0||floor==9)&&(eta==0||eta==7)"); //for run==133
tree->Draw("eff>>h2(40,0.80,1.05)","(eta==0||eta==7)"); //for run==133
h2->SetLineColor(2);
h1->Draw();
h2->Draw("same");
TLegend *leg = new TLegend(0.20,0.65,0.70,0.85);
leg->AddEntry(h1,"all vfats","l");
//leg->AddEntry(h2,"vfats at the edge of QC8","l");
//leg->AddEntry(h2,"overlapped vfats under center chamber","l");
//leg->AddEntry(h2,"vfats in i#eta=1 or 8 when floor=0 and 9","l");
leg->AddEntry(h2,"vfats in i#eta=1 or 8","l");
leg->Draw();
png = savePngDir+"/run"+run+"_number_of_vfats_VS_efficiency01.png";
c3->Print(png);
//png = savePngDir+"/eff_VS_chamber_2D.png";
//c3->Print(png);

TCanvas *c4 = new TCanvas("c4","c4",700,500);
c4->SetRightMargin(0.15);
//TCanvas *c5[2];
TCanvas *c5 = new TCanvas("c5","c5",1442,437*2);
c5->Divide(3,4,0,0);

TString TB[2] = {"B","T"};

TLine *line1 = new TLine(maxNphi*1+1,1,maxNphi*1+1,maxNeta+1);
TLine *line2 = new TLine(maxNphi*2+1,1,maxNphi*2+1,maxNeta+1);
line1->SetLineWidth(2);
line2->SetLineWidth(2);

for(int i=0;i<maxNlayer/3;i++)
{
//	if(i%5==0)
//	{
		//c5[i/5] = new TCanvas(Form("c5_%d",i/5),Form("c5_%d",i/5),2100,1000);
		//c5[i/5]->Divide(3,2,0,0);
		// 2100/36.97cm=56.8028/cm, 1000/17.15cm=58.3090/cm ==> 25.4cm - 1442.79, 7.5cm - 437.318
//	}
	hname = Form("hvfatEff3_%d",i);
	//htitle = Form("Efficiency VS VFAT (floor%d)",i);
	//htitle = Form("Efficiency VS VFAT (floor%d, ROW%d/",i,(i/2)+1);
	htitle = Form("Efficiency VS VFAT (%d/X/",(i/2)+1)+TB[i%2]+")";
        //if(i%2==0) htitle += "B)";
	//else htitle += "T)";
	hvfatEff3[i] = new TH2D(hname,htitle,maxNphi*3,1,maxNphi*3+1,maxNeta,1,maxNeta+1);
	hvfatEff3[i]->GetXaxis()->SetTitle("ith-#phi");
	hvfatEff3[i]->GetYaxis()->SetTitle("ith-#eta");
	hvfatEff3[i]->SetMaximum(maxVfatEff + (maxVfatEff-minVfatEff)*0.05);
	hvfatEff3[i]->SetMinimum(minVfatEff - (maxVfatEff-minVfatEff)*0.05);
	for(int j=0;j<maxNphi*3;j++)
	{
		for(int k=0;k<maxNeta;k++)
		{
			int n1 = (j/3)*10+i;
			int n2 = j%3;
			int n3 = k;
			hvfatEff3[i]->SetBinContent(j+1,k+1, vfatEff[n1][n2][n3]);
		}
	}
	c4->cd();
	hvfatEff3[i]->Draw("colz text");
	line1->Draw();
	line2->Draw();
	png = savePngDir+"/run"+run+Form("_eff_VS_vfat_floor%d.png",i);
	c4->Print(png);

//	int n0 = 11-i;
//	if(i>=5) n0--;
//	c5->cd(n0)->SetRightMargin(0.15);
//	hvfatEff3[i]->Draw("colz");

//	c5[i/5]->cd(i%5+1)->SetRightMargin(0.15);
//	c5[i/5]->cd(5-i%5)->SetRightMargin(0.15);
//	hvfatEff3[i]->Draw("colz");
//	if(i%5==4)
//	{
//		png = savePngDir+"/run"+run+Form("_eff_VS_vfat_all0%d.png",i/5+1);
//		c5[i/5]->Print(png);
//	}
}

gStyle->SetTitleFontSize(0.1);
line1->SetLineWidth(1);
line2->SetLineWidth(1);
for(int i=0;i<maxNlayer/3;i++)
{
	int n0 = 11-i;
	if(i>=5) n0--;
	c5->cd(n0)->SetRightMargin(0.15);
	//htitle = Form("floor%d",i);
	htitle = Form("%d/X/",(i/2)+1)+TB[i%2];;
        //if(i%2==0) htitle += "B)";
	//else htitle += "T)";
	hvfatEff3[i]->SetTitle(htitle);
	hvfatEff3[i]->Draw("colz");

	//for bug fix at line width in ROOT
//	if(n0%3==1) line1->SetLineWidth(5);
//	else line1->SetLineWidth(2);
//	if(n0%3==0) line1->SetLineWidth(5);
//	else line1->SetLineWidth(2);

	//if(n0==10) line1->SetLineWidth(5);
	//else line1->SetLineWidth(2);
	line1->Draw();
	line2->Draw();
}
c5->cd();
png = savePngDir+"/run"+run+"_eff_VS_vfat_AllInOne.png";
c5->Print(png);

if(makeTreeOn)
{
	fEff->cd();
	tree->Write();
	fEff->Close();
}

f->cd("gemcrValidation");
double nev1 = hev->GetBinContent(1);
double nev2 = hev->GetBinContent(2);
double eff1 = nev2/nev1;
double eff2 = minVfatDenominator/nev1;
cout<<"nev1 "<<hev->GetBinContent(1)<<endl; //good event ratio
cout<<"nev2 "<<hev->GetBinContent(2)<<endl; //min hits per triggered event
cout<<"eff1 "<<eff1<<endl;
cout<<"eff2 "<<eff2<<endl;


}

