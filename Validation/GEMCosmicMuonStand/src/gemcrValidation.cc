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
  makeTrack = cfg.getParameter<bool>("makeTrack");
  trackChi2 = cfg.getParameter<double>("trackChi2");
  trackResY = cfg.getParameter<double>("trackResY"); 
  trackResX = cfg.getParameter<double>("trackResX");
  MulSigmaOnWindow = cfg.getParameter<double>("MulSigmaOnWindow");
  SuperChamType = cfg.getParameter<vector<string>>("SuperChamberType");
  vecChamType = cfg.getParameter<vector<double>>("SuperChamberSeedingLayers");
  edm::ParameterSet smootherPSet = cfg.getParameter<edm::ParameterSet>("MuonSmootherParameters");
  theSmoother = new CosmicMuonSmoother(smootherPSet, theService);
  theUpdator = new KFUpdator();
  time(&rawTime);

  edm::Service<TFileService> fs;
  hev = fs->make<TH1D>("hev","EventSummary",3,1,3);

  genTree = fs->make<TTree>("genTree", "gen info for QC8");
  genTree->Branch("genMuPt",&genMuPt,"genMuPt/F");
  genTree->Branch("genMuTheta",&genMuTheta,"genMuTheta/F");
  genTree->Branch("genMuPhi",&genMuPhi,"genMuPhi/F");
  genTree->Branch("genMuX",&genMuX,"genMuX/F");
  genTree->Branch("genMuY",&genMuY,"genMuY/F");
  genTree->Branch("genMuZ",&genMuZ,"genMuZ/F");
  genTree->Branch("nTraj",&nTraj,"nTraj/I");

  genTree->Branch("nrecHit",&nrecHit,"nrecHit/I");
  genTree->Branch("grecHitX",grecHitX,"grecHitX[nrecHit]/F");
  genTree->Branch("grecHitY",grecHitY,"grecHitY[nrecHit]/F");
  genTree->Branch("grecHitZ",grecHitZ,"grecHitZ[nrecHit]/F");

  hvfatHit_numerator = fs->make<TH3D>("hvfatHit_numerator","vfat hit (numerator of efficiency)",8,0,8,9,0,9,10,0,10);
  hvfatHit_denominator = fs->make<TH3D>("hvfatHit_denominator","vfat hit (denominator of efficienct)",8,0,8,9,0,9,10,0,10);
  tree = fs->make<TTree>("tree", "Tree for QC8");
  tree->Branch("run",&run,"run/I");
  tree->Branch("lumi",&lumi,"lumi/I");
  tree->Branch("ev",&nev,"ev/I");

  tree->Branch("genMuPt",&genMuPt,"genMuPt/F");
  tree->Branch("genMuTheta",&genMuTheta,"genMuTheta/F");
  tree->Branch("genMuPhi",&genMuPhi,"genMuPhi/F");
  tree->Branch("genMuX",&genMuX,"genMuX/F");
  tree->Branch("genMuY",&genMuY,"genMuY/F");
  tree->Branch("genMuZ",&genMuZ,"genMuZ/F");

  tree->Branch("trajTheta",&trajTheta,"trajTheta/F");
  tree->Branch("trajPhi",&trajPhi,"trajPhi/F");
  tree->Branch("trajX",&trajX,"trajX/F");
  tree->Branch("trajY",&trajY,"trajY/F");
  tree->Branch("trajZ",&trajZ,"trajZ/F");
  tree->Branch("trajPx",&trajPx,"trajPx/F");
  tree->Branch("trajPy",&trajPy,"trajPy/F");
  tree->Branch("trajPz",&trajPz,"trajPz/F");
  tree->Branch("nTrajHit",&ntrajHit,"nTrajHit/I");
  tree->Branch("nTrajRecHit",&ntrajRecHit,"nTrajRecHit/I");

  tree->Branch("vfatI",vfatI,"vfatI[30][3][8]/I");
  tree->Branch("vfatF",vfatF,"vfatF[30][3][8]/I");

  tree->Branch("trajHitX",trajHitX,"trajHitX[30][3][8]/F");
  tree->Branch("trajHitY",trajHitY,"trajHitY[30][3][8]/F");
  tree->Branch("trajHitZ",trajHitZ,"trajHitZ[30][3][8]/F");
  tree->Branch("recHitX",recHitX,"recHitX[30][3][8]/F");
  tree->Branch("recHitY",recHitY,"recHitY[30][3][8]/F");
  tree->Branch("recHitZ",recHitZ,"recHitZ[30][3][8]/F");
  tree->Branch("genHitX",genHitX,"genHitX[30][3][8]/F");
  tree->Branch("genHitY",genHitY,"genHitY[30][3][8]/F");
  tree->Branch("genHitZ",genHitZ,"genHitZ[30][3][8]/F");

  tree->Branch("floorHitX",floorHitX,"floorHitX[10]/F");
  tree->Branch("floorHitY",floorHitY,"floorHitY[10]/F");
  tree->Branch("floorHitZ",floorHitZ,"floorHitZ[10]/F");

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
    if ( gemChambers[c].id().chamber() == id_.chamber() && gemChambers[c].id().layer() == id_.layer() ){index = c;}
  }
  return index;
}

int gemcrValidation::findVFAT(float x, float a, float b) {
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

  hev->Fill(1);

  genMuPt = -10;
  genMuTheta = -10;
  genMuPhi = -10;
  genMuX = 0;
  genMuY = 0;
  genMuZ = 0;

  nTraj = 0;

  for(int i=0;i<maxNlayer;i++)
  {
    for(int j=0;j<maxNphi;j++)
    {
      for(int k=0;k<maxNeta;k++)
      {
        vfatI[i][j][k] = 0;
        vfatF[i][j][k] = 0;
        trajHitX[i][j][k] = 0;
        trajHitY[i][j][k] = 0;
        trajHitZ[i][j][k] = 0;
        recHitX[i][j][k] = 0;
        recHitY[i][j][k] = 0;
        recHitZ[i][j][k] = 0;
        genHitX[i][j][k] = 0;
        genHitY[i][j][k] = 0;
        genHitZ[i][j][k] = 0;
      }
    }
  }
  trajTheta = -10;
  trajPhi = -10;
  trajX = 0;
  trajY = 0;
  trajZ = 0;
  trajPx = 0;
  trajPy = 0;
  trajPz = 0;
  ntrajHit = 0;
  ntrajRecHit = 0;

  for(int i=0;i<maxNfloor;i++)
  {
    floorHitX[i] = 0;
    floorHitY[i] = 0;
    floorHitZ[i] = 0;
  }

  nrecHit = 0;

  theService->update(iSetup);

  edm::Handle<GEMRecHitCollection> gemRecHits;
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
        cout << digi->strip() << gemId.roll(); // Will be filled in a histogram!!
      }
    }
  }
  
  e.getByToken( this->InputTagToken_RH, gemRecHits);
  if (!gemRecHits.isValid())
  {
    edm::LogError("gemcrValidation") << "Cannot get strips by Token RecHits Token.\n";
    return ;
  }
  
  double gen_px = 0;
  double gen_py = 0;
  double gen_pz = 0;
  double gen_pt = 0;
  double gen_theta = 0;
  double gen_phi = 0;

  if ( isMC )
  {
    HepMC::GenParticle *genMuon = NULL;
  
    edm::Handle<edm::HepMCProduct> genVtx;
    e.getByToken( this->InputTagToken_US, genVtx);
    genMuon = genVtx->GetEvent()->barcode_to_particle(1);
    
    double dUnitGen = 0.1;
    
    genMuX = dUnitGen * genMuon->production_vertex()->position().x();
    genMuY = dUnitGen * genMuon->production_vertex()->position().y();
    genMuZ = dUnitGen * genMuon->production_vertex()->position().z();

    gen_px = genMuon->momentum().x();
    gen_py = genMuon->momentum().y();
    gen_pz = genMuon->momentum().z();
    gen_pt = genMuon->momentum().perp();
    gen_theta = genMuon->momentum().theta();
    gen_phi = genMuon->momentum().phi();

    genMuPt = gen_pt;
    genMuTheta = gen_theta;
    genMuPhi = gen_phi;

    for ( GEMRecHitCollection::const_iterator rechit = gemRecHits->begin(); rechit != gemRecHits->end(); ++rechit )
    {
      if ((*rechit).clusterSize()<minCLS) continue;
      if ((*rechit).clusterSize()>maxCLS) continue;

      GlobalPoint recHitGP = GEMGeometry_->idToDet((*rechit).gemId())->surface().toGlobal(rechit->localPosition());
      grecHitX[nrecHit] = recHitGP.x();
      grecHitY[nrecHit] = recHitGP.y();
      grecHitZ[nrecHit] = recHitGP.z();
      nrecHit++;
    }
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

  genTree->Fill();

  if ( idxChTraj->size() == 0 ) return;

  if (!makeTrack) return; 
  int countTC = 0;

  for (auto tch : gemChambers)
  {
    countTC += 1;
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

    std::vector<int>::const_iterator it1 = idxChTraj->begin();
    std::vector<TrajectorySeed>::const_iterator it2 = seedGCM->begin();
    std::vector<Trajectory>::const_iterator it3 = trajGCM->begin();
    std::vector<reco::Track>::const_iterator it4 = trackCollection->begin();
    std::vector<unsigned int>::const_iterator it5 = seedTypes->begin();
    
    TrajectorySeed bestSeed;
    Trajectory bestTraj;
    reco::Track bestTrack;
    unsigned int unTypeSeed = 0;
    
    for ( ; it1 != idxChTraj->end() ; )
    {
      if ( *it1 == countTC )
      {
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

    int nTrajHit = 0, nTrajRecHit = 0;

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
        int n_roll = ch.nEtaPartitions();
        double minDely = 50.;
        int mRoll = -1;
        for (int r=0;r<n_roll;r++)
        {
          const BoundPlane& bproll = GEMGeometry_->idToDet(ch.etaPartition(r+1)->id())->surface();
          Local3DPoint rtlp = bproll.toLocal(gtrp);
          if(minDely > abs(rtlp.y())){minDely = abs(rtlp.y()); mRoll = r+1;}
        }

        if (mRoll == -1){cout << "no mRoll" << endl;continue;}
        
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

          int ivfat = vfat - 1;
          int imRoll = mRoll - 1;
          vfatI[index][ivfat][imRoll]=1;
          vfatF[index][ivfat][imRoll]=0;

          hvfatHit_denominator->Fill(imRoll,2-ivfat + int(2-index/10)*3,index%10);

          genHitX[index][ivfat][imRoll] = genMuX + (gtrp.z()-genMuZ)*(gen_px/gen_pz);
          genHitY[index][ivfat][imRoll] = genMuY + (gtrp.z()-genMuZ)*(gen_py/gen_pz);
          genHitZ[index][ivfat][imRoll] = gtrp.z();
          trajHitX[index][ivfat][imRoll] = gtrp.x();
          trajHitY[index][ivfat][imRoll] = gtrp.y();
          trajHitZ[index][ivfat][imRoll] = gtrp.z();
          recHitX[index][ivfat][imRoll] = 0;
          recHitY[index][ivfat][imRoll] = 0;
          recHitZ[index][ivfat][imRoll] = 0;

          int floor = index%10;
          floorHitX[floor] = gtrp.x();
          floorHitY[floor] = gtrp.y();
          floorHitZ[floor] = gtrp.z();

          ntrajHit++;
          g_nNumTrajHit++;
          nTrajHit++;
          
          double maxR = 99.9;
          shared_ptr<MuonTransientTrackingRecHit> tmpRecHit;
          
          for (auto hit : testRecHits)
          {
            GEMDetId hitID(hit->rawId());
            if (hitID.chamberId() != ch.id()) continue;
            GlobalPoint hitGP = hit->globalPosition();
            if (abs(hitGP.x() - gtrp.x()) > maxRes) continue;
            if (abs(hitID.roll() - mRoll)>1) continue;
            double deltaR = (hitGP - gtrp).mag();
            if (maxR > deltaR)
            {
              tmpRecHit = hit;
              maxR = deltaR;
            }
          }

          if(tmpRecHit)
          {
            Global3DPoint recHitGP = tmpRecHit->globalPosition();
            nTrajRecHit++;
            recHitX[index][ivfat][imRoll] = recHitGP.x();
            recHitY[index][ivfat][imRoll] = recHitGP.y();
            recHitZ[index][ivfat][imRoll] = recHitGP.z();
            vfatF[index][ivfat][imRoll]=1;
            hvfatHit_numerator->Fill(imRoll,2-ivfat + int(2-index/10)*3,index%10);
            ntrajRecHit++;
            g_nNumMatched++;
          } 
        }
        continue;
      }
    }
  }
  
  g_nNumTest++;

  tree->Fill();
  hev->Fill(2);
}
