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

#include "RecoMuon/CosmicMuonProducer/interface/HeaderForQC8.h"

using namespace std;

class GEMCosmicMuonForQC8 : public edm::stream::EDProducer<> {
public:
  /// Constructor
  explicit GEMCosmicMuonForQC8(const edm::ParameterSet&);
  /// Destructor
  virtual ~GEMCosmicMuonForQC8() {}
  /// Produce the GEMSegment collection
  void produce(edm::Event&, const edm::EventSetup&) override;
  double maxCLS;
  double minCLS;
  double trackChi2, trackResX, trackResY;
  double MulSigmaOnWindow;
  std::vector<std::string> g_SuperChamType;
  vector<double> g_vecChamType;
private:
  int iev; // events through
  edm::EDGetTokenT<GEMRecHitCollection> theGEMRecHitToken;
  CosmicMuonSmoother* theSmoother;
  MuonServiceProxy* theService;
  KFUpdator* theUpdator;
  int findSeeds(std::vector<TrajectorySeed> *tmptrajectorySeeds, 
    MuonTransientTrackingRecHit::MuonRecHitContainer &seedupRecHits, 
    MuonTransientTrackingRecHit::MuonRecHitContainer &seeddnRecHits, 
    std::vector<unsigned int> &vecunInfoSeeds);
  Trajectory makeTrajectory(TrajectorySeed seed, MuonTransientTrackingRecHit::MuonRecHitContainer &muRecHits, std::vector<GEMChamber> gemChambers, GEMChamber testChamber);
  const GEMGeometry* gemGeom;
  int nev;
  bool checkCrossSeeds = false;
};

GEMCosmicMuonForQC8::GEMCosmicMuonForQC8(const edm::ParameterSet& ps) : iev(0) {
  time_t rawTime;
  time(&rawTime);
  printf("Begin of GEMCosmicMuonForQC8::GEMCosmicMuonForQC8() at %s\n", asctime(localtime(&rawTime)));
  maxCLS = ps.getParameter<double>("maxClusterSize");
  minCLS = ps.getParameter<double>("minClusterSize");
  trackChi2 = ps.getParameter<double>("trackChi2");
  trackResX = ps.getParameter<double>("trackResX");
  trackResY = ps.getParameter<double>("trackResY");
  MulSigmaOnWindow = ps.getParameter<double>("MulSigmaOnWindow");
  g_SuperChamType = ps.getParameter<vector<string>>("SuperChamberType");
  g_vecChamType = ps.getParameter<vector<double>>("SuperChamberSeedingLayers");
  theGEMRecHitToken = consumes<GEMRecHitCollection>(ps.getParameter<edm::InputTag>("gemRecHitLabel"));
  // register what this produces
  edm::ParameterSet serviceParameters = ps.getParameter<edm::ParameterSet>("ServiceParameters");
  theService = new MuonServiceProxy(serviceParameters);
  edm::ParameterSet smootherPSet = ps.getParameter<edm::ParameterSet>("MuonSmootherParameters");
  theSmoother = new CosmicMuonSmoother(smootherPSet,theService);
  theUpdator = new KFUpdator();
  produces<reco::TrackCollection>();
  produces<TrackingRecHitCollection>();
  produces<reco::TrackExtraCollection>();
  produces<vector<Trajectory> >();
  produces<vector<TrajectorySeed> >();
  produces<vector<int> >();
  produces<vector<unsigned int> >();
  printf("End of GEMCosmicMuonForQC8::GEMCosmicMuonForQC8() at %s\n", asctime(localtime(&rawTime)));
}


void GEMCosmicMuonForQC8::produce(edm::Event& ev, const edm::EventSetup& setup)
{
  nev = ev.id().event();

  unique_ptr<reco::TrackCollection >          trackCollection( new reco::TrackCollection() );
  unique_ptr<TrackingRecHitCollection >       trackingRecHitCollection( new TrackingRecHitCollection() );
  unique_ptr<reco::TrackExtraCollection >     trackExtraCollection( new reco::TrackExtraCollection() );
  unique_ptr<vector<Trajectory> >             trajectorys( new vector<Trajectory>() );
  unique_ptr<vector<TrajectorySeed> >         trajectorySeeds( new vector<TrajectorySeed>() );
  unique_ptr<vector<int> >                    trajectoryChIdx( new vector<int>() );
  unique_ptr<vector<unsigned int> >           trajectoryType( new vector<unsigned int>() );
  unique_ptr<vector<double> >                 trajectorySeedsHits( new vector<double>() );
  TrackingRecHitRef::key_type recHitsIndex = 0;
  TrackingRecHitRefProd recHitCollectionRefProd = ev.getRefBeforePut<TrackingRecHitCollection>();
  reco::TrackExtraRef::key_type trackExtraIndex = 0;
  reco::TrackExtraRefProd trackExtraCollectionRefProd = ev.getRefBeforePut<reco::TrackExtraCollection>();
  
  theService->update(setup);

  edm::ESHandle<GEMGeometry> gemg;
  setup.get<MuonGeometryRecord>().get(gemg);
  const GEMGeometry* mgeom = &*gemg;
  gemGeom = &*gemg;
  
  vector<GEMChamber> gemChambers;
  
  const std::vector<const GEMSuperChamber*>& superChambers_ = mgeom->superChambers();   
  for (auto sch : superChambers_)
  {
	int n_lay = sch->nChambers();
    for (int l=0;l<n_lay;l++)
   	{
	  gemChambers.push_back(*sch->chamber(l+1));
    }
  }

  // get the collection of GEMRecHit
  edm::Handle<GEMRecHitCollection> gemRecHits;
  ev.getByToken(theGEMRecHitToken,gemRecHits);

  if (gemRecHits->size() <= 3)
  {
    ev.put(std::move(trajectorySeeds));
    ev.put(std::move(trackCollection));
    ev.put(std::move(trackingRecHitCollection));
    ev.put(std::move(trackExtraCollection));
    ev.put(std::move(trajectorys));
    ev.put(std::move(trajectoryChIdx));
    ev.put(std::move(trajectoryType));
    return;
  }
  
  int countTC = 0;
    
  for (auto tch : gemChambers)
  {
    countTC++;
    MuonTransientTrackingRecHit::MuonRecHitContainer muRecHits;
    MuonTransientTrackingRecHit::MuonRecHitContainer seedupRecHits;
    MuonTransientTrackingRecHit::MuonRecHitContainer seeddnRecHits;
    
    int nUpType = 2;
    int nDnType = 1;
    
    int nIdxTestCh = tch.id().chamber() + tch.id().layer() - 2; // (tch.id.chamber - 1) + (tch.id.layer - 1) -> array numbering starts from 0 and not 1
    
    if ( g_vecChamType[ nIdxTestCh ] == 2 ) {nUpType = 4;}
    if ( g_vecChamType[ nIdxTestCh ] == 1 ) {nDnType = 3;}
    
    int TCN = 0; //number of hitted chamber without tch
    for (auto ch : gemChambers)
    {
      if (tch == ch) continue;
      int nHitOnceFilter = 0;
      for (auto etaPart : ch.etaPartitions())
      {
        GEMDetId etaPartID = etaPart->id();
        GEMRecHitCollection::range range = gemRecHits->get(etaPartID);   

        for (GEMRecHitCollection::const_iterator rechit = range.first; rechit!=range.second; ++rechit)
        {
          const GeomDet* geomDet(etaPart);
          if ((*rechit).clusterSize()<minCLS) continue;
          if ((*rechit).clusterSize()>maxCLS) continue;
          muRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));
          
          if ( nHitOnceFilter == 0 ) {
            TCN++;
            nHitOnceFilter = 1;
          }
          
          int nIdxCh  = ch.id().chamber() + ch.id().layer() - 2;

          if ( g_vecChamType[ nIdxCh ] == nUpType ) {
            seedupRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));
          } else if ( g_vecChamType[ nIdxCh ] == nDnType ) {
            seeddnRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));
          }
        }
      }
    }
    if (muRecHits.size()<3) continue;
    if (TCN < 3) continue;

    vector<TrajectorySeed> trajSeedsBody;
    std::vector<TrajectorySeed> *trajSeeds = &trajSeedsBody;
    std::vector<uint32_t> vecunInfoSeeds;
    findSeeds(trajSeeds, seedupRecHits, seeddnRecHits, vecunInfoSeeds);
    Trajectory bestTrajectory;
    TrajectorySeed bestSeed;

    float maxChi2 = 10000000.0;
    int countTR = 0;
    int nIdxBest = -1;

    for (auto seed : *trajSeeds)
    {
      Trajectory smoothed = makeTrajectory(seed, muRecHits, gemChambers,tch);
      
      countTR += 1;
      
      if (smoothed.isValid())
      {
        float dProbChiNDF = smoothed.chiSquared()/float(smoothed.ndof());
        
        if (fabs(maxChi2-1) > fabs(dProbChiNDF-1))
        {
          maxChi2 = dProbChiNDF;
          bestTrajectory = smoothed;
          bestSeed = seed;
          nIdxBest = countTR - 1;
        }
      }
    }

    if (!bestTrajectory.isValid()) continue;
    if (maxChi2 > trackChi2) continue;
    
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
   
    reco::TrackExtra tx;
    
    //adding rec hits
    Trajectory::RecHitContainer transHits = bestTrajectory.recHits();
    unsigned int nHitsAdded = 0;
    for (Trajectory::RecHitContainer::const_iterator recHit = transHits.begin(); recHit != transHits.end(); ++recHit)
    {
      TrackingRecHit *singleHit = (**recHit).hit()->clone();
      trackingRecHitCollection->push_back( singleHit );  
      ++nHitsAdded;
    }
    
    tx.setHits(recHitCollectionRefProd, recHitsIndex, nHitsAdded);
    recHitsIndex += nHitsAdded;
    
    trackExtraCollection->push_back(tx);
    
    reco::TrackExtraRef trackExtraRef(trackExtraCollectionRefProd, trackExtraIndex++ );
    track.setExtra(trackExtraRef);
    trackCollection->push_back(track);
    
    trajectorys->push_back(bestTrajectory);
    trajectorySeeds->push_back(bestSeed);
    trajectoryChIdx->push_back(countTC);
    trajectoryType->push_back(vecunInfoSeeds[ nIdxBest ]);
  }
  
  // fill the collection
  // put collection in event
  ev.put(std::move(trajectorySeeds));
  ev.put(std::move(trackCollection));
  ev.put(std::move(trackingRecHitCollection));
  ev.put(std::move(trackExtraCollection));
  ev.put(std::move(trajectorys));
  ev.put(std::move(trajectoryChIdx));
  ev.put(std::move(trajectoryType));
  
}


int GEMCosmicMuonForQC8::findSeeds(std::vector<TrajectorySeed> *tmptrajectorySeeds, 
    MuonTransientTrackingRecHit::MuonRecHitContainer &seedupRecHits, 
    MuonTransientTrackingRecHit::MuonRecHitContainer &seeddnRecHits, 
    std::vector<unsigned int> &vecunInfoSeeds)
{
  for (auto hit1 : seeddnRecHits){
    for (auto hit2 : seedupRecHits){
      if (hit1->globalPosition().z() < hit2->globalPosition().z())
      {
        LocalPoint segPos = hit1->localPosition();
        GlobalVector segDirGV(hit2->globalPosition().x() - hit1->globalPosition().x(),
                              hit2->globalPosition().y() - hit1->globalPosition().y(),
                              hit2->globalPosition().z() - hit1->globalPosition().z());

        segDirGV *=10;
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
        
        uint32_t unInfoSeeds = 0;
        
        GEMDetId detId1(hit1->rawId()), detId2(hit2->rawId());
        uint32_t unChNo1 = detId1.chamber()+detId1.layer()-1;
        uint32_t unChNo2 = detId2.chamber()+detId2.layer()-1;
        
        uint32_t unRoll1 = detId1.roll(), unRoll2 = detId2.roll();
        
        uint32_t unCol1 = ( unChNo1 - 1 ) / 10, unCol2 = ( unChNo2 - 1 ) / 10;
        uint32_t unDiffCol = (uint32_t)abs(( (int32_t)unCol1 ) - ( (int32_t)unCol2 ));
        
        unInfoSeeds |= ( unDiffCol  ) << QC8FLAG_SEEDINFO_SHIFT_DIFFCOL;
        
        uint32_t unIsForRef = ( g_vecChamType[ unChNo1 - 1 ] == 3 || g_vecChamType[ unChNo2 - 1 ] == 4 ? 1 : 0 );
        
        if ( unIsForRef == 1 && ( ( unRoll1 == 1 && unRoll2 == 1 ) || ( unRoll1 == 8 && unRoll2 == 8 ) ) )
        {
          unInfoSeeds |= QC8FLAG_SEEDINFO_MASK_REFVERTROLL18;
        }
        
        tmptrajectorySeeds->push_back(seed);
        vecunInfoSeeds.push_back(unInfoSeeds);
      }
    }
  }
  
  return 0;
}


Trajectory GEMCosmicMuonForQC8::makeTrajectory(TrajectorySeed seed, MuonTransientTrackingRecHit::MuonRecHitContainer &muRecHits, std::vector<GEMChamber> gemChambers, GEMChamber testChamber)
{
  PTrajectoryStateOnDet ptsd1(seed.startingState());
  DetId did(ptsd1.detId());
  const BoundPlane& bp = theService->trackingGeometry()->idToDet(did)->surface();
  TrajectoryStateOnSurface tsos = trajectoryStateTransform::transientState(ptsd1,&bp,&*theService->magneticField());
  TrajectoryStateOnSurface tsosCurrent = tsos;
  TransientTrackingRecHit::ConstRecHitContainer consRecHits;

  TrajectorySeed::range range = seed.recHits();
  int nseed = 0;
  GlobalPoint seedGP[2];
  for (edm::OwnVector<TrackingRecHit>::const_iterator rechit = range.first; rechit!=range.second; ++rechit){
    GEMDetId hitID(rechit->rawId());
    seedGP[nseed] = gemGeom->idToDet((*rechit).rawId())->surface().toGlobal(rechit->localPosition());
    nseed++;
  }
  std::map<double,int> rAndhit;

  for (auto ch : gemChambers)
  {
    std::shared_ptr<MuonTransientTrackingRecHit> tmpRecHit;
    tsosCurrent = theService->propagator("SteppingHelixPropagatorAny")->propagate(tsosCurrent, theService->trackingGeometry()->idToDet(ch.id())->surface());
    if (!tsosCurrent.isValid()) return Trajectory();
    GlobalPoint tsosGP = tsosCurrent.freeTrajectoryState()->position();

    float maxR = 9999;
    int nhit=-1;
    int tmpNhit=-1;
    double tmpR=-1;
    for (auto hit : muRecHits)
    {
      nhit++;
      GEMDetId hitID(hit->rawId());
      if (hitID.chamberId() == ch.id() )
      {
        GlobalPoint hitGP = hit->globalPosition();
        double y_err = hit->localPositionError().yy();
        if (fabs(hitGP.x() - tsosGP.x()) > trackResX * MulSigmaOnWindow) continue;
        if (fabs(hitGP.y() - tsosGP.y()) > trackResY * MulSigmaOnWindow * y_err) continue; // global y, local y
        float deltaR = (hitGP - tsosGP).mag();
        if (maxR > deltaR)
        {
          tmpRecHit = hit;
          maxR = deltaR;
          tmpNhit = nhit;
          tmpR = (hitGP - seedGP[0]).mag();
        }
      }
    }
    if (tmpRecHit)
    {
      rAndhit[tmpR] = tmpNhit;
    }
  }

  if (rAndhit.size() < 3) return Trajectory();
  vector<pair<double,int>> rAndhitV;
  copy(rAndhit.begin(), rAndhit.end(), back_inserter<vector<pair<double,int>>>(rAndhitV));
  for(unsigned int i=0;i<rAndhitV.size();i++)
  {
    consRecHits.push_back(muRecHits[rAndhitV[i].second]);
  }

  if (consRecHits.size() <3) return Trajectory();
  vector<Trajectory> fitted = theSmoother->trajectories(seed, consRecHits, tsos);
  if ( fitted.size() <= 0 ) return Trajectory();
  
  Trajectory smoothed = fitted.front();
  return fitted.front();
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GEMCosmicMuonForQC8);
