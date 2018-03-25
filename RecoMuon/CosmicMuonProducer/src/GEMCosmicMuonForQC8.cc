/** \class GEMCosmicMuonForQC8
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

#include "RecoMuon/CosmicMuonProducer/interface/HeaderForQC8.h"

//#include <TMath.h>
//#include <Math/Vector3D.h>


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
  //math::XYZVector crossPoint(double x,double y,double z, double x0,double y0,double z0, double px,double py,double pz);
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
  bool checkCrossSeeds = false;
  bool recHitArrangeOn = true;
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

/*
math::XYZVector GEMCosmicMuonForQC8::crossPoint(double x,double y,double z, double x0,double y0,double z0, double px,double py,double pz)
{
  cout<<"crossPoint"<<endl;
  math::XYZVector v(x,y,z);
  math::XYZVector v0(x0,y0,z0);
  math::XYZVector u(px,py,pz);
  u = u.Unit();

  //distance between (x,y,z) and 3D line
  double d = (v-v0).Cross(u).Mag2();
  math::XYZVector c = v0 + d*u;
  cout<<"CP x "<<c.x()<<endl;
  return c;
}
*/

void GEMCosmicMuonForQC8::produce(edm::Event& ev, const edm::EventSetup& setup)
{ 
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

  if (gemRecHits->size() <3)
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

    //int floor = nIdxTestCh%10;
    //if ( floor == 0 ) { nUpType = 2; nDnType = 3;}   
    //if ( 1 <= floor && floor <= 4 ) { nUpType = 4; nDnType = 1;}   
    //if ( 5 <= floor && floor <= 8 ) { nUpType = 2; nDnType = 3;}   
    //if ( floor == 9 ) { nUpType = 4; nDnType = 1;}   
    //cout<<"nIdxTestCh "<<nIdxTestCh<<", nUpType "<<nUpType<<", nDnType "<<nDnType<<endl;

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
          
          //GlobalPoint recHitGP = mgeom->idToDet((*rechit).rawId())->surface().toGlobal(rechit->localPosition());
          ////LocalPoint  recHitLP = rechit->localPosition();

          if ( g_vecChamType[ nIdxCh ] == nUpType ) {
            seedupRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));
            //cout<<"upnIdxCh "<<nIdxCh<<", GP : x "<<recHitGP.x()<<", y "<<recHitGP.y()<<", z "<<recHitGP.z()<<endl;
            ////cout<<" LP : x "<<recHitLP.x()<<", y "<<recHitLP.y()<<endl;
          } else if ( g_vecChamType[ nIdxCh ] == nDnType ) {
            seeddnRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));
            //cout<<"dnnIdxCh "<<nIdxCh<<", GP : x "<<recHitGP.x()<<", y "<<recHitGP.y()<<", z "<<recHitGP.z()<<endl;
            ////cout<<" LP : x "<<recHitLP.x()<<", y "<<recHitLP.y()<<endl;
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
    int seedIdx[2] = {0,};

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
        TrajectorySeed::range range = seed.recHits();
        int nseed = 0;
        for (edm::OwnVector<TrackingRecHit>::const_iterator rechit = range.first; rechit!=range.second; ++rechit)
        {
          GEMDetId hitID(rechit->rawId());
          seedIdx[nseed] = hitID.chamber()+hitID.layer()-2;
          //GlobalPoint recHitGP = mgeom->idToDet((*rechit).rawId())->surface().toGlobal(rechit->localPosition());
          ////LocalPoint  recHitLP = rechit->localPosition();
          //cout<<"seedIdx "<<seedIdx<<", GP : x "<<recHitGP.x()<<", y "<<recHitGP.y()<<", z "<<recHitGP.z()<<" / ";
          ////cout<<" / LP : x "<<recHitLP.x()<<", y "<<recHitLP.y()<<" / ";
          nseed++;
        }
        if(checkCrossSeeds && nIdxTestCh==14 && ((seedIdx[0]==20 && seedIdx[1]==9) || (seedIdx[0]==0 && seedIdx[1]==29)) )
        {
          Trajectory::RecHitContainer smoothedHits = smoothed.recHits();
          cout<<"nIdxTestCh "<<nIdxTestCh<<", ntrajSeeds "<<countTR<<", seedIdx "<<seedIdx[0]<<" "<<seedIdx[1]<<", Nchi2 "<<dProbChiNDF<<", nmu WO tch "<<muRecHits.size()<<", nrecHits of trajectory "<<smoothedHits.size()<<endl;
          int nhit=0;
          for (Trajectory::RecHitContainer::const_iterator recHit = smoothedHits.begin(); recHit != smoothedHits.end(); ++recHit)
          {
            GEMDetId hitID((*recHit)->rawId());
            int chIdx = hitID.chamber()+hitID.layer()-2;
            GlobalPoint recHitGP = (*recHit)->globalPosition();
            cout<<"ntrajSeeds "<<countTR<<", nhit "<<nhit<<", chIdx "<<chIdx<<", GP : x "<<recHitGP.x()<<", y "<<recHitGP.y()<<", z "<<recHitGP.z()<<endl;
            //cout<<"chIdx "<<chIdx<<", GP : x "<<recHitGP.x()<<", y "<<recHitGP.y()<<", z "<<recHitGP.z()<<endl;
            nhit++;
          }
        }
      }
    }

    //TrajectorySeed::range range = bestSeed.recHits();
    //for (edm::OwnVector<TrackingRecHit>::const_iterator rechit = range.first; rechit!=range.second; ++rechit){
    //  GEMDetId hitID(rechit->rawId());
    //  int bestSeedIdx = hitID.chamber()+hitID.layer()-2;
    //  cout<<"bestSeedIdx "<<bestSeedIdx<<endl;
    //}
    
    if (!bestTrajectory.isValid()) continue;
    if (maxChi2 > trackChi2) continue;
    //trackChi2 = 3
    
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

      //GEMDetId hitID((*recHit)->rawId());
      //int chIdx = hitID.chamber()+hitID.layer()-2;
      //GlobalPoint recHitGP = (*recHit)->globalPosition();
      //cout<<"chIdx "<<chIdx<<", GP : x "<<recHitGP.x()<<", y "<<recHitGP.y()<<", z "<<recHitGP.z()<<endl;
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
      if (hit1->globalPosition().y() < hit2->globalPosition().y())
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
        //TrajectorySeed seed(seedTSOS,seedHits,oppositeToMomentum);
        
        uint32_t unInfoSeeds = 0;
        
        GEMDetId detId1(hit1->rawId()), detId2(hit2->rawId());
        uint32_t unChNo1 = detId1.chamber()+detId1.layer()-1;
        uint32_t unChNo2 = detId2.chamber()+detId2.layer()-1;
        
        uint32_t unRoll1 = detId1.roll(), unRoll2 = detId2.roll();
        uint32_t unDiffRoll = (uint32_t)abs(( (int32_t)unRoll1 ) - ( (int32_t)unRoll2 ));
        
        uint32_t unCol1 = ( unChNo1 - 1 ) / 10, unCol2 = ( unChNo2 - 1 ) / 10;
        uint32_t unDiffCol = (uint32_t)abs(( (int32_t)unCol1 ) - ( (int32_t)unCol2 ));
        
        //cout<<"unChNo1 "<<unChNo1-1<<", unChNo2 "<<unChNo2-1<<endl;
        //if ( unDiffCol != 0 ) continue;
        
        unInfoSeeds |= ( unDiffCol  ) << QC8FLAG_SEEDINFO_SHIFT_DIFFCOL;
        unInfoSeeds |= ( unDiffRoll ) << QC8FLAG_SEEDINFO_SHIFT_DIFFROLL;
        
        uint32_t unIsForRef = ( g_vecChamType[ unChNo1 - 1 ] == 3 || g_vecChamType[ unChNo2 - 1 ] == 4 ? 1 : 0 );
        
        if ( unIsForRef == 1 && ( ( unRoll1 == 1 && unRoll2 == 1 ) || ( unRoll1 == 8 && unRoll2 == 8 ) ) )
        {
          unInfoSeeds |= QC8FLAG_SEEDINFO_MASK_REFVERTROLL18;
        }
        
        if ( (g_SuperChamType[ int((unChNo1-1)/2) ] == "L" && unRoll1 == 8) ||
        	 (g_SuperChamType[ int((unChNo2-1)/2) ] == "L" && unRoll2 == 8) )
        {
          uint32_t seedingEight = 1;
          unInfoSeeds |= ( seedingEight ) << QC8FLAG_SEEDINFO_SHIFT_TOPBOTTOMETA8;
        }
        
        if ( (g_SuperChamType[ int((unChNo1-1)/2) ] == "L" && unRoll1 == 7) &&
        	 (g_SuperChamType[ int((unChNo2-1)/2) ] == "L" && unRoll2 == 7) )
        {
          uint32_t seedingSeven = 1;
          unInfoSeeds |= ( seedingSeven ) << QC8FLAG_SEEDINFO_SHIFT_TOPBOTTOMETA7;
        }
        
        tmptrajectorySeeds->push_back(seed);
        vecunInfoSeeds.push_back(unInfoSeeds);

        //cout<<"unChNo1 "<<unChNo1-1<<", GP : x "<<hit1->globalPosition().x()<<", y "<<hit1->globalPosition().y()<<", z "<<hit1->globalPosition().z()<<endl;
        //cout<<"unChNo2 "<<unChNo2-1<<", GP : x "<<hit2->globalPosition().x()<<", y "<<hit2->globalPosition().y()<<", z "<<hit2->globalPosition().z()<<endl;
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

  //cout<<"trackResX "<<trackResX<<", trackResY "<<trackResY<<", MulSigmaOnWindow "<<MulSigmaOnWindow<<endl;
  int nIdxTestCh  = testChamber.id().chamber() + testChamber.id().layer() - 2;
  TrajectorySeed::range range = seed.recHits();
  int seedIdx[2] = {0,};
  int nseed = 0;
  GlobalPoint seedGP[2];
  for (edm::OwnVector<TrackingRecHit>::const_iterator rechit = range.first; rechit!=range.second; ++rechit){
    GEMDetId hitID(rechit->rawId());
    seedIdx[nseed] = hitID.chamber()+hitID.layer()-2;
    seedGP[nseed] = gemGeom->idToDet((*rechit).rawId())->surface().toGlobal(rechit->localPosition());
    nseed++;
  }
  bool crossSeeds = false;
  if( nIdxTestCh==14 && ((seedIdx[0]==20 && seedIdx[1]==9) || (seedIdx[0]==0 && seedIdx[1]==29)) )
    crossSeeds = true;
    //cout<<"nIdxTestCh "<<nIdxTestCh<<", seedIdx "<<seedIdx[0]<<" "<<seedIdx[1]<<", Nchi2 "<<dProbChiNDF<<", nmu WO tch "<<muRecHits.size()<<endl;
  std::map<double,int> rAndhit;

  for (auto ch : gemChambers)
  {
    std::shared_ptr<MuonTransientTrackingRecHit> tmpRecHit;
    tsosCurrent = theService->propagator("SteppingHelixPropagatorAny")->propagate(tsosCurrent, theService->trackingGeometry()->idToDet(ch.id())->surface());
    if (!tsosCurrent.isValid()) return Trajectory();
    GlobalPoint tsosGP = tsosCurrent.freeTrajectoryState()->position();
    //cout<<"chID  "<<ch.id().chamber()+ch.id().layer()-2<<", tsosGP : x "<<tsosGP.x()<<", y "<<tsosGP.y()<<", z "<<tsosGP.z()<<endl;
    if(checkCrossSeeds && crossSeeds) cout<<"chID  "<<ch.id().chamber()+ch.id().layer()-2<<", trajHit :	x "<<tsosGP.x()<<", y "<<tsosGP.y()<<", z "<<tsosGP.z()<<endl;

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
        //cout<<"hitID "<<hitID.chamber()+hitID.layer()-2<<", x "<<hitGP.x()<<", y "<<hitGP.y()<<", z "<<hitGP.z()<<" +- "<<y_err<<endl;
        if(checkCrossSeeds && crossSeeds) cout<<"hitID "<<hitID.chamber()+hitID.layer()-2<<", recHit :	x "<<hitGP.x()<<", y "<<hitGP.y()<<", z "<<hitGP.z()<<" +- "<<y_err<<endl;
        if (fabs(hitGP.x() - tsosGP.x()) > trackResX * MulSigmaOnWindow) continue;
        if (fabs(hitGP.z() - tsosGP.z()) > trackResY * MulSigmaOnWindow * y_err) continue; // global z, local y
        float deltaR = (hitGP - tsosGP).mag();
        if (maxR > deltaR)
        {
          tmpRecHit = hit;
          maxR = deltaR;
          tmpNhit = nhit;
          tmpR = (hitGP - seedGP[0]).mag();
          //tmpY = hitGP.y();
        }
      }
    }
    if (tmpRecHit)
    {
      if(!recHitArrangeOn) consRecHits.push_back(tmpRecHit);
      rAndhit[tmpR] = tmpNhit;
      //cout<<"addID "<<tmpRecHit->rawId().chamber()+tmpRecHit->rawId().layer()<<", recHit :	x "<<tmpRecHit->globalPosition().x()<<", y "<<tmpRecHit->globalPosition().y()<<", z "<<tmpRecHit->globalPosition().z()<<endl;
      //GEMDetId hitID(tmpRecHit->rawId());
      //if(crossSeeds) cout<<"addID "<<hitID.chamber()+hitID.layer()-2<<", recHit :	x "<<tmpRecHit->globalPosition().x()<<", y "<<tmpRecHit->globalPosition().y()<<", z "<<tmpRecHit->globalPosition().z()<<endl;
    }
  }

  vector<pair<double,int>> rAndhitV;
  copy(rAndhit.begin(), rAndhit.end(), back_inserter<vector<pair<double,int>>>(rAndhitV));
  for(unsigned int i=0;i<rAndhitV.size();i++)
  {
    if(checkCrossSeeds && crossSeeds) cout<<"i "<<i<<", r "<<rAndhitV[i].first<<", nhit "<<rAndhitV[i].second<<endl;
    if(recHitArrangeOn) consRecHits.push_back(muRecHits[rAndhitV[i].second]);
  }

  if (consRecHits.size() <3) return Trajectory();
  vector<Trajectory> fitted = theSmoother->trajectories(seed, consRecHits, tsos);
  if ( fitted.size() <= 0 ) return Trajectory();
  
  Trajectory smoothed = fitted.front();
  float dProbChiNDF = smoothed.chiSquared()/float(smoothed.ndof());
  if(checkCrossSeeds && crossSeeds) cout<<"Nchi2 "<<dProbChiNDF<<", chi2 "<<smoothed.chiSquared()<<", ndof "<<smoothed.ndof()<<endl;;
  return fitted.front();
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GEMCosmicMuonForQC8);
