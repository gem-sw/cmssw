/** \class GEMCosmicMuon  
 * Produces a collection of tracks's in GEM cosmic ray stand. 
 *
 * \author Jason Lee
 */

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include <Geometry/Records/interface/MuonGeometryRecord.h>
#include <Geometry/GEMGeometry/interface/GEMGeometry.h>

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrajectorySeed/interface/PropagationDirection.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeed.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "RecoMuon/TransientTrackingRecHit/interface/MuonTransientTrackingRecHit.h"
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"
#include "RecoMuon/CosmicMuonProducer/interface/CosmicMuonSmoother.h"
#include "RecoMuon/StandAloneTrackFinder/interface/StandAloneMuonSmoother.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/KalmanUpdators/interface/KFUpdator.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"

using namespace std;
//CAS ADD MuonRecHitContainerLayered
class MuonRecHitContainerLayered : public MuonTransientTrackingRecHit::MuonRecHitContainer {
 protected:
  
 public:
  std::vector<int> Layers;
  MuonRecHitContainerLayered( MuonTransientTrackingRecHit::MuonRecHitContainer &rechit, std::vector<int> _Layers );
  ~MuonRecHitContainerLayered( void );
  int GetLayer( unsigned int _L );
  // MuonTransientTrackingRecHit::MuonRecHitContaine& GetmuRecHits(void);
};

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

class GEMCosmicMuon : public edm::stream::EDProducer<> {
public:
  /// Constructor
  explicit GEMCosmicMuon(const edm::ParameterSet&);
  /// Destructor
  virtual ~GEMCosmicMuon() {}
  /// Produce the GEMSegment collection
  void produce(edm::Event&, const edm::EventSetup&) override;
  double maxCLS;
  double minCLS;
  double trackChi2, trackResX, trackResY;
private:
  int iev; // events through
  edm::EDGetTokenT<GEMRecHitCollection> theGEMRecHitToken;
  CosmicMuonSmoother* theSmoother;
  MuonServiceProxy* theService;
  KFUpdator* theUpdator;
  std::vector<int> refChambers;
  //CAS MuonRecHitContainerLayered &muRecHits
  unique_ptr<std::vector<TrajectorySeed> > findSeeds(MuonRecHitContainerLayered &muRecHits);
  Trajectory makeTrajectory(TrajectorySeed seed, MuonTransientTrackingRecHit::MuonRecHitContainer &muRecHits, vector<const GEMChamber*> gemChambers);
};

GEMCosmicMuon::GEMCosmicMuon(const edm::ParameterSet& ps) : iev(0) {
  maxCLS = ps.getParameter<double>("maxClusterSize");
  minCLS = ps.getParameter<double>("minClusterSize");
  trackChi2 = ps.getParameter<double>("trackChi2");
  trackResX = ps.getParameter<double>("trackResX");
  trackResY = ps.getParameter<double>("trackResY");
  theGEMRecHitToken = consumes<GEMRecHitCollection>(ps.getParameter<edm::InputTag>("gemRecHitLabel"));
  // register what this produces
  edm::ParameterSet serviceParameters = ps.getParameter<edm::ParameterSet>("ServiceParameters");
  theService = new MuonServiceProxy(serviceParameters);
  edm::ParameterSet smootherPSet = ps.getParameter<edm::ParameterSet>("MuonSmootherParameters");
  theSmoother = new CosmicMuonSmoother(smootherPSet,theService);
  theUpdator = new KFUpdator();
  //CAS Add refChambers 
  refChambers = ps.getParameter<std::vector<int>>("refChambers");//Reference chambers
  produces<reco::TrackCollection>();
  produces<TrackingRecHitCollection>();
  produces<reco::TrackExtraCollection>();
  produces<vector<Trajectory> >();
  produces<vector<TrajectorySeed> >();
}

void GEMCosmicMuon::produce(edm::Event& ev, const edm::EventSetup& setup) {
  //  cout << "GEMCosmicMuon::start producing segments for " << ++iev << "th event with gem data" << endl;
  
  unique_ptr<reco::TrackCollection >          trackCollection( new reco::TrackCollection() );
  unique_ptr<TrackingRecHitCollection >       trackingRecHitCollection( new TrackingRecHitCollection() );
  unique_ptr<reco::TrackExtraCollection >     trackExtraCollection( new reco::TrackExtraCollection() );
  unique_ptr<vector<Trajectory> >             trajectorys( new vector<Trajectory>() );
  unique_ptr<vector<TrajectorySeed> >         trajectorySeeds( new vector<TrajectorySeed>() );
  TrackingRecHitRef::key_type recHitsIndex = 0;
  TrackingRecHitRefProd recHitCollectionRefProd = ev.getRefBeforePut<TrackingRecHitCollection>();
  reco::TrackExtraRef::key_type trackExtraIndex = 0;
  reco::TrackExtraRefProd trackExtraCollectionRefProd = ev.getRefBeforePut<reco::TrackExtraCollection>();
  
  theService->update(setup);

  edm::ESHandle<GEMGeometry> gemg;
  setup.get<MuonGeometryRecord>().get(gemg);
  const GEMGeometry* mgeom = &*gemg;

  //vector<const DetLayer*> gemLayers = theMuonLayers->allDTLayers();
  
  // get the collection of GEMRecHit
  edm::Handle<GEMRecHitCollection> gemRecHits;
  ev.getByToken(theGEMRecHitToken,gemRecHits);

  if (gemRecHits->size() <3){
    ev.put(std::move(trajectorySeeds));
    ev.put(std::move(trackCollection));
    ev.put(std::move(trackingRecHitCollection));
    ev.put(std::move(trackExtraCollection));
    ev.put(std::move(trajectorys));
    return;
  }
  
  //  cout << "GEMCosmicMuon::gemRecHits " << gemRecHits->size() << endl;
  
  MuonTransientTrackingRecHit::MuonRecHitContainer muRecHits;
  vector<const GEMChamber*> gemChambers;
    
  //for (auto ch : chambers){
  // Create the chamber Id
  for (int i =1; i < 31; i++){
    for (int j=1; j < 3; j++){
      GEMDetId chamberID(1,1,1, j, i, 0);
      const GEMChamber* ch = mgeom->chamber(chamberID);
      if (!ch) continue;
      //gemChambers.push_back(theService->detLayerGeometry()->idToLayer( chamberID.rawId() ));
      gemChambers.push_back(ch);
      for (auto etaPart : ch->etaPartitions()){
	GEMDetId etaPartID = etaPart->id();
	// Get the GEM-Segment which relies on this chamber
	GEMRecHitCollection::range range = gemRecHits->get(etaPartID);

	//cout<< "Number of GEM rechits available , from chamber: "<< etaPartID<<endl;
	// Create the MuonTransientTrackingRecHit
	for (GEMRecHitCollection::const_iterator rechit = range.first; rechit!=range.second; ++rechit){
	  const GeomDet* geomDet(etaPart);	
          if ((*rechit).clusterSize()<minCLS) continue;
          if ((*rechit).clusterSize()>maxCLS) continue;
	  muRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));

	  //GEMRecHit *newRH = rechit->clone();
	  //TransientTrackingRecHit::ConstRecHitPointer trkRecHit(newRH);
	  //cout << "GEMCosmicMuon::hit " << muRecHits.back()->globalPosition() << endl;
	}
      }
    }
  }
  // cout<< "Number of GEM rechits   = " << muRecHits.size()<<endl;
  /////////////////////////////////////////////////////////////////////////////////////
  //Changed to use the new class CAS
  /////////////////////////////////////////////////////////////////////////////////////
  //std::vector<unsigned> Layers {, true, false};//temporal 
  MuonRecHitContainerLayered muRecHitsl(muRecHits,refChambers);
  trajectorySeeds =findSeeds(muRecHitsl);
  /////////////////////////////////////////////////////////////////////////////////////
  //  cout << "GEMCosmicMuon::trajectorySeeds->size() " << trajectorySeeds->size() << endl;
  
  // need to loop over seeds, make best track and save only best track
  //TrajectorySeed seed =trajectorySeeds->at(0);
  Trajectory bestTrajectory;
  TrajectorySeed bestSeed;
  float maxChi2 = trackChi2;
  for (auto seed : *trajectorySeeds){
    Trajectory smoothed = makeTrajectory(seed, muRecHits, gemChambers);
    if (smoothed.isValid()){
      trajectorys->push_back(smoothed);
      //      cout << "GEMCosmicMuon::Trajectory " << smoothed.foundHits() << endl;
      //      cout << "GEMCosmicMuon::Trajectory chiSquared/ ndof " << smoothed.chiSquared()/float(smoothed.ndof()) << endl;
      //if (( maxChi2 > smoothed.chiSquared()/float(smoothed.ndof())) and ( smoothed.chiSquared()/float(smoothed.ndof()) > 7.0)){
      if (maxChi2 > smoothed.chiSquared()/float(smoothed.ndof())){
	maxChi2 = smoothed.chiSquared()/float(smoothed.ndof());
	bestTrajectory = smoothed;
        bestSeed = seed;
      }
    }
  }
  if (!bestTrajectory.isValid()){
    ev.put(std::move(trajectorySeeds));
    ev.put(std::move(trackCollection));
    ev.put(std::move(trackingRecHitCollection));
    ev.put(std::move(trackExtraCollection));
    ev.put(std::move(trajectorys));
    return;
  }
  //cout << maxChi2 << endl;
  //cout << "GEMCosmicMuon::bestTrajectory " << bestTrajectory.foundHits() << endl;
  //cout << "GEMCosmicMuon::bestTrajectory chiSquared/ ndof " << bestTrajectory.chiSquared()/float(bestTrajectory.ndof()) << endl;
  //cout << maxChi2 << endl;
  // make track
  const FreeTrajectoryState* ftsAtVtx = bestTrajectory.geometricalInnermostState().freeState();
   
  GlobalPoint pca = ftsAtVtx->position();
  math::XYZPoint persistentPCA(pca.x(),pca.y(),pca.z());
  GlobalVector p = ftsAtVtx->momentum();
  math::XYZVector persistentMomentum(p.x(),p.y(),p.z());
  
  reco::Track track(bestTrajectory.chiSquared(), 
		    bestTrajectory.ndof(true),
		    persistentPCA,
		    persistentMomentum,
		    ftsAtVtx->charge(),
		    ftsAtVtx->curvilinearError());
 
  // create empty collection of Segments
  //cout << "GEMCosmicMuon::track " << track.pt() << endl;
  
  // reco::TrackExtra tx(track.outerPosition(), track.outerMomentum(), track.outerOk(),
  // 		      track.innerPosition(), track.innerMomentum(), track.innerOk(),
  // 		      track.outerStateCovariance(), track.outerDetId(),
  // 		      track.innerStateCovariance(), track.innerDetId(),
  // 		      track.seedDirection(), edm::RefToBase<TrajectorySeed>());
  // 		      //, bestTrajectory.seedRef() );
  reco::TrackExtra tx;
  //tx.setResiduals(track.residuals());
  //adding rec hits
  Trajectory::RecHitContainer transHits = bestTrajectory.recHits();
  unsigned int nHitsAdded = 0;
  for (Trajectory::RecHitContainer::const_iterator recHit = transHits.begin(); recHit != transHits.end(); ++recHit) {
    TrackingRecHit *singleHit = (**recHit).hit()->clone();
    trackingRecHitCollection->push_back( singleHit );  
    ++nHitsAdded;
  }
  tx.setHits(recHitCollectionRefProd, recHitsIndex, nHitsAdded);
  recHitsIndex +=nHitsAdded;

  trackExtraCollection->push_back(tx );

  reco::TrackExtraRef trackExtraRef(trackExtraCollectionRefProd, trackExtraIndex++ );
  track.setExtra(trackExtraRef);
  trackCollection->push_back(track);
  
  
  // fill the collection
  // put collection in event
  trajectorySeeds->push_back(bestSeed);

  ev.put(std::move(trajectorySeeds));
  ev.put(std::move(trackCollection));
  ev.put(std::move(trackingRecHitCollection));
  ev.put(std::move(trackExtraCollection));
  ev.put(std::move(trajectorys));
  
}

unique_ptr<std::vector<TrajectorySeed> > GEMCosmicMuon::findSeeds(MuonRecHitContainerLayered &muRecHits)
{
  unique_ptr<std::vector<TrajectorySeed> > tmptrajectorySeeds( new vector<TrajectorySeed>());
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
Trajectory GEMCosmicMuon::makeTrajectory(TrajectorySeed seed, MuonTransientTrackingRecHit::MuonRecHitContainer &muRecHits, vector<const GEMChamber*> gemChambers)
{
  PTrajectoryStateOnDet ptsd1(seed.startingState());
  DetId did(ptsd1.detId());
  const BoundPlane& bp = theService->trackingGeometry()->idToDet(did)->surface();
  TrajectoryStateOnSurface tsos = trajectoryStateTransform::transientState(ptsd1,&bp,&*theService->magneticField());

  TrajectoryStateOnSurface tsosCurrent = tsos;
  
  TransientTrackingRecHit::ConstRecHitContainer consRecHits;
  for (auto ch : gemChambers){
    //const DetLayer* layer = theService->detLayerGeometry()->idToLayer( ch->id().rawId() );
    std::shared_ptr<MuonTransientTrackingRecHit> tmpRecHit;
    tsosCurrent = theService->propagator("SteppingHelixPropagatorAny")->propagate(tsosCurrent,ch->surface());
    if (!tsosCurrent.isValid()) return Trajectory();
    GlobalPoint tsosGP = tsosCurrent.freeTrajectoryState()->position();
    //TrackingRecHit *tmpRecHit = new TrackingRecHit(ch);
    //cout << "tsosGP "<< tsosGP <<endl;
    float maxR = 9999;
    for (auto hit : muRecHits){
      GEMDetId hitID(hit->rawId());
      if (hitID.chamberId() == ch->id() ){
	GlobalPoint hitGP = hit->globalPosition();
	// cut is deltaX is too big - can be tighter?
        //double x_err = hit->localPositionError().xx();
        double y_err = hit->localPositionError().yy();
	if (abs(hitGP.x() - tsosGP.x()) > trackResX) continue;
	//if (abs(hitGP.z() - tsosGP.z()) > abs(y_err)) continue;
	if (abs(hitGP.z() - tsosGP.z()) > y_err*trackResY) continue;
	// need to find best hits per chamber
	float deltaR = (hitGP - tsosGP).mag();
	if (maxR > deltaR){
	  tmpRecHit = hit;
	  maxR = deltaR;
	}
      }
    }
    
    if (tmpRecHit){
      //cout << "hit gp "<< tmpRecHit->globalPosition() <<endl;
      tsosCurrent = theUpdator->update(tsosCurrent, *tmpRecHit);
      consRecHits.push_back(tmpRecHit);
    }
  }
  if (consRecHits.size() <3) return Trajectory();
  vector<Trajectory> fitted = theSmoother->trajectories(seed, consRecHits, tsos);
  return fitted.front();
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GEMCosmicMuon);
