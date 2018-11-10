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

#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

using namespace std;

class AlignmentTrackRecoQC8 : public edm::stream::EDProducer<> {
public:
    /// Constructor
    explicit AlignmentTrackRecoQC8(const edm::ParameterSet&);
    /// Destructor
    virtual ~AlignmentTrackRecoQC8() {}
    /// Produce the GEMSegment collection
    void produce(edm::Event&, const edm::EventSetup&) override;
    double maxCLS;
    double minCLS;
    double trackChi2, trackResX, trackResY;
    double MulSigmaOnWindow;
    std::vector<std::string> g_SuperChamType;
    std::vector<double> shiftX;
    std::vector<double> rotationZ;
    std::vector<double> trueDx;
    std::vector<double> trueRz;
private:
    int iev; // events through
    edm::EDGetTokenT<GEMRecHitCollection> theGEMRecHitToken;
    edm::EDGetToken InputTagToken_US;
    CosmicMuonSmoother* theSmoother;
    MuonServiceProxy* theService;
    KFUpdator* theUpdator;
    const GEMGeometry* gemGeom;
    int nev;
    int findSeeds(TrajectorySeed *tmptrajectorySeeds, Trajectory *tmptrajectory,
                  MuonTransientTrackingRecHit::MuonRecHitContainer &seedupRecHits,
                  MuonTransientTrackingRecHit::MuonRecHitContainer &seeddnRecHits,
                  MuonTransientTrackingRecHit::MuonRecHitContainer &muRecHits,
                  std::vector<GEMChamber> gemChambers
                  );
    Trajectory makeTrajectory(TrajectorySeed seed,
                              MuonTransientTrackingRecHit::MuonRecHitContainer &muRecHits,
                              std::vector<GEMChamber> gemChambers,
                              int seedCol
                              );
};

AlignmentTrackRecoQC8::AlignmentTrackRecoQC8(const edm::ParameterSet& ps) : iev(0) {
    time_t rawTime;
    time(&rawTime);
    printf("Begin of AlignmentTrackRecoQC8::AlignmentTrackRecoQC8() at %s\n", asctime(localtime(&rawTime)));
    maxCLS = ps.getParameter<double>("maxClusterSize");
    minCLS = ps.getParameter<double>("minClusterSize");
    trackChi2 = ps.getParameter<double>("trackChi2");
    trackResX = ps.getParameter<double>("trackResX");
    trackResY = ps.getParameter<double>("trackResY");
    MulSigmaOnWindow = ps.getParameter<double>("MulSigmaOnWindow");
    g_SuperChamType = ps.getParameter<vector<string>>("SuperChamberType");
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
    
    shiftX = ps.getParameter<vector<double>>("shiftX");
    rotationZ = ps.getParameter<vector<double>>("rotationZ");
    trueDx = ps.getParameter<vector<double>>("trueDx");
    trueRz = ps.getParameter<vector<double>>("trueRz");
    
    printf("End of AlignmentTrackRecoQC8::AlignmentTrackRecoQC8() at %s\n", asctime(localtime(&rawTime)));
}


void AlignmentTrackRecoQC8::produce(edm::Event& ev, const edm::EventSetup& setup)
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
    
    MuonTransientTrackingRecHit::MuonRecHitContainer muRecHits;
    MuonTransientTrackingRecHit::MuonRecHitContainer seedupRecHits;
    MuonTransientTrackingRecHit::MuonRecHitContainer seeddnRecHits;
    
    for (auto ch : gemChambers)
    {
        int nIdxCh  = ch.id().chamber() + ch.id().layer() - 2; // This way, from 0 to 29
        for (auto etaPart : ch.etaPartitions())
        {
            GEMDetId etaPartID = etaPart->id();
            GEMRecHitCollection::range range = gemRecHits->get(etaPartID);
            
            for (GEMRecHitCollection::const_iterator rechit = range.first; rechit!=range.second; ++rechit)
            {
                const GeomDet* geomDet(etaPart);
                if ((*rechit).clusterSize()<minCLS) continue;
                if ((*rechit).clusterSize()>maxCLS) continue;
                
                LocalPoint rechitLP = rechit->localPosition();
                
                int nIdxSch = nIdxCh/2;
                double Dx = trueDx[nIdxSch];
                double Rz = trueRz[nIdxSch]; // [degree]
                double phi = -Rz*3.14159/180; // [radiants]
                
                double Dx2 = shiftX[nIdxSch];
                double Rz2 = rotationZ[nIdxSch]; // [degree]
                double phi2 = Rz2*3.14159/180; // [radian]
                GlobalPoint rechitGP = mgeom->idToDet((*rechit).rawId())->surface().toGlobal(rechit->localPosition());
                int columnFactor = nIdxCh/10 - 1;
                double centerOfColumn = 56;
                double gx1 = rechitGP.x() + centerOfColumn*columnFactor;
                double gy1 = rechitGP.y();
                double gx2 = gx1*cos(phi+phi2) - gy1*sin(phi+phi2);
                double Dx_Rz = gx1-gx2; // = dx by rz
                double rdx = rechitLP.x();
                double rdy = rechitLP.y();
                double rdz = rechitLP.z();
                
                LocalPoint temphitLP(rdx+Dx-Dx2 +Dx_Rz, rdy, rdz);
                
                GEMRecHit temphit(rechit->gemId(), rechit->BunchX(), rechit->firstClusterStrip(), rechit->clusterSize(), temphitLP, rechit->localPositionError());
                muRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&temphit));
                
                if(nIdxCh%10>=2) seedupRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&temphit)); // up from 2: if only 2 chambers in a column in lower positions... Then zDown<zUp checked...
                if(nIdxCh%10<=7) seeddnRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&temphit)); // up from 7: if only 2 chambers in a column in upper positions... Then zDown<zUp checked...
            }
        }
    }
    
    if (muRecHits.size()<3) return;
    
    Trajectory bestTrajectory;
    TrajectorySeed bestSeed;
    
    findSeeds(&bestSeed, &bestTrajectory, seedupRecHits, seeddnRecHits, muRecHits, gemChambers);
    
    if(!bestTrajectory.isValid())
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
    
    ev.put(std::move(trajectorySeeds));
    ev.put(std::move(trackCollection));
    ev.put(std::move(trackingRecHitCollection));
    ev.put(std::move(trackExtraCollection));
    ev.put(std::move(trajectorys));
    ev.put(std::move(trajectoryChIdx));
    ev.put(std::move(trajectoryType));
}


int AlignmentTrackRecoQC8::findSeeds(TrajectorySeed *tmptrajectorySeeds,
                                     Trajectory *tmptrajectory,
                                     MuonTransientTrackingRecHit::MuonRecHitContainer &seedupRecHits,
                                     MuonTransientTrackingRecHit::MuonRecHitContainer &seeddnRecHits,
                                     MuonTransientTrackingRecHit::MuonRecHitContainer &muRecHits,
                                     std::vector<GEMChamber> gemChambers)
{
    TrajectorySeed bestSeed;
    Trajectory bestTrajectory;
    double minNchi2 = 10000;
    
    for (auto hit1 : seeddnRecHits)
    {
        for (auto hit2 : seedupRecHits)
        {
            if (hit1->globalPosition().z() < hit2->globalPosition().z())
            {
                GEMDetId detId1(hit1->rawId()), detId2(hit2->rawId());
                uint32_t unChNo1 = detId1.chamber()+detId1.layer()-2;
                uint32_t unChNo2 = detId2.chamber()+detId2.layer()-2;
                
                uint32_t unSchCol1 = int(unChNo1/10);
                uint32_t unSchCol2 = int(unChNo2/10);
                uint32_t unDiffCol = unSchCol2 - unSchCol1;
                if(unDiffCol != 0) continue;
                
                uint32_t unSchRow1 = (unChNo1%10)/2;
                uint32_t unSchRow2 = (unChNo2%10)/2;
                uint32_t unDiffRow = unSchRow2 - unSchRow1;
                if(unDiffRow < 1) continue;
                
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
                Trajectory smoothed = makeTrajectory(seed, muRecHits, gemChambers, unSchCol1);
                float Nchi2 = smoothed.chiSquared()/float(smoothed.ndof());
                
                if(minNchi2 > fabs(Nchi2-1) && Nchi2!=0)
                {
                    minNchi2 = fabs(Nchi2-1);
                    bestSeed = seed;
                    bestTrajectory = smoothed;
                }
            }
        }
    }
    
    *tmptrajectorySeeds = bestSeed;
    *tmptrajectory = bestTrajectory;
    
    return 0;
}

Trajectory AlignmentTrackRecoQC8::makeTrajectory(TrajectorySeed seed, MuonTransientTrackingRecHit::MuonRecHitContainer &muRecHits, std::vector<GEMChamber> gemChambers, int seedCol)
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
            if (hitID.chamberId() == ch.id())
            {
            	int chNumber = hitID.chamber()+hitID.layer()-2;
            	if (int(chNumber/10) != seedCol) continue;
                GlobalPoint hitGP = hit->globalPosition();
                double y_err = hit->localPositionError().yy();
                if (fabs(hitGP.x() - tsosGP.x()) > 10.0 * trackResX * MulSigmaOnWindow) continue;
                if (fabs(hitGP.y() - tsosGP.y()) > 1.2 * trackResY * MulSigmaOnWindow * y_err) continue; // global y, local y
                float deltaR = (hitGP - tsosGP).mag();
                if (maxR > deltaR)
                {
                    tmpRecHit = hit;
                    maxR = deltaR;
                    tmpNhit = nhit;
                    tmpR = hitGP.z();
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
DEFINE_FWK_MODULE(AlignmentTrackRecoQC8);

