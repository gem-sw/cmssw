#include "Validation/GEMCosmicMuonStand/interface/gemcrValidation.h"

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

#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"

#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

#include "RecoMuon/CosmicMuonProducer/interface/HeaderForQC8.h"

#include <iomanip>

#include <TCanvas.h>

using namespace std;
using namespace edm;

gemcrValidation::gemcrValidation(const edm::ParameterSet& cfg): GEMBaseValidation(cfg)
{
  time_t rawTime;
  time(&rawTime);
  printf("Begin of gemcrValidation::gemcrValidation() at %s\n", asctime(localtime(&rawTime)));
  isMC = cfg.getParameter<bool>("isMC");
  InputTagToken_RH = consumes<GEMRecHitCollection>(cfg.getParameter<edm::InputTag>("recHitsInputLabel"));
  InputTagToken_TR = consumes<vector<reco::Track>>(cfg.getParameter<edm::InputTag>("tracksInputLabel"));
  InputTagToken_TS = consumes<vector<TrajectorySeed>>(cfg.getParameter<edm::InputTag>("seedInputLabel"));
  InputTagToken_TJ = consumes<vector<Trajectory>>(cfg.getParameter<edm::InputTag>("trajInputLabel"));
  InputTagToken_TI = consumes<vector<int>>(cfg.getParameter<edm::InputTag>("chNoInputLabel"));
  InputTagToken_TT = consumes<vector<unsigned int>>(cfg.getParameter<edm::InputTag>("seedTypeInputLabel"));
  InputTagToken_DG = consumes<GEMDigiCollection>(cfg.getParameter<edm::InputTag>("gemDigiLabel"));
  if ( isMC ) InputTagToken_US = consumes<edm::HepMCProduct>(cfg.getParameter<edm::InputTag>("genVtx"));
  edm::ParameterSet serviceParameters = cfg.getParameter<edm::ParameterSet>("ServiceParameters");
  theService = new MuonServiceProxy(serviceParameters);
  minCLS = cfg.getParameter<double>("minClusterSize"); 
  maxCLS = cfg.getParameter<double>("maxClusterSize");
  maxRes = cfg.getParameter<double>("maxResidual");
  SuperChamType = cfg.getParameter<vector<string>>("SuperChamberType");
  vecChamType = cfg.getParameter<vector<double>>("SuperChamberSeedingLayers");
  edm::ParameterSet smootherPSet = cfg.getParameter<edm::ParameterSet>("MuonSmootherParameters");
  theSmoother = new CosmicMuonSmoother(smootherPSet, theService);
  theUpdator = new KFUpdator();
  time(&rawTime);

  edm::Service<TFileService> fs;
  
  // Histograms declaration
  
  goodVStriggeredEvts = fs->make<TH1D>("goodVStriggeredEvts","Events with track vs triggered events",2,0,2);
  hitsVFATnum = fs->make<TH3D>("hitsVFATnum","confirmedHits per VFAT (numerator of efficiency)",3,0,3,8,0,8,30,0,30);
  hitsVFATdenom = fs->make<TH3D>("hitsVFATdenom","trajHits per VFAT (denominator of efficiency)",3,0,3,8,0,8,30,0,30);
  digiStrips = fs->make<TH3D>("digiStrips","digi per strip",384,0,384,8,0,8,30,0,30);
  recHits3D = fs->make<TH3D>("recHits3D","recHits 3D map",200,-100,100,156,-61,95,83,-12,154); // volume defined by the scintillators
  clusterSize = fs->make<TH1D>("clusterSize","clusterSize",10,0,10);
  residualPhi = fs->make<TH1D>("residualPhi","residualPhi",400,-5,5);
  residualEta = fs->make<TH1D>("residualEta","residualEta",200,-10,10);
  recHitsPerTrack = fs->make<TH1D>("recHitsPerTrack","recHits per recontructed track",15,0,15);
  
  // Tree branches declaration
  
  tree = fs->make<TTree>("tree", "Tree for QC8");
  tree->Branch("run",&run,"run/I");
  tree->Branch("lumi",&lumi,"lumi/I");
  tree->Branch("ev",&nev,"ev/I");
  tree->Branch("trajTheta",&trajTheta,"trajTheta/F");
  tree->Branch("trajPhi",&trajPhi,"trajPhi/F");
  tree->Branch("trajX",&trajX,"trajX/F");
  tree->Branch("trajY",&trajY,"trajY/F");
  tree->Branch("trajZ",&trajZ,"trajZ/F");
  tree->Branch("trajPx",&trajPx,"trajPx/F");
  tree->Branch("trajPy",&trajPy,"trajPy/F");
  tree->Branch("trajPz",&trajPz,"trajPz/F");
  tree->Branch("testTrajHitX",&testTrajHitX,"testTrajHitX[30]/F");
  tree->Branch("testTrajHitY",&testTrajHitY,"testTrajHitY[30]/F");
  tree->Branch("testTrajHitZ",&testTrajHitZ,"testTrajHitZ[30]/F");
  tree->Branch("confTestHitX",&confTestHitX,"confTestHitX[30]/F");
  tree->Branch("confTestHitY",&confTestHitY,"confTestHitY[30]/F");
  tree->Branch("confTestHitZ",&confTestHitZ,"confTestHitZ[30]/F");
  tree->Branch("nTrajHit",&nTrajHit,"nTrajHit/I");
  tree->Branch("nTrajRecHit",&nTrajRecHit,"nTrajRecHit/I");

  printf("End of gemcrValidation::gemcrValidation() at %s\n", asctime(localtime(&rawTime)));
}

void gemcrValidation::bookHistograms(DQMStore::IBooker & ibooker, edm::Run const & Run, edm::EventSetup const & iSetup ) {
  time_t rawTime;
  time(&rawTime);
  printf("Begin of gemcrValidation::bookHistograms() at %s\n", asctime(localtime(&rawTime)));
  GEMGeometry_ = initGeometry(iSetup);
  if ( GEMGeometry_ == nullptr) return ;  

  const std::vector<const GEMSuperChamber*>& superChambers_ = GEMGeometry_->superChambers();   
  for (auto sch : superChambers_)
  {
    int n_lay = sch->nChambers();
    for (int l=0;l<n_lay;l++)
    {
   	  gemChambers.push_back(*sch->chamber(l+1));
    }
  }
  n_ch = gemChambers.size();
  time(&rawTime);
  printf("End of gemcrValidation::bookHistograms() at %s\n", asctime(localtime(&rawTime)));
}

int gemcrValidation::findIndex(GEMDetId id_) {
  int index=-1;
  for ( int c=0 ; c<n_ch ; c++ )
  {
    if ( gemChambers[c].id().chamber() == id_.chamber() && gemChambers[c].id().layer() == id_.layer() )
    {
      index = c; // index from 0 to 29
    }
  }
  return index;
}

int gemcrValidation::findVFAT(float x, float a, float b) {
  float step = fabs(b-a)/3.0;
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

int g_nEvt = 0;
int g_nNumTrajHit = 0;
int g_nNumMatched = 0;

gemcrValidation::~gemcrValidation() {
  printf("Number of events : %i\n", g_nEvt);
  printf("Number of trajHits : %i (g_nNumTrajHit)\n", g_nNumTrajHit);
  printf("Number of matching trajHits : %i (g_nNumMatched)\n", g_nNumMatched);
  if(g_nNumTrajHit>0) printf("eff : %f\n", double(g_nNumMatched)/g_nNumTrajHit);
}

int g_nNumTest = 0;

void gemcrValidation::analyze(const edm::Event& e, const edm::EventSetup& iSetup){
  
  g_nEvt++;

  run = e.id().run();
  lumi = e.id().luminosityBlock();
  nev = e.id().event();

  goodVStriggeredEvts->Fill(0);
  
  // Variables initialization
  
  nrecHit = 0;
  nTraj = 0;
  
  trajTheta = -999.9;
  trajPhi = -999.9;
  trajX = -999.9;
  trajY = -999.9;
  trajZ = -999.9;
  trajPx = -999.9;
  trajPy = -999.9;
  trajPz = -999.9;
  nTrajHit = 0;
  nTrajRecHit = 0;
  
  for (int i=0; i<30; i++)
  {
    testTrajHitX[i] = testTrajHitY[i] = testTrajHitZ[i] = -999.9;
    confTestHitX[i] = confTestHitY[i] = confTestHitZ[i] = -999.9;
  }

  theService->update(iSetup);

  // digis
  
  if (!isMC)
  {
    edm::Handle<GEMDigiCollection> digis;
    e.getByToken( this->InputTagToken_DG, digis);
    int nNumDigi = 0;
    for (GEMDigiCollection::DigiRangeIterator gemdgIt = digis->begin(); gemdgIt != digis->end(); ++gemdgIt)
    {
      nNumDigi++;
      const GEMDetId& gemId = (*gemdgIt).first;
      const GEMDigiCollection::Range& range = (*gemdgIt).second;
      for ( auto digi = range.first; digi != range.second; ++digi )
      {
        digiStrips->Fill(digi->strip(),gemId.roll(),findIndex(gemId.chamberId()));
      }
    }
  }
  
  // recHits
  
  edm::Handle<GEMRecHitCollection> gemRecHits;
  e.getByToken(this->InputTagToken_RH, gemRecHits);
  if (!gemRecHits.isValid())
  {
    edm::LogError("gemcrValidation") << "Cannot get strips by Token RecHits Token.\n";
    return ;
  }
  
  for ( GEMRecHitCollection::const_iterator rechit = gemRecHits->begin(); rechit != gemRecHits->end(); ++rechit )
  {
    clusterSize->Fill((*rechit).clusterSize());
    if ((*rechit).clusterSize()<minCLS) continue;
    if ((*rechit).clusterSize()>maxCLS) continue;
    
    GlobalPoint recHitGP = GEMGeometry_->idToDet((*rechit).gemId())->surface().toGlobal(rechit->localPosition());
    recHits3D->Fill(recHitGP.x(),recHitGP.y(),recHitGP.z());
    nrecHit++;
  }
  
  edm::Handle<std::vector<int>> idxChTraj;
  e.getByToken( this->InputTagToken_TI, idxChTraj);
  
  edm::Handle<std::vector<TrajectorySeed>> seedGCM;
  e.getByToken( this->InputTagToken_TS, seedGCM);
  
  edm::Handle<std::vector<Trajectory>> trajGCM;
  e.getByToken( this->InputTagToken_TJ, trajGCM);
  
  edm::Handle<vector<reco::Track>> trackCollection;
  e.getByToken( this->InputTagToken_TR, trackCollection);
  
  edm::Handle<std::vector<unsigned int>> seedTypes;
  e.getByToken( this->InputTagToken_TT, seedTypes);

  if ( idxChTraj->size() == 0 ) return;
 
  int countTC = 0;

  for (auto tch : gemChambers)
  {
    countTC += 1;
    
    // Create collection of recHits in the test chamber
    
    MuonTransientTrackingRecHit::MuonRecHitContainer testRecHits;
    
    for (auto etaPart : tch.etaPartitions())
    {
      GEMDetId etaPartID = etaPart->id();
      GEMRecHitCollection::range range = gemRecHits->get(etaPartID);
      for (GEMRecHitCollection::const_iterator rechit = range.first; rechit!=range.second; ++rechit)
      {
        const GeomDet* geomDet(etaPart);
        if ((*rechit).clusterSize()<minCLS) continue;
        if ((*rechit).clusterSize()>maxCLS) continue;
        testRecHits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));        
      }
    }

    // Select trajectory correspondent to test chamber excluded from fit
    
    std::vector<int>::const_iterator it1 = idxChTraj->begin();
    std::vector<TrajectorySeed>::const_iterator it2 = seedGCM->begin();
    std::vector<Trajectory>::const_iterator it3 = trajGCM->begin();
    std::vector<reco::Track>::const_iterator it4 = trackCollection->begin();
    std::vector<unsigned int>::const_iterator it5 = seedTypes->begin();
    
    TrajectorySeed bestSeed;
    Trajectory bestTraj;
    reco::Track bestTrack;
    unsigned int unTypeSeed = 0;
    
    for ( ; it1 != idxChTraj->end() ; ) {
      if ( *it1 == countTC ) {
        bestTraj = *it3;
        bestSeed = (*it3).seed();
        bestTrack = *it4;
        unTypeSeed = *it5;
        break;
      }
      it1++;
      it2++;
      it3++;
      it4++;
      it5++;
    }
    
    if ( it1 == idxChTraj->end() ) continue;
    
    const FreeTrajectoryState* ftsAtVtx = bestTraj.geometricalInnermostState().freeState();
    
    GlobalPoint trackPCA = ftsAtVtx->position();
    GlobalVector gvecTrack = ftsAtVtx->momentum();
    
    PTrajectoryStateOnDet ptsd1(bestSeed.startingState());
    DetId did(ptsd1.detId());
    const BoundPlane& bp = theService->trackingGeometry()->idToDet(did)->surface();
    TrajectoryStateOnSurface tsos = trajectoryStateTransform::transientState(ptsd1,&bp,&*theService->magneticField());
    TrajectoryStateOnSurface tsosCurrent = tsos;

    nTraj++;

    trajTheta = gvecTrack.theta();
    trajPhi = gvecTrack.phi();
    trajX = trackPCA.x();
    trajY = trackPCA.y();
    trajZ = trackPCA.z();
    trajPx = gvecTrack.x();
    trajPy = gvecTrack.y();
    trajPz = gvecTrack.z();

    recHitsPerTrack->Fill(size(bestTraj.recHits()));
    
    // Extrapolation to all the chambers, test chamber selected for efficiency calculation
    
    nTrajHit = 0;
    nTrajRecHit = 0;

    for(int c=0; c<n_ch;c++)
    {
      GEMChamber ch = gemChambers[c];
      const BoundPlane& bpch = GEMGeometry_->idToDet(ch.id())->surface();
      tsosCurrent = theService->propagator("SteppingHelixPropagatorAny")->propagate(tsosCurrent, bpch);
      if (!tsosCurrent.isValid()) continue;
      Global3DPoint gtrp = tsosCurrent.freeTrajectoryState()->position();
      Local3DPoint tlp = bpch.toLocal(gtrp);
      if (!bpch.bounds().inside(tlp)) continue;
      
      if (ch==tch)
      {
        // Find the ieta partition ( -> mRoll )
        
        int n_roll = ch.nEtaPartitions();
        double minDeltaY = 50.;
        int mRoll = -1;
        for (int r=0; r<n_roll; r++)
        {
          const BoundPlane& bproll = GEMGeometry_->idToDet(ch.etaPartition(r+1)->id())->surface();
          Local3DPoint rtlp = bproll.toLocal(gtrp);
          if (minDeltaY > fabs(rtlp.y()))
          {
            minDeltaY = fabs(rtlp.y());
            mRoll = r+1;
          }
        }

        if (mRoll == -1)
        {
          cout << "no mRoll" << endl;
          continue;
        }
        
        // Define region 'inside' the ieta of the chamber
        
        int n_strip = ch.etaPartition(mRoll)->nstrips();
        double min_x = ch.etaPartition(mRoll)->centreOfStrip(1).x();
        double max_x = ch.etaPartition(mRoll)->centreOfStrip(n_strip).x();
        
        if ( (tlp.x()>(min_x)) & (tlp.x() < (max_x)) )
        {
          // For testing the edge eta partition on the top and bottom layers only vertical seeds are allowed!
          
          if ( ( vecChamType[ countTC - 1 ] == 2 || vecChamType[ countTC - 1 ] == 1 ) && 
               ( mRoll == 1 || mRoll == 8 ) && 
               ( unTypeSeed & QC8FLAG_SEEDINFO_MASK_REFVERTROLL18 ) == 0 ) continue;

          uint32_t unDiffCol = ( unTypeSeed & QC8FLAG_SEEDINFO_MASK_DIFFCOL ) >> QC8FLAG_SEEDINFO_SHIFT_DIFFCOL;
            
          if ( ! ( (tlp.x()>(min_x + 1.5)) & (tlp.x() < (max_x - 1.5)) ) )
          {
            if ( unDiffCol != 0 ) 
            {
              continue;
            }
            else if ( ( vecChamType[ countTC - 1 ] == 2 || vecChamType[ countTC - 1 ] == 1 ) )
            {
              continue;
            }
          }
          
          int index = findIndex(ch.id());
          int vfat = findVFAT(tlp.x(), min_x, max_x);

          testTrajHitX[index] = gtrp.x();
          testTrajHitY[index] = gtrp.y();
          testTrajHitZ[index] = gtrp.z();
          hitsVFATdenom->Fill(vfat-1,mRoll-1,index);

          g_nNumTrajHit++;
          nTrajHit++;
          
          // Check if there's a matching recHit in the test chamber (tmpRecHit)
          
          double maxR = 99.9;
          shared_ptr<MuonTransientTrackingRecHit> tmpRecHit;
          
          for (auto hit : testRecHits)
          {
            GEMDetId hitID(hit->rawId());
            if (hitID.chamberId() != ch.id()) continue;
            
            GlobalPoint hitGP = hit->globalPosition();
            
            if (fabs(hitGP.x() - gtrp.x()) > maxRes) continue;
            if (fabs(hitID.roll() - mRoll) > 1) continue;
            
            // Choosing the closest one
            
            double deltaR = (hitGP - gtrp).mag();
            if (deltaR < maxR)
            {
              tmpRecHit = hit;
              maxR = deltaR;
            }
          }

          if(tmpRecHit)
          {
            Global3DPoint recHitGP = tmpRecHit->globalPosition();
            confTestHitX[index] = recHitGP.x();
            confTestHitY[index] = recHitGP.y();
            confTestHitZ[index] = recHitGP.z();
            hitsVFATnum->Fill(vfat-1,mRoll-1,index);
            nTrajRecHit++;
            g_nNumMatched++;
            
            residualPhi->Fill(recHitGP.x()-gtrp.x());
            residualEta->Fill(recHitGP.y()-gtrp.y());
          } 
        }
        continue;
      }
    }
  }
  
  g_nNumTest++;

  tree->Fill();
  goodVStriggeredEvts->Fill(1);
}

