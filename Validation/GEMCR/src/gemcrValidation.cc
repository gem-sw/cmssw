#include "Validation/GEMCR/interface/gemcrValidation.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "Geometry/GEMGeometry/interface/GEMSuperChamber.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/GeometrySurface/interface/Bounds.h"
#include "DataFormats/GeometryVector/interface/LocalPoint.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/TrajectorySeed/interface/PropagationDirection.h"
#include "DataFormats/Math/interface/Vector.h"
#include "DataFormats/Math/interface/Point3D.h"
#include "DataFormats/Common/interface/RefToBase.h" 
#include "DataFormats/TrajectoryState/interface/PTrajectoryStateOnDet.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"




#include <iomanip>

#include <TCanvas.h>
using namespace std;

gemcrValidation::gemcrValidation(const edm::ParameterSet& cfg): GEMBaseValidation(cfg)
{
  //InputTagToken_   = consumes<edm::PSimHitContainer>(cfg.getParameter<edm::InputTag>("simInputLabel"));
  InputTagToken_RH = consumes<GEMRecHitCollection>(cfg.getParameter<edm::InputTag>("recHitsInputLabel"));
  InputTagToken_TR = consumes<vector<reco::Track>>(cfg.getParameter<edm::InputTag>("tracksInputLabel"));
  InputTagToken_TS = consumes<vector<TrajectorySeed>>(cfg.getParameter<edm::InputTag>("seedInputLabel"));
  //InputTagToken_GP = consumes<vector<reco::GenParticle>>(cfg.getParameter<edm::InputTag>("genParticleLabel"));
  edm::ParameterSet serviceParameters = cfg.getParameter<edm::ParameterSet>("ServiceParameters");
  theService = new MuonServiceProxy(serviceParameters);
}

void gemcrValidation::bookHistograms(DQMStore::IBooker & ibooker, edm::Run const & Run, edm::EventSetup const & iSetup ) {
  GEMGeometry_ = initGeometry(iSetup);
  if ( GEMGeometry_ == nullptr) return ;  

  const std::vector<const GEMSuperChamber*>& superChambers_ = GEMGeometry_->superChambers();   
  for (auto sch : superChambers_){
    int n_lay = sch->nChambers();
    for (int l=0;l<n_lay;l++){
      gemChambers.push_back(*sch->chamber(l+1));
    }
  }
  n_ch = gemChambers.size();
  
  ibooker.setCurrentFolder("MuonGEMRecHitsV/GEMRecHitsTask");

  gemcr_g = ibooker.book3D("gemcr_g","GEMCR GLOBAL RECHITS", 500,-25,25,20,-55,55, 140,0,140);
  gem_cls_tot = ibooker.book1D("cluster_size","Cluseter size",20,0,20);  
  gem_bx_tot = ibooker.book1D("bx", "BX" , 30, -15,15);
  tr_size = ibooker.book1D("tr_size", "track size",10,0,10);
  tr_hit_size = ibooker.book1D("tr_hit_size", "hit size in track",15,0,15); 
  del_rx = ibooker.book1D("rec_tr_del_x", "rec-tr del x", 200,-10,10);
  del_ry = ibooker.book1D("rec_tr_del_y", "rec-tr del y", 200,-100,100);

  tr_chamber = ibooker.book1D("tr_eff_ch", "tr rec /chamber",n_ch,0,n_ch); 
  th_chamber = ibooker.book1D("th_eff_ch", "tr hit/chamber",n_ch,0,n_ch); 
  rh_chamber = ibooker.book1D("rh_eff_ch", "rec hit/chamber",n_ch,0,n_ch); 
  //sh_chamber = ibooker.book1D("sh_eff_ch", "sim hit/chamber",n_ch,0,n_ch); 
  for(int c = 0; c<n_ch; c++){
   cout << gemChambers[c].id() << endl;
   GEMDetId gid = gemChambers[c].id();
   string b_name = "chamber_"+to_string(gid.chamber())+"_layer_"+to_string(gid.layer());
   tr_chamber->setBinLabel(c+1,b_name);
   th_chamber->setBinLabel(c+1,b_name);
   rh_chamber->setBinLabel(c+1,b_name);
   //sh_chamber->setBinLabel(c+1,b_name);
  }
  for(int c = 0; c<n_ch;c++){
     GEMDetId gid = gemChambers[c].id();
     string h_name = "chamber_"+to_string(gid.chamber())+"_layer_"+to_string(gid.layer());
     gem_chamber_x_y.push_back(ibooker.book2D(h_name+"_x_roll",h_name+" x vs roll", 500,-25,25,10,0,10));
     gem_chamber_cl_size.push_back(ibooker.book2D(h_name+"_cl_size", h_name+" cluster size", 20,0,20,10,0,10));
     gem_chamber_firedStrip.push_back(ibooker.book2D(h_name+"_firedStrip", h_name+" fired strip", 385,0,385,10,0,10));
     gem_chamber_bx.push_back(ibooker.book2D(h_name+"_bx", h_name+" BX", 30,-15,15,10,0,10));
     gem_chamber_th_eff.push_back(ibooker.book1D(h_name+"_th_eff", h_name+" th eff", n_ch,0,n_ch));
     gem_chamber_tr_eff.push_back(ibooker.book1D(h_name+"_tr_eff", h_name+" tr eff", n_ch,0,n_ch));
     gem_chamber_rec_eff.push_back(ibooker.book1D(h_name+"_rec_eff", h_name+" rec eff", n_ch,0,n_ch));
     gem_chamber_tr2D_eff.push_back(ibooker.book2D(h_name+"_tr2D_eff", h_name+"_tr2D_eff", 10,-25,25,28,-70,70));
     gem_chamber_th2D_eff.push_back(ibooker.book2D(h_name+"_th2D_eff", h_name+"_th2D_eff", 10,-25,25,28,-70,70));
     for(int r = 1; r<9;r++){
       string r_name = h_name+"_roll_"+to_string(r);
       gem_chamber_trroll_eff.push_back(ibooker.book1D(r_name+"_tr_eff", r_name+"_tr_eff",4,-10,10));
       gem_chamber_throll_eff.push_back(ibooker.book1D(r_name+"_th_eff", r_name+"_th_eff",4,-10,10));

     }
  }
  numRec1 = 0; numRec2 = 0; numRec3 = 0;
  //numSim1 = 0; numSim2 = 0; numSim3 = 0;
  numTH = 0; numTHfp = 0;
  numTR = 0; numTRfp = 0;
  countNum = 0;
  //bEff = new  TEfficiency("biErr", "biErr", n_ch,0,n_ch); 
}

int gemcrValidation::findIndex(GEMDetId id_){
  int index=-1;
  for(int c =0;c<n_ch;c++){
    if((gemChambers[c].id().chamber() == id_.chamber())&(gemChambers[c].id().layer() == id_.layer()) ){index = c;}
  }
  return index;
}

const GEMGeometry* gemcrValidation::initGeometry(edm::EventSetup const & iSetup) {
  const GEMGeometry* GEMGeometry_ = nullptr;
  try {
    edm::ESHandle<GEMGeometry> hGeom;
    iSetup.get<MuonGeometryRecord>().get(hGeom);
    GEMGeometry_ = &*hGeom;
  }
  catch( edm::eventsetup::NoProxyException<GEMGeometry>& e) {
    edm::LogError("MuonGEMBaseValidation") << "+++ Error : GEM geometry is unavailable on event loop. +++\n";
    return nullptr;
  }
  return GEMGeometry_;
}

gemcrValidation::~gemcrValidation() {
  std::cout << " JOB END!" << std::endl;

  //std::cout << "eff ori : " << numRec1/numSim1*100.0 << std::endl;
  //std::cout << "eff w tr : " << numRec2/numSim2*100.0 << std::endl;
  std::cout << "eff Tr : " << numTR/numTH*100.0 << std::endl;
  std::cout << "eff Tr with cut : " << numTRfp/numTHfp*100.0 << std::endl;
  //std::cout << numRec1 << ", " << numSim1 << "\n" << numRec2 << ", " << numSim2 << "\n" << numTR << ", " << numTH <<"\n" << numTRfp << ", " << numTHfp <<std::endl;
  //std::cout << "eff 3 : " << numRec3/numSim3*100.0 << std::endl;
  //std::cout << numRec3 << ", " << numSim3 << std::endl;
  std::cout << "contN : " << countNum << std::endl;
  effOut->cd();
  bEff->Write();
  bEff2->Write();
  bEff3->Write();
  effOut->Write();
  effOut->Close(); 
   //TCanvas* c1 = new TCanvas("example","",600,400);
   //c1->SetFillStyle(1001);
   //c1->SetFillColor(kWhite);
   //bEff->Draw("AP");
   //c1->SaveAs("test.png");
}

void gemcrValidation::analyze(const edm::Event& e, const edm::EventSetup& iSetup){

  theService->update(iSetup);

  edm::Handle<GEMRecHitCollection> gemRecHits;
  //edm::Handle<edm::PSimHitContainer> gemSimHits;
  edm::Handle<vector<reco::Track>> gemTracks;
  edm::Handle<vector<TrajectorySeed>> trSeed;
  edm::Handle<vector<reco::GenParticle>> gMuon; 

  //e.getByToken( this->InputTagToken_, gemSimHits);
  e.getByToken( this->InputTagToken_RH, gemRecHits);
  e.getByToken( this->InputTagToken_TR, gemTracks);
  e.getByToken( this->InputTagToken_TS, trSeed);
  //e.getByToken( this->InputTagToken_GP, gMuon);
  if (!gemRecHits.isValid()) {
    edm::LogError("gemcrValidation") << "Cannot get strips by Token RecHits Token.\n";
    return ;
  }

  //vector<bool> checkRH, checkSH, checkTH, checkTR;
  vector<bool> checkRH, checkTH, checkTR;
  vector<int> check_th_roll, check_tr_roll;
  vector<double> tr_hit_x, tr_hit_y, roll_y;
   
  for(int c=0;c<n_ch;c++){
    checkRH.push_back(0);
    //checkSH.push_back(0);
    checkTH.push_back(0);
    checkTR.push_back(0);
    check_th_roll.push_back(-1);
    check_tr_roll.push_back(-1);
    tr_hit_x.push_back(-99);
    tr_hit_y.push_back(-99);
    roll_y.push_back(-99);
  }
  /*for (edm::PSimHitContainer::const_iterator hits = gemSimHits->begin(); hits!=gemSimHits->end(); ++hits) {
    const GEMDetId id(hits->detUnitId());
    int index = findIndex(id);
    checkSH[index] = 1;   
    numSim1 += 1;    
    test1D->Fill(index);
  }*/
  for (GEMRecHitCollection::const_iterator recHit = gemRecHits->begin(); recHit != gemRecHits->end(); ++recHit){

    Float_t  rh_l_x = recHit->localPosition().x();
    Int_t  bx = recHit->BunchX();
    Int_t  clusterSize = recHit->clusterSize();
    Int_t  firstClusterStrip = recHit->firstClusterStrip();

    GEMDetId id((*recHit).gemId());
    int index = findIndex(id);
    checkRH[index] = 1;
    numRec1 += 1;
    Short_t rh_roll = (Short_t) id.roll();
    LocalPoint recHitLP = recHit->localPosition();

    if ( GEMGeometry_->idToDet((*recHit).gemId()) == nullptr) {
      std::cout<<"This gem recHit did not matched with GEMGeometry."<<std::endl;
      continue;
    }
    //for(int c=0;c<n_ch;c++){
    //  bEff->Fill(checkRH[c], c);
    //}
    GlobalPoint recHitGP = GEMGeometry_->idToDet((*recHit).gemId())->surface().toGlobal(recHitLP);

    Float_t     rh_g_X = recHitGP.x();
    Float_t     rh_g_Y = recHitGP.y();
    Float_t     rh_g_Z = recHitGP.z();
    gem_chamber_x_y[index]->Fill(rh_l_x, rh_roll);
    gem_chamber_cl_size[index]->Fill(clusterSize,rh_roll);
    gem_chamber_bx[index]->Fill(bx,rh_roll);
    gemcr_g->Fill(rh_g_X,rh_g_Z,rh_g_Y);
    gem_cls_tot->Fill(clusterSize);
    gem_bx_tot->Fill(bx);
     
    std::vector<int> stripsFired;
    for(int i = firstClusterStrip; i < (firstClusterStrip + clusterSize); i++){
      gem_chamber_firedStrip[index]->Fill(i,rh_roll);
      stripsFired.push_back(i);
    }
  }

  if (gemTracks.isValid()){
    tr_size->Fill(gemTracks.product()->size());
    for(auto tr: *gemTracks.product()){
      TrajectorySeed seed = trSeed.product()->back();
      PTrajectoryStateOnDet ptsd1(seed.startingState());
      DetId did(ptsd1.detId());
      const BoundPlane& bp = theService->trackingGeometry()->idToDet(did)->surface();
      TrajectoryStateOnSurface tsos = trajectoryStateTransform::transientState(ptsd1,&bp,&*theService->magneticField());
      TrajectoryStateOnSurface tsosCurrent = tsos;
      for(int c=0; c<n_ch;c++){
        GEMChamber ch = gemChambers[c];
        tsosCurrent = theService->propagator("SteppingHelixPropagatorAny")->propagate(tsosCurrent,ch.surface());
        if (!tsosCurrent.isValid()) continue;
        Global3DPoint gtrp = tsosCurrent.freeTrajectoryState()->position();
        Local3DPoint tlp = ch.surface().toLocal(gtrp);
        if (!ch.surface().bounds().inside(tlp)){continue;}
        int n_roll = ch.nEtaPartitions();
        double minDely = 50.;
        int mRoll = -1;
        for (int r=0;r<n_roll;r++){
          Local3DPoint rtlp = ch.etaPartition(r+1)->surface().toLocal(gtrp);
          if(minDely > abs(rtlp.y())){minDely = abs(rtlp.y()); mRoll = r+1;}
        }
        if (mRoll == -1){cout << "no mRoll" << endl;continue;}
        int n_strip = ch.etaPartition(mRoll)->nstrips();
        double min_x = ch.etaPartition(mRoll)->centreOfStrip(0).x();
        double max_x = ch.etaPartition(mRoll)->centreOfStrip(n_strip).x();
        if ((gtrp.x()>(min_x)) & (gtrp.x() < (max_x)) ){ 
          Local3DPoint rtlp = ch.etaPartition(mRoll)->surface().toLocal(gtrp);
          check_th_roll[c] = mRoll;
          checkTH[c] = 1; 
          numTH += 1; 
          tr_hit_x[c] = rtlp.x();
          tr_hit_y[c] = gtrp.z(); 
          roll_y[c] = rtlp.y(); 
          continue;
        }
      }
      for(trackingRecHit_iterator hit = tr.recHitsBegin(); hit != tr.recHitsEnd(); hit++){
        //LocalPoint tr_rh_lp = (*hit)->localPosition();
        GEMDetId tid = GEMDetId((*hit)->geographicalId());
        int index = findIndex(tid);
        //GlobalPoint recHitGP = gemChambers[index].surface().toGlobal(tr_rh_lp);
        checkTR[index] = 1;
        check_tr_roll[index] = tid.roll();
        numTR += 1;
      }
      for(int c=0;c<n_ch;c++){
        if (!checkTH[c]) {continue;}
        if(checkRH[c] and !checkTR[c]){
        del_rx->Fill(c);}
        if (checkRH[c]){bEff3->Fill(checkTR[c],c);}
        bEff->Fill(checkTR[c],c);
        bEff2->Fill(checkRH[c],c);
        gem_chamber_th2D_eff[c]->Fill(tr_hit_x[c],tr_hit_y[c]);
        if(checkRH[c]){gem_chamber_tr2D_eff[c]->Fill(tr_hit_x[c],tr_hit_y[c]);}
        countNum += 1;
        //if (tr_hit_y[c] == -99) {continue;}
        //if (checkSH[c]){sh_chamber->Fill(c); }
        if (checkRH[c]){rh_chamber->Fill(c); }
        if (checkTH[c]){th_chamber->Fill(c); gem_chamber_th_eff[c]->Fill(check_th_roll[c]); numTHfp += 1;gem_chamber_throll_eff[c*8+check_th_roll[c]-1]->Fill(roll_y[c]);}
        if (checkTR[c]){tr_chamber->Fill(c); gem_chamber_tr_eff[c]->Fill(check_th_roll[c]); numTRfp += 1;gem_chamber_trroll_eff[c*8+check_th_roll[c]-1]->Fill(roll_y[c]);}
      }
    }
  }
}


