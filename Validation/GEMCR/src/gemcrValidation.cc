#include "Validation/GEMCR/interface/gemcrValidation.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "Geometry/GEMGeometry/interface/GEMSuperChamber.h"
#include <Geometry/GEMGeometry/interface/GEMGeometry.h>
#include <Geometry/Records/interface/MuonGeometryRecord.h>

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

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeed.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "RecoMuon/TransientTrackingRecHit/interface/MuonTransientTrackingRecHit.h"
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"

#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "DataFormats/DetId/interface/DetId.h"

#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"


#include <iomanip>
#include<exception>
#include <TCanvas.h>
using namespace std;

// ----------member data ---------------------------   
//unsigned int SeedsNumber;

MuonRecHitContainerLayered::MuonRecHitContainerLayered( MuonTransientTrackingRecHit::MuonRecHitContainer &rechit, std::vector<int> _Layers )
  :MuonRecHitContainer( rechit )//MuonRecHitContaine constructor call
{
  Layers = _Layers;
}

MuonRecHitContainerLayered::~MuonRecHitContainerLayered( void ){

}
int MuonRecHitContainerLayered::GetLayer( unsigned int _L ){
  try {
    if( _L > Layers.capacity() )
      {
	throw  "Max number of layer exceeded";
      }
  }catch(const char* msg)
    {
      std::cerr << msg << endl;
    }
  return Layers[_L];  
}

//MuonTransientTrackingRecHit::MuonRecHitContaine& MuonRecHitContainerLayered::GetmuRecHits( void ){
//  return MuonRecHitContainer;
//}


gemcrValidation::gemcrValidation(const edm::ParameterSet& cfg): GEMBaseValidation(cfg)
{
  InputTagToken_RH = consumes<GEMRecHitCollection>(cfg.getParameter<edm::InputTag>("recHitsInputLabel"));
  InputTagToken_TR = consumes<vector<reco::Track>>(cfg.getParameter<edm::InputTag>("tracksInputLabel"));
  InputTagToken_TS = consumes<vector<TrajectorySeed>>(cfg.getParameter<edm::InputTag>("seedInputLabel"));
  InputTagToken_DG = consumes<GEMDigiCollection>(cfg.getParameter<edm::InputTag>("gemDigiLabel"));
  edm::ParameterSet serviceParameters = cfg.getParameter<edm::ParameterSet>("ServiceParameters");
  theService = new MuonServiceProxy(serviceParameters);
  minCLS = cfg.getParameter<double>("minClusterSize"); 
  maxCLS = cfg.getParameter<double>("maxClusterSize");
  maxRes = cfg.getParameter<double>("maxResidual");
  makeTrack = cfg.getParameter<bool>("makeTrack");
  isMC = cfg.getParameter<bool>("isMC");
  trackChi2 = cfg.getParameter<double>("trackChi2");
  trackResY = cfg.getParameter<double>("trackResY"); 
  trackResX = cfg.getParameter<double>("trackResX");
  edm::ParameterSet smootherPSet = cfg.getParameter<edm::ParameterSet>("MuonSmootherParameters");
  theSmoother = new CosmicMuonSmoother(smootherPSet, theService);
  theUpdator = new KFUpdator();
  refChambers = cfg.getParameter<std::vector<int>>("refChambers");//Reference chambers
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
  
  gemcr_g = ibooker.book3D("gemcr_g","GEMCR GLOBAL RECHITS", 200,-100,100,20,-55,55, 140,0,140);
  gem_cls_tot = ibooker.book1D("cluster_size","Cluseter size",20,0,20);  
  gem_bx_tot = ibooker.book1D("bx", "BX" , 30, -15,15);
  tr_size = ibooker.book1D("tr_size", "track size",10,0,10);
  tr_hit_size = ibooker.book1D("tr_hit_size", "hit size in track",15,0,15); 
  trajectoryh = ibooker.book1D("trajectory","trajectory", 4,0,4);
  trajectoryh->setBinLabel(1, "total seeds");
  trajectoryh->setBinLabel(2, "unvalid");
  trajectoryh->setBinLabel(3, "valid");
  trajectoryh->setBinLabel(4, "passed chi2");
  firedMul = ibooker.book1D("firedMul","fired chamber multiplicity",n_ch+1,0,n_ch+1);
  firedChamber = ibooker.book1D("firedChamber", "fired chamber",n_ch,0,n_ch);

  tr_chamber = ibooker.book1D("tr_eff_ch", "tr rec /chamber",n_ch,0,n_ch); 
  th_chamber = ibooker.book1D("th_eff_ch", "tr hit/chamber",n_ch,0,n_ch); 
  rh_chamber = ibooker.book1D("rh_eff_ch", "rec hit/chamber",n_ch,0,n_ch); 
  rh1_chamber = ibooker.book1D("rh1_chamber", "all recHits",n_ch,0,n_ch); 
  rh2_chamber = ibooker.book1D("rh2_chamber", "cut passed recHits ",n_ch,0,n_ch); 
  rh3_chamber = ibooker.book1D("rh3_chamber", "tracking recHits",n_ch,0,n_ch); 
  //Counter of Number of seed CAS
  NumberOfSeeds  = ibooker.book3D("NumberOfSeeds","RECHITS only from seeds", 200,-100,100,20,-55,55, 140,0,140);
  

  for(int c = 0; c<n_ch; c++){
    //   cout << gemChambers[c].id() << endl;#CAS
   GEMDetId gid = gemChambers[c].id();
   string b_name = "chamber_"+to_string(gid.chamber())+"_layer_"+to_string(gid.layer());
   tr_chamber->setBinLabel(c+1,b_name);
   th_chamber->setBinLabel(c+1,b_name);
   rh_chamber->setBinLabel(c+1,b_name);
   rh1_chamber->setBinLabel(c+1,b_name);
   rh2_chamber->setBinLabel(c+1,b_name);
   rh3_chamber->setBinLabel(c+1,b_name);
   firedChamber->setBinLabel(c+1,b_name);
  }
  for(int c = 0; c<n_ch;c++){
     GEMDetId gid = gemChambers[c].id();
     string h_name = "chamber_"+to_string(gid.chamber())+"_layer_"+to_string(gid.layer());
     gem_chamber_x_y.push_back(ibooker.book2D(h_name+"_recHit",h_name+" recHit", 500,-25,25,8,1,9));
     gem_chamber_cl_size.push_back(ibooker.book2D(h_name+"_recHit_size", h_name+" recHit size", 50,0,50,24,0,24));
     gem_chamber_bx.push_back(ibooker.book2D(h_name+"_bx", h_name+" BX", 30,-15,15,10,0,10));
     gem_chamber_tr2D_eff.push_back(ibooker.book2D(h_name+"_recHit_efficiency", h_name+" recHit efficiency", 3,1,4,8,1,9));
     gem_chamber_th2D_eff.push_back(ibooker.book2D(h_name+"_th2D_eff", h_name+"_th2D_eff", 3,1,4,8,1,9));
     gem_chamber_trxroll_eff.push_back(ibooker.book2D(h_name+"_trxroll_eff", h_name+" recHit efficiency", 50,-25,25,8,1,9));
     gem_chamber_trxy_eff.push_back(ibooker.book2D(h_name+"_trxy_eff", h_name+" recHit efficiency", 50,-25,25,120,-60,60));
     gem_chamber_thxroll_eff.push_back(ibooker.book2D(h_name+"_thxroll_eff", h_name+"_th2D_eff", 50,-25,25,8,1,9));
     gem_chamber_thxy_eff.push_back(ibooker.book2D(h_name+"_thxy_eff", h_name+"_th2D_eff", 50,-25,25,120,-60,60));
     gem_chamber_residual.push_back(ibooker.book2D(h_name+"_residual", h_name+" residual", 500,-25,25,100,-5,5));
     gem_chamber_local_x.push_back(ibooker.book2D(h_name+"_local_x", h_name+" local x",500,-25,25,500,-25,25));
     gem_chamber_digi_digi.push_back(ibooker.book2D(h_name+"_digi_gemDigi", h_name+" gemDigi (DIGI)", 384,0,384,8,1,9));
     gem_chamber_digi_recHit.push_back(ibooker.book2D(h_name+"_recHit_gemDigi", h_name+" gemDigi (recHit)", 384,0,384,8,1,9));
     gem_chamber_digi_CLS.push_back(ibooker.book2D(h_name+"_CLS_gemDigi", h_name+" gemDigi (CLS)", 384,0,384,8,1,9));
     gem_chamber_hitMul.push_back(ibooker.book1D(h_name+"_hit_mul", h_name+" hit multiplicity",25,0,25 ));
     gem_chamber_vfatHitMul.push_back(ibooker.book2D(h_name+"_vfatHit_mul", h_name+" vfat hit multiplicity",25,0,25, 24,0,24));
     gem_chamber_stripHitMul.push_back(ibooker.book2D(h_name+"_stripHit_mul", h_name+" strip hit multiplicity", 150,0,150,9,0,9));
     gem_chamber_bestChi2.push_back(ibooker.book1D(h_name+"_bestChi2", h_name+" #chi^{2} distribution", trackChi2*10,0,trackChi2));
     gem_chamber_track.push_back(ibooker.book1D(h_name+"_track", h_name+" track",5,0,5));
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
  float step = abs(b-a)/3.0;
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


/////////////////////////////////////////////////
/// Function to modify CAS
/////////////////////////////////////////////////

/*This function recives MuonRecHitContainer and return std::vector<TrajectorySeed> */
auto_ptr<std::vector<TrajectorySeed> > gemcrValidation::findSeeds(MuonRecHitContainerLayered &muRecHits)
/*auto_ptr is a smart pointer that manages an object obtained via new expression and deletes that object when auto_ptr itself is destroyed*/
/*std::unique_ptr is preferred for this and other uses. (since C++11)*/
{
  auto_ptr<std::vector<TrajectorySeed> > tmptrajectorySeeds( new vector<TrajectorySeed>());
  for (auto hit1 : muRecHits){/* Range-based for loop, access by value, the type of hit1 is MuonTransientTrackingRecHit::MuonRecHitContaine&*/
    
    /////////////////////////////////////////////////////////////
    // Requesting Only seed for reference layer
    /////////////////////////////////////////////////////////////
    GEMDetId hit1ID(hit1->rawId());
    //    cout<< "layer" <<hit1ID.layer() <<endl;
    //    cout<< "chamber" <<hit1ID.chamber() <<endl;
    bool flaghit1 = false;
    for (auto chambers : muRecHits.Layers){
      if( chambers % 2 == 0  ){
	int SchamberN = chambers - 1;
	if (SchamberN == hit1ID.chamber()){
	  if ( hit1ID.layer() == 1 ) {
	    flaghit1 = true;
	  }
	}
      }else{
	if ( chambers == hit1ID.chamber()){
	  if ( hit1ID.layer() == 2 ) {
	    flaghit1 = true;
	  }
	}
      }
    

      if(flaghit1){//Condition over hit1
	bool flaghit2 = false;
	for (auto hit2 : muRecHits){
	  GEMDetId hit2ID(hit2->rawId());
	  for (auto chambers : muRecHits.Layers){	 
	    if( chambers % 2 == 0  ){
	      int SchamberN = chambers - 1;
	      if (SchamberN == hit2ID.chamber()){
		if ( hit2ID.layer() == 1 ) {
		  flaghit2 = true;
		}
	      }
	    }else{
	      if ( chambers == hit2ID.chamber()){
		if ( hit2ID.layer() == 2 ) {
		  flaghit2 = true;
		}
	      }
	    }
	    
	    if (flaghit2){//Condition over the hit2
	      // 	  cout<< "hit1ID_chamber" <<hit1ID.chamber() <<endl;
	      // 	  cout<< "hit2ID_chamber" <<hit2ID.chamber() <<endl;
	      ////////////////////////////////////////////////////////////
	      
	      if (hit1->globalPosition().y() < hit2->globalPosition().y()){/*y direction is up-down? (cosmic), see if hit1 is Higher than the hit2*/
		LocalPoint segPos = hit1->localPosition();/*Define a LocalPoint with the position of hit1*/
		GlobalVector segDirGV(hit2->globalPosition().x() - hit1->globalPosition().x(),/*Define a vector called segDirGV from hit2-hit1 */
				      (hit2->globalPosition().y() - hit1->globalPosition().y()),
				      hit2->globalPosition().z() - hit1->globalPosition().z());
		//cout<< " hit1->localPosition().y()" << hit1->localPosition().y()<<endl;
		//cout<< " hit1->globalPosition().y()" << hit1->globalPosition().y()<<endl;
		segDirGV *=10;/*multiply per 10 ????*/
		//segDirGV *=1;
		LocalVector segDir = hit1->det()->toLocal(segDirGV);
		
		int charge= 1;
		LocalTrajectoryParameters param(segPos, segDir, charge);
		
		AlgebraicSymMatrix mat(5,0);
		mat = hit1->parametersError().similarityT( hit1->projectionMatrix() );
		LocalTrajectoryError error(asSMatrix<5>(mat));
		
		TrajectoryStateOnSurface tsos(param, error, hit1->det()->surface(), &*theService->magneticField());
		uint32_t id = hit1->rawId();
		PTrajectoryStateOnDet const & seedTSOS = trajectoryStateTransform::persistentState(tsos, id);
		
		edm::OwnVector<TrackingRecHit> seedHits;
		seedHits.push_back(hit1->hit()->clone());
		seedHits.push_back(hit2->hit()->clone());
		
		TrajectorySeed seed(seedTSOS,seedHits,alongMomentum);
		tmptrajectorySeeds->push_back(seed);
	      }
	    }
	  }
	}
      }
    }
  }
  return tmptrajectorySeeds;
}

Trajectory gemcrValidation::makeTrajectory(TrajectorySeed seed, MuonTransientTrackingRecHit::MuonRecHitContainer &muRecHits, std::vector<GEMChamber> gemChambers, GEMChamber testChamber)
{
  PTrajectoryStateOnDet ptsd1(seed.startingState());
  DetId did(ptsd1.detId());
  const BoundPlane& bp = theService->trackingGeometry()->idToDet(did)->surface();
  TrajectoryStateOnSurface tsos = trajectoryStateTransform::transientState(ptsd1,&bp,&*theService->magneticField());
  TrajectoryStateOnSurface tsosCurrent = tsos;
  TransientTrackingRecHit::ConstRecHitContainer consRecHits;
  for (auto ch : gemChambers){
     //cout << ch.id() << endl;
    //if (ch == testChamber) continue;
    std::shared_ptr<MuonTransientTrackingRecHit> tmpRecHit;
    tsosCurrent = theService->propagator("SteppingHelixPropagatorAny")->propagate(tsosCurrent,ch.surface());
    if (!tsosCurrent.isValid()) return Trajectory();
    GlobalPoint tsosGP = tsosCurrent.freeTrajectoryState()->position();
    float maxR = 9999;
    for (auto hit : muRecHits){
      GEMDetId hitID(hit->rawId());
      if (hitID.chamberId() == ch.id() ){
        GlobalPoint hitGP = hit->globalPosition();

        double y_err = hit->localPositionError().yy();
        //double x_err = hit->localPositionError().xx();
        //cout << "chamber #" << findIndex(ch.id()) << ", resX : " << abs(hitGP.x() - tsosGP.x()) << ", resY : " << abs(hitGP.z() - tsosGP.z()) << ", delR : " << deltaR << endl;
        //cout << "recHit (position, err x) : (" << hitGP.x() << ", "<<x_err << "), y : (" << hitGP.z() << ", "<<y_err<<")" << endl;
        if (abs(hitGP.x() - tsosGP.x()) > trackResX) continue;
        if (abs(hitGP.z() - tsosGP.z()) > y_err*trackResY) continue;
        //if (abs(hitGP.z() - tsosGP.z()) > y_err*trackResY) continue;
        float deltaR = (hitGP - tsosGP).mag();
        //cout <<"chamber : " << findIndex(ch.id()) << ", recHit : "<<hitGP << ", trackHit : " << tsosGP << ", delR : "<< deltaR << ", err :" << x_err << ", "<< y_err << endl; 
        if (maxR > deltaR){
          tmpRecHit = hit;
          maxR = deltaR;
        }
      }
    }
    if (tmpRecHit){
      tsosCurrent = theUpdator->update(tsosCurrent, *tmpRecHit);
      consRecHits.push_back(tmpRecHit);
    }
  }
  if (consRecHits.size() <3) return Trajectory();
  vector<Trajectory> fitted = theSmoother->trajectories(seed, consRecHits, tsos);
  return fitted.front();
}

void gemcrValidation::analyze(const edm::Event& e, const edm::EventSetup& iSetup){

  theService->update(iSetup);

  edm::Handle<GEMRecHitCollection> gemRecHits;
  if (!isMC){
    edm::Handle<GEMDigiCollection> digis;
    e.getByToken( this->InputTagToken_DG, digis);
    for (GEMDigiCollection::DigiRangeIterator gemdgIt = digis->begin(); gemdgIt != digis->end(); ++gemdgIt){
      const GEMDetId& gemId = (*gemdgIt).first;
      int index = findIndex(gemId);
      const GEMDigiCollection::Range& range = (*gemdgIt).second;
      for ( auto digi = range.first; digi != range.second; ++digi ) {
        gem_chamber_digi_digi[index]->Fill(digi->strip(),gemId.roll());
      }
    }
  }
  e.getByToken( this->InputTagToken_RH, gemRecHits);
  if (!gemRecHits.isValid()) {
    edm::LogError("gemcrValidation") << "Cannot get strips by Token RecHits Token.\n";
    return ;
  }
  vector<bool> firedCh;
  vector<int> rMul;
  vector<vector<int>> vMul(n_ch, vector<int>(24, 0));
  vector<vector<int>> sMul(n_ch, vector<int>(9, 0));
  for (int c=0;c<n_ch;c++){
    firedCh.push_back(0);
    rMul.push_back(0);
  }
  for (GEMRecHitCollection::const_iterator recHit = gemRecHits->begin(); recHit != gemRecHits->end(); ++recHit){

    Float_t  rh_l_x = recHit->localPosition().x();
    Int_t  bx = recHit->BunchX();
    Int_t  clusterSize = recHit->clusterSize();
    Int_t  firstClusterStrip = recHit->firstClusterStrip();

    GEMDetId id((*recHit).gemId());
    int index = findIndex(id);
    firedCh[index] = 1;
    rMul[index] += 1;
    //checkRH[index] = 1;
    Short_t rh_roll = (Short_t) id.roll();
    LocalPoint recHitLP = recHit->localPosition();

    if ( GEMGeometry_->idToDet((*recHit).gemId()) == nullptr) {
      std::cout<<"This gem recHit did not matched with GEMGeometry."<<std::endl;
      continue;
    }
    //Puts the points coordenades for a 3D plot 
    GlobalPoint recHitGP = GEMGeometry_->idToDet((*recHit).gemId())->surface().toGlobal(recHitLP);
    Float_t     rh_g_X = recHitGP.x();
    Float_t     rh_g_Y = recHitGP.y();
    Float_t     rh_g_Z = recHitGP.z();
    int nVfat = 8*(findvfat(firstClusterStrip+clusterSize*0.5, 0, 128*3)-1) + (8-rh_roll);
    vMul[index][nVfat] += 1;
    gem_chamber_x_y[index]->Fill(rh_l_x, rh_roll);
    gem_chamber_cl_size[index]->Fill(clusterSize, nVfat);
    gem_chamber_bx[index]->Fill(bx,rh_roll);
    gemcr_g->Fill(rh_g_X,rh_g_Z,rh_g_Y);
    gem_cls_tot->Fill(clusterSize);
    gem_bx_tot->Fill(bx);
    rh1_chamber->Fill(index);
    
    //Filling de 3d graph CAS 
    
    for (auto chambers : refChambers){
      if( chambers % 2 == 0  ){
	int SchamberN = chambers - 1;
	if (SchamberN == id.chamber()){
	  if ( id.layer() == 1 ) {
	    NumberOfSeeds->Fill(rh_g_X,rh_g_Z,rh_g_Y);
	  }
	}
      }else{
	if ( chambers == id.chamber()){
	  if ( id.layer() == 2 ) {
	    NumberOfSeeds->Fill(rh_g_X,rh_g_Z,rh_g_Y);
	  }	
	}
      }
    }

    for(int i = firstClusterStrip; i < (firstClusterStrip + clusterSize); i++){
      gem_chamber_digi_recHit[index]->Fill(i,rh_roll);
    }
    if (clusterSize < minCLS) continue;
    if (clusterSize > maxCLS) continue;
    rh2_chamber->Fill(index);
    for(int i = firstClusterStrip; i < (firstClusterStrip + clusterSize); i++){
      sMul[index][rh_roll] +=1;
      sMul[index][0] +=1;
      gem_chamber_digi_CLS[index]->Fill(i,rh_roll);
    }
  }
  int fChMul = 0;
  for(int c=0;c<n_ch;c++){
    gem_chamber_hitMul[c]->Fill(rMul[c]);
    for(int v=0; v<24;v++){
      gem_chamber_vfatHitMul[c]->Fill(vMul[c][v],v);    
    }
    for(int r=0; r<9;r++){
      gem_chamber_stripHitMul[c]->Fill(sMul[c][r],r);
    } 
    if (firedCh[c]){ 
      firedChamber->Fill(c+0.5);
      fChMul += 1;
    }
  }
 if (fChMul > 3) cout << "more then 3 chambers fired !"<< endl;
 firedMul->Fill(fChMul);
  /// Tracking start
  if (!makeTrack) return; 
  int countTC = 0;
  for (auto tch : gemChambers){
    countTC += 1;
    MuonTransientTrackingRecHit::MuonRecHitContainer testRecHits;
    for (auto etaPart : tch.etaPartitions()){
      GEMDetId etaPartID = etaPart->id();
      GEMRecHitCollection::range range = gemRecHits->get(etaPartID);
      for (GEMRecHitCollection::const_iterator rechit = range.first; rechit!=range.second; ++rechit){
	const GeomDet* geomDet(etaPart);
	if ((*rechit).clusterSize()<minCLS) continue;
	if ((*rechit).clusterSize()>maxCLS) continue;
	testRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));        
      }
    }
    MuonTransientTrackingRecHit::MuonRecHitContainer muRecHits;
    for (auto ch : gemChambers){
      if (tch == ch) continue;
      for (auto etaPart : ch.etaPartitions()){
        GEMDetId etaPartID = etaPart->id();
        GEMRecHitCollection::range range = gemRecHits->get(etaPartID);   
        for (GEMRecHitCollection::const_iterator rechit = range.first; rechit!=range.second; ++rechit){
          const GeomDet* geomDet(etaPart);
          if ((*rechit).clusterSize()<minCLS) continue;
          if ((*rechit).clusterSize()>maxCLS) continue;
          muRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));
        }
      }
    }
    if (muRecHits.size()<3){ 
      //cout << "tracking passed due to # recHit under 3" << endl;
      continue;}
    int TCN = 0;
    for(int c=0;c<n_ch;c++){
      if (c == (countTC - 1)) continue;
      if (firedCh[c]){ TCN += 1;}
    }
    if (TCN < 3) continue;
    gem_chamber_track[findIndex(tch.id())]->Fill(0.5);
    auto_ptr<std::vector<TrajectorySeed> > trajectorySeeds( new vector<TrajectorySeed>());

    /////////////////////////////////////////////////////////////////////////////////////
    //Changed to use the new class CAS
    /////////////////////////////////////////////////////////////////////////////////////
    //std::vector<unsigned> Layers {, true, false};//temporal 
    MuonRecHitContainerLayered muRecHitsl(muRecHits,refChambers);
    trajectorySeeds =findSeeds(muRecHitsl);
    /////////////////////////////////////////////////////////////////////////////////////

    Trajectory bestTrajectory;
    TrajectorySeed bestSeed;
    trajectoryh->Fill(0, trajectorySeeds->size());  
    //cout <<"test chamber #" << findIndex(tch.id()) << ", "<< countTC <<" # of seeds : " << trajectorySeeds->size() << ", # of recHit for track :" << muRecHits.size() << ", # of recHit for testChamber : " << testRecHits.size()<< endl;
    //if (trajectorySeeds->size() > 100) continue;
    float maxChi2 = 10000.0;
    int countTR = 0;
    for (auto seed : *trajectorySeeds){
      Trajectory smoothed;
      try{smoothed = makeTrajectory(seed, muRecHits, gemChambers,tch);}
      catch(int n){
        cout << "bad trajectory" << endl;
        throw; 
      }
      countTR += 1;
      //cout << smoothed.chiSquared()/float(smoothed.ndof()) << endl;
      if (smoothed.isValid()){
        trajectoryh->Fill(2,1);
        //cout << "Trajectory " << countTR << ", chi2 : " << smoothed.chiSquared()/float(smoothed.ndof()) << ", track ResX :" << trackResX << ", track ResY : " << trackResY << endl;
        if (maxChi2 > smoothed.chiSquared()/float(smoothed.ndof())){
          maxChi2 = smoothed.chiSquared()/float(smoothed.ndof());
          bestTrajectory = smoothed;
          bestSeed = seed;
        }
      }else{
        trajectoryh->Fill(1,1);
        //cout << "trajectory " << countTR << " is not valid" << endl;
      }
    }
    //cout << "# of trajectories : " << countTR << endl;
    //cout <<maxChi2 << endl;
    if (!bestTrajectory.isValid()) continue; //{cout<<"no Best Trajectory" << endl; continue;}
    gem_chamber_track[findIndex(tch.id())]->Fill(1.5);
    if (maxChi2 > trackChi2 or maxChi2 < 9) continue;
    trajectoryh->Fill(3,1);
    gem_chamber_bestChi2[findIndex(tch.id())]->Fill(maxChi2);
    //cout << maxChi2 << endl;
    gem_chamber_track[findIndex(tch.id())]->Fill(2.5);
    PTrajectoryStateOnDet ptsd1(bestSeed.startingState());
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
      if (ch==tch){
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
          gem_chamber_track[findIndex(tch.id())]->Fill(3.5);
          int index = findIndex(tch.id());
          double vfat = findvfat(tlp.x(), min_x, max_x);
          gem_chamber_th2D_eff[index]->Fill(vfat, mRoll);                
          gem_chamber_thxroll_eff[index]->Fill(tlp.x(), mRoll);
          gem_chamber_thxy_eff[index]->Fill(tlp.x(), gtrp.z());
          double maxR = 99.9;
          shared_ptr<MuonTransientTrackingRecHit> tmpRecHit;
          for (auto hit : testRecHits){
            GEMDetId hitID(hit->rawId());
            if (hitID.chamberId() != tch.id()) continue;
            GlobalPoint hitGP = hit->globalPosition();
            if (abs(hitGP.x() - gtrp.x()) > maxRes) continue;
            if ((hitID.roll() - mRoll)>1) continue;
            double deltaR = (hitGP - gtrp).mag();
            if (maxR > deltaR){
              tmpRecHit = hit;
              maxR = deltaR;
            }
          }
          if(tmpRecHit){
            gem_chamber_track[findIndex(tch.id())]->Fill(4.5);
            LocalPoint hitLP = tmpRecHit->localPosition();
            gem_chamber_tr2D_eff[index]->Fill(vfat, mRoll);
            gem_chamber_trxroll_eff[index]->Fill(tlp.x(), mRoll);
            gem_chamber_trxy_eff[index]->Fill(tlp.x(), gtrp.z());
            gem_chamber_local_x[index]->Fill(tlp.x(), hitLP.x());
            gem_chamber_residual[index]->Fill(tlp.x(), hitLP.x() - tlp.x());
            rh3_chamber->Fill(index);
            //cout << "chamber " << index << ", Track x : " << tlp.x() <<", RecHit x : " << hitLP.x() << ", Roll : " << mRoll << endl;
          }
        }
        continue;
      }
    }
  }
}


