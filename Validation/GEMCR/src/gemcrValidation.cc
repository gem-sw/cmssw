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
  InputTagToken_RH = consumes<GEMRecHitCollection>(cfg.getParameter<edm::InputTag>("recHitsInputLabel"));
  InputTagToken_TR = consumes<vector<reco::Track>>(cfg.getParameter<edm::InputTag>("tracksInputLabel"));
  InputTagToken_TS = consumes<vector<TrajectorySeed>>(cfg.getParameter<edm::InputTag>("seedInputLabel"));
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

  tr_chamber = ibooker.book1D("tr_eff_ch", "tr rec /chamber",n_ch,0,n_ch); 
  th_chamber = ibooker.book1D("th_eff_ch", "tr hit/chamber",n_ch,0,n_ch); 
  rh_chamber = ibooker.book1D("rh_eff_ch", "rec hit/chamber",n_ch,0,n_ch); 
  for(int c = 0; c<n_ch; c++){
   cout << gemChambers[c].id() << endl;
   GEMDetId gid = gemChambers[c].id();
   string b_name = "chamber_"+to_string(gid.chamber())+"_layer_"+to_string(gid.layer());
   tr_chamber->setBinLabel(c+1,b_name);
   th_chamber->setBinLabel(c+1,b_name);
   rh_chamber->setBinLabel(c+1,b_name);
  }
  for(int c = 0; c<n_ch;c++){
     GEMDetId gid = gemChambers[c].id();
     string h_name = "chamber_"+to_string(gid.chamber())+"_layer_"+to_string(gid.layer());
     gem_chamber_x_y.push_back(ibooker.book2D(h_name+"_x_roll",h_name+" x vs roll", 500,-25,25,10,0,10));
     gem_chamber_cl_size.push_back(ibooker.book2D(h_name+"_cl_size", h_name+" cluster size", 20,0,20,24,0,24));
     gem_chamber_firedStrip.push_back(ibooker.book2D(h_name+"_firedStrip", h_name+" fired strip", 385,0,385,10,0,10));
     gem_chamber_bx.push_back(ibooker.book2D(h_name+"_bx", h_name+" BX", 30,-15,15,10,0,10));
     gem_chamber_tr2D_eff.push_back(ibooker.book2D(h_name+"_tr2D_eff", h_name+"_tr2D_eff", 10,0,5,18,0,9));
     gem_chamber_th2D_eff.push_back(ibooker.book2D(h_name+"_th2D_eff", h_name+"_th2D_eff", 10,0,5,18,0,9));
  }
}

int gemcrValidation::findIndex(GEMDetId id_) {
  int index=-1;
  for(int c =0;c<n_ch;c++){
    if((gemChambers[c].id().chamber() == id_.chamber())&(gemChambers[c].id().layer() == id_.layer()) ){index = c;}
  }
  return index;
}

int gemcrValidation::findvfat(float x, float a, float b) {
  float step = (max(a,b)-min(a,b))/3.0;
  if ( x < (min(a,b)+step) ) { return 1;}
  else if ( x < (min(a,b)+2.0*step) ) { return 2;}
  else { return 3;}
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
}

void gemcrValidation::analyze(const edm::Event& e, const edm::EventSetup& iSetup){

  theService->update(iSetup);

  edm::Handle<GEMRecHitCollection> gemRecHits;
  edm::Handle<vector<reco::Track>> gemTracks;
  edm::Handle<vector<TrajectorySeed>> trSeed;

  e.getByToken( this->InputTagToken_RH, gemRecHits);
  e.getByToken( this->InputTagToken_TR, gemTracks);
  e.getByToken( this->InputTagToken_TS, trSeed);
  if (!gemRecHits.isValid()) {
    edm::LogError("gemcrValidation") << "Cannot get strips by Token RecHits Token.\n";
    return ;
  }

  vector<bool> checkRH, checkTH, checkTR;
  vector<int> check_th_roll, check_th_vfat;//, check_tr_roll, check_tr_vfat;
   
  for(int c=0;c<n_ch;c++){
    checkRH.push_back(0);
    checkTH.push_back(0);
    checkTR.push_back(0);
    check_th_roll.push_back(-1);
    //check_tr_roll.push_back(-1);
    check_th_vfat.push_back(-1);
    //check_tr_vfat.push_back(-1);
  }
  for (GEMRecHitCollection::const_iterator recHit = gemRecHits->begin(); recHit != gemRecHits->end(); ++recHit){

    Float_t  rh_l_x = recHit->localPosition().x();
    Int_t  bx = recHit->BunchX();
    Int_t  clusterSize = recHit->clusterSize();
    Int_t  firstClusterStrip = recHit->firstClusterStrip();

    GEMDetId id((*recHit).gemId());
    int index = findIndex(id);
    checkRH[index] = 1;
    Short_t rh_roll = (Short_t) id.roll();
    LocalPoint recHitLP = recHit->localPosition();

    if ( GEMGeometry_->idToDet((*recHit).gemId()) == nullptr) {
      std::cout<<"This gem recHit did not matched with GEMGeometry."<<std::endl;
      continue;
    }
    GlobalPoint recHitGP = GEMGeometry_->idToDet((*recHit).gemId())->surface().toGlobal(recHitLP);

    Float_t     rh_g_X = recHitGP.x();
    Float_t     rh_g_Y = recHitGP.y();
    Float_t     rh_g_Z = recHitGP.z();
    int nVfat = findvfat(firstClusterStrip, 0, 128*3) + (rh_roll-1)*3;
    gem_chamber_x_y[index]->Fill(rh_l_x, rh_roll);
    gem_chamber_cl_size[index]->Fill(clusterSize, nVfat-1);
    gem_chamber_bx[index]->Fill(bx,rh_roll);
    gemcr_g->Fill(rh_g_X,rh_g_Z,rh_g_Y);
    gem_cls_tot->Fill(clusterSize);
    gem_bx_tot->Fill(bx);
     
    for(int i = firstClusterStrip; i < (firstClusterStrip + clusterSize); i++){
      gem_chamber_firedStrip[index]->Fill(i,rh_roll);
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
        if ((tlp.x()>(min_x)) & (tlp.x() < (max_x)) ){ 
          check_th_roll[c] = mRoll;
          check_th_vfat[c] = findvfat(tlp.x(), min_x, max_x);
          checkTH[c] = 1; 
          continue;
        }
      }
      for(trackingRecHit_iterator hit = tr.recHitsBegin(); hit != tr.recHitsEnd(); hit++){
        //LocalPoint tr_rh_lp = (*hit)->localPosition();
        GEMDetId tid = GEMDetId((*hit)->geographicalId());
        int index = findIndex(tid);
        //GlobalPoint recHitGP = gemChambers[index].surface().toGlobal(tr_rh_lp);
        checkTR[index] = 1;
        //check_tr_roll[index] = tid.roll();
        //float min_x = gemChambers[index].etaPartition(tid.roll())->centreOfStrip(0).x();
        //float max_x = gemChambers[index].etaPartition(tid.roll())->centreOfStrip(128*3).x();
        //check_tr_vfat[index] = findvfat(tr_rh_lp.x(),min_x, max_x);
        //cout << "roll : " << tid.roll() << ", vfat : " << findvfat(tr_rh_lp.x(),min_x, max_x) << endl;
      }
      for(int c=0;c<n_ch;c++){
        if (!checkTH[c]) {continue;}
        gem_chamber_th2D_eff[c]->Fill(check_th_vfat[c],check_th_roll[c]);
        if(checkTR[c]){gem_chamber_tr2D_eff[c]->Fill(check_th_vfat[c],check_th_roll[c]);}
      }
    }
  }
}


