#include "Validation/GEMCosmicMuonStand/interface/AlignmentValidationQC8.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "Geometry/GEMGeometry/interface/GEMSuperChamber.h"
#include <Geometry/GEMGeometry/interface/GEMGeometry.h>
#include <Geometry/Records/interface/MuonGeometryRecord.h>

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/GeometrySurface/interface/Bounds.h"
#include "DataFormats/GeometrySurface/interface/TrapezoidalPlaneBounds.h"
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
#include <Math/Vector3D.h>

using namespace std;
using namespace edm;
using namespace ROOT::Math;

AlignmentValidationQC8::AlignmentValidationQC8(const edm::ParameterSet& cfg): GEMBaseValidation(cfg)
{
  time_t rawTime;
  time(&rawTime);
  printf("Begin of AlignmentValidationQC8::AlignmentValidationQC8() at %s\n", asctime(localtime(&rawTime)));
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
  tree->Branch("trajChi2",&trajPz,"trajChi2/F");
  tree->Branch("nTrajHit",&ntrajHit,"nTrajHit/I");
  tree->Branch("nTrajRecHit",&ntrajRecHit,"nTrajRecHit/I");

  tree->Branch("SchSeed",SchSeed,"SchSeed[2]/I");
  tree->Branch("excludedSch", &excludedSch,"excludedSch/I");
  tree->Branch("excludedSch2", &excludedSch2,"excludedSch2/I");
  tree->Branch("chI",chI,"chI[30]/I");
  tree->Branch("chF",chF,"chF[30]/I");
  tree->Branch("chTrajHitX",chTrajHitX,"chTrajHitX[30]/F");
  tree->Branch("chTrajHitY",chTrajHitY,"chTrajHitY[30]/F");
  tree->Branch("chTrajHitZ",chTrajHitZ,"chTrajHitZ[30]/F");
  tree->Branch("chRecHitX",chRecHitX,"chRecHitX[30]/F");
  tree->Branch("chRecHitY",chRecHitY,"chRecHitY[30]/F");
  tree->Branch("chRecHitZ",chRecHitZ,"chRecHitZ[30]/F");
  tree->Branch("chRecHitXE",chRecHitXE,"chRecHitXE[30]/F");
  tree->Branch("chRecHitYE",chRecHitYE,"chRecHitYE[30]/F");
  tree->Branch("chGenHitX",chGenHitX,"chGenHitX[30]/F");
  tree->Branch("chGenHitY",chGenHitY,"chGenHitY[30]/F");
  tree->Branch("chGenHitZ",chGenHitZ,"chGenHitZ[30]/F");
  tree->Branch("chCHitX",chCHitX,"chCHitX[30]/F");
  tree->Branch("chCHitY",chCHitY,"chCHitY[30]/F");
  tree->Branch("chCHitZ",chCHitZ,"chCHitZ[30]/F");
  tree->Branch("chCHitD",chCHitD,"chCHitD[30]/F");
  tree->Branch("chPHitZ",chPHitZ,"chPHitZ[30]/F");

  tree->Branch("dx",dx,"dx[30]/F");
  tree->Branch("rz",rz,"rz[30]/F");
  tree->Branch("tDx",tDx,"tDx[30]/F");
  tree->Branch("tRz",tRz,"tRz[30]/F");

  shiftX = cfg.getParameter<vector<double>>("shiftX");
  rotationZ = cfg.getParameter<vector<double>>("rotationZ");
  trueDx = cfg.getParameter<vector<double>>("trueDx");
  trueRz = cfg.getParameter<vector<double>>("trueRz");

  printf("End of AlignmentValidationQC8::AlignmentValidationQC8() at %s\n", asctime(localtime(&rawTime)));
}

void AlignmentValidationQC8::bookHistograms(DQMStore::IBooker & ibooker, edm::Run const & Run, edm::EventSetup const & iSetup ) {
  time_t rawTime;
  time(&rawTime);
  printf("Begin of AlignmentValidationQC8::bookHistograms() at %s\n", asctime(localtime(&rawTime)));
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
  printf("End of AlignmentValidationQC8::bookHistograms() at %s\n", asctime(localtime(&rawTime)));
}

int AlignmentValidationQC8::findIndex(GEMDetId id_) {
  int index=-1;
  for ( int c=0 ; c<n_ch ; c++ )
  {
    if ( gemChambers[c].id().chamber() == id_.chamber() && gemChambers[c].id().layer() == id_.layer() ){index = c;}
  }
  return index;
}

int AlignmentValidationQC8::findVFAT(float x, float a, float b) {
  float step = abs(b-a)/3.0;
  if ( x < (min(a,b)+step) ) { return 1;}
  else if ( x < (min(a,b)+2.0*step) ) { return 2;}
  else { return 3;}
}

const GEMGeometry* AlignmentValidationQC8::initGeometry(edm::EventSetup const & iSetup) {
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

int a_nEvt = 0;
int a_nNumTrajHit = 0;
int a_nNumMatched = 0;
int a_nNumTrajHitChNumerator[100] = {0,};
int a_nNumTrajHitChDenominator[100] = {0,};

double a_dMinX = 100000.0, a_dMaxX = -10000000.0;
double a_dMinY = 100000.0, a_dMaxY = -10000000.0;
double a_dMinZ = 100000.0, a_dMaxZ = -10000000.0;

AlignmentValidationQC8::~AlignmentValidationQC8() {
  printf("Number of events : %i\n", a_nEvt);
  printf("Number of trajHits : %i (a_nNumTrajHit)\n", a_nNumTrajHit);
  printf("Number of matching trajHits : %i (a_nNumMatched)\n", a_nNumMatched);
  if(a_nNumTrajHit>0) printf("eff : %f\n", double(a_nNumMatched)/a_nNumTrajHit);
  
  printf("numerator per chamber\n");
  for(int i=0;i<30;i++)
  {
    printf("%i\t",a_nNumTrajHitChNumerator[i]);
    if(i%10==9) printf("\n");
  }
  printf("\ndenominator per chamber\n");
  for(int i=0;i<30;i++)
  {
    printf("%i\t",a_nNumTrajHitChDenominator[i]);
    if(i%10==9) printf("\n");
  }
  printf("\neff per chamber\n");
  for(int i=0;i<30;i++)
  {
    printf("%3.1f\t",100*double(a_nNumTrajHitChNumerator[i])/a_nNumTrajHitChDenominator[i]);
    if(i%10==9) printf("\n");
  }
  printf("\n");
  
  printf("X range : %lf, %lf\n", a_dMinX, a_dMaxX);
  printf("Y range : %lf, %lf\n", a_dMinY, a_dMaxY);
  printf("Z range : %lf, %lf\n", a_dMinZ, a_dMaxZ);
}

int a_nNumTest = 0;


void AlignmentValidationQC8::analyze(const edm::Event& e, const edm::EventSetup& iSetup){

  a_nEvt++;

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
    chI[i] = 0;
    chF[i] = 0;
    chTrajHitX[i] = 0;
    chTrajHitY[i] = 0;
    chTrajHitZ[i] = 0;
    chRecHitX[i] = 0;
    chRecHitY[i] = 0;
    chRecHitZ[i] = 0;
    chRecHitXE[i] = 0;
    chRecHitYE[i] = 0;
    chGenHitX[i] = 0;
    chGenHitY[i] = 0;
    chGenHitZ[i] = 0;
    chChi2[i] = -1;

    int n = i/2;
    dx[i] = shiftX[n];
    rz[i] = rotationZ[n];
    tDx[i] = trueDx[n];
    tRz[i] = trueRz[n];
  }
  trajTheta = -10;
  trajPhi = -10;
  trajX = 0;
  trajY = 0;
  trajZ = 0;
  trajPx = 0;
  trajPy = 0;
  trajPz = 0;
  trajChi2 = -1;
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
  e.getByToken( this->InputTagToken_RH, gemRecHits);
  if(gemRecHits->size() <= 4) return;
  
  float fXGenGP1x = 0.0, fXGenGP1y = 0.0, fXGenGP1z = 0.0;
  
  HepMC::GenParticle *genMuon = NULL;
  
  double gen_px = 0;
  double gen_py = 0;
  double gen_pz = 0;
  double gen_pt = 0;
  double gen_theta = 0;
  double gen_phi = 0;

  if ( isMC )
  {
    edm::Handle<edm::HepMCProduct> genVtx;
    e.getByToken( this->InputTagToken_US, genVtx);
    genMuon = genVtx->GetEvent()->barcode_to_particle(1);
    
    double dUnitGen = 0.1;
    
    fXGenGP1x = dUnitGen * genMuon->production_vertex()->position().x();
    fXGenGP1y = dUnitGen * genMuon->production_vertex()->position().y();
    fXGenGP1z = dUnitGen * genMuon->production_vertex()->position().z();

    gen_px = genMuon->momentum().x();
    gen_py = genMuon->momentum().y();
    gen_pz = genMuon->momentum().z();
    gen_pt = genMuon->momentum().perp();
    gen_theta = genMuon->momentum().theta();
    gen_phi = genMuon->momentum().phi();

    genMuPt = gen_pt;
    genMuTheta = gen_theta;
    genMuPhi = gen_phi;

    genMuX = fXGenGP1x;
    genMuY = fXGenGP1y;
    genMuZ = fXGenGP1z;

    genTree->Fill();
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

  if(trackCollection->size() == 0) return;

  std::vector<int>::const_iterator it1 = idxChTraj->begin();
  excludedSch = *it1;
  excludedSch2 = *(++it1);

  std::vector<Trajectory>::const_iterator it3 = trajGCM->begin();
  Trajectory bestTraj = *it3;

  Trajectory::RecHitContainer transHits = bestTraj.recHits();


  MuonTransientTrackingRecHit::MuonRecHitContainer testRecHits;
  for (auto tch : gemChambers)
  {
    for (auto etaPart : tch.etaPartitions()){
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
  }

  TrajectorySeed bestSeed = (*it3).seed();
  TrajectorySeed::range range = bestSeed.recHits();
  int nseed = 0;
  
  for (edm::OwnVector<TrackingRecHit>::const_iterator rechit = range.first; rechit!=range.second; ++rechit){
    GEMDetId hitID(rechit->rawId());
    int nIdxSch = int(hitID.chamber()+hitID.layer()-2)/2;
    SchSeed[nseed] = nIdxSch;
    nseed++;
  }

  PTrajectoryStateOnDet ptsd1(bestSeed.startingState());
  DetId did(ptsd1.detId());
  const BoundPlane& bp = theService->trackingGeometry()->idToDet(did)->surface();
  TrajectoryStateOnSurface tsos = trajectoryStateTransform::transientState(ptsd1,&bp,&*theService->magneticField());
  TrajectoryStateOnSurface tsosCurrent = tsos;

  const FreeTrajectoryState* ftsAtVtx = bestTraj.geometricalInnermostState().freeState();
  GlobalPoint trajVertex = ftsAtVtx->position();
  GlobalVector trajMomentum = ftsAtVtx->momentum();
  trajX = trajVertex.x();
  trajY = trajVertex.y();
  trajZ = trajVertex.z();
  trajPx = trajMomentum.x();
  trajPy = trajMomentum.y();
  trajPz = trajMomentum.z();
  trajTheta = trajMomentum.theta();
  trajPhi = trajMomentum.phi();
  trajChi2 = bestTraj.chiSquared()/float(bestTraj.ndof());

  int nTrajHit = 0, nTrajRecHit = 0;
  for(int c=0; c<n_ch;c++)
  {
    GEMChamber ch = gemChambers[c];
    const BoundPlane& bpch = GEMGeometry_->idToDet(ch.id())->surface();

    tsosCurrent = theService->propagator("SteppingHelixPropagatorAny")->propagate(tsosCurrent, bpch);
    if (!tsosCurrent.isValid()) continue;
    Global3DPoint gtrp = tsosCurrent.freeTrajectoryState()->position();
    Global3DPoint gtrpGEN(
      fXGenGP1x + ( genMuon->momentum().x() / genMuon->momentum().z() ) * ( gtrp.z() - fXGenGP1z ), 
      fXGenGP1y + ( genMuon->momentum().y() / genMuon->momentum().z() ) * ( gtrp.z() - fXGenGP1z ),
      gtrp.z());
      
    Global3DPoint gtrp2(trajX + (gtrp.z() - 0.34125 - trajZ)*trajPx/trajPz, trajY + (gtrp.z() - 0.34125 - trajZ)*trajPy/trajPz, gtrp.z() - 0.34125);
    gtrp = gtrp2;

    const Surface::PositionType oldPos(bpch.position());
    const Surface::RotationType oldRot(bpch.rotation());
    
    GlobalPoint newPos(oldPos.x(), oldPos.y(), oldPos.z());

    const BoundPlane& bpch2 = BoundPlane(newPos, oldRot, bpch.bounds().clone());
    Local3DPoint tlp2 = bpch2.toLocal(gtrp);

    if (!bpch2.bounds().inside(tlp2)){continue;}
    {
      int n_roll = ch.nEtaPartitions();
      double minDely = 50.;
      int mRoll = -1;
      for (int r=0;r<n_roll;r++)
      {
        const BoundPlane& bproll = GEMGeometry_->idToDet(ch.etaPartition(r+1)->id())->surface();

        const Surface::PositionType oldRollPos(bproll.position());
        const Surface::RotationType oldRollRot(bproll.rotation());
        
        GlobalPoint newRollPos(oldRollPos.x()+shiftX[c/2], oldRollPos.y(), oldRollPos.z());
        const BoundPlane& bproll2 = BoundPlane(newRollPos, oldRollRot, bproll.bounds().clone());
        Local3DPoint rtlp2 = bproll2.toLocal(gtrp);

        if(minDely > fabs(rtlp2.y())) {minDely = fabs(rtlp2.y()); mRoll = r+1;}
      }

      if (mRoll == -1){cout << "no mRoll" << endl;continue;}
      
      int n_strip = ch.etaPartition(mRoll)->nstrips();
      double min_x = ch.etaPartition(mRoll)->centreOfStrip(1).x();
      double max_x = ch.etaPartition(mRoll)->centreOfStrip(n_strip).x();
      
      if ( (tlp2.x()>(min_x)) & (tlp2.x() < (max_x)) )
      {
        int index = findIndex(ch.id());
        double vfat = findVFAT(tlp2.x(), min_x, max_x);
        int idx = index;
        int ivfat = (int)vfat - 1;
        int imRoll = mRoll - 1;
        vfatI[idx][ivfat][imRoll]=1;
        vfatF[idx][ivfat][imRoll]=0;
        chI[idx]=1;
        chTrajHitX[idx] = gtrp.x();
        chTrajHitY[idx] = gtrp.y();
        chTrajHitZ[idx] = gtrp.z();
        chRecHitX[idx] = 0;
        chRecHitY[idx] = 0;
        chRecHitZ[idx] = 0;
        chRecHitXE[idx] = 0;
        chRecHitYE[idx] = 0;
        chGenHitX[idx] = genMuX + (gtrp.z()-genMuZ)*(gen_px/gen_pz);
        chGenHitY[idx] = genMuY + (gtrp.z()-genMuZ)*(gen_py/gen_pz);
        chGenHitZ[idx] = gtrp.z();
        chEtaTrajHitX[idx][imRoll] = gtrp.x();
        chEtaTrajHitY[idx][imRoll] = gtrp.y();
        chEtaTrajHitZ[idx][imRoll] = gtrp.z();

        chChi2[idx] = bestTraj.chiSquared()/float(bestTraj.ndof());

        int n1 = imRoll;
        int n2 = 2-ivfat + int(2-idx/10)*3;
        int n3 = idx%10;
        hvfatHit_denominator->Fill(n1,n2,n3);

        genHitX[idx][ivfat][imRoll] = genMuX + (gtrp.z()-genMuZ)*(gen_px/gen_pz);
        genHitY[idx][ivfat][imRoll] = genMuY + (gtrp.z()-genMuZ)*(gen_py/gen_pz);
        genHitZ[idx][ivfat][imRoll] = gtrp.z();
        trajHitX[idx][ivfat][imRoll] = gtrp.x();
        trajHitY[idx][ivfat][imRoll] = gtrp.y();
        trajHitZ[idx][ivfat][imRoll] = gtrp.z();
        recHitX[idx][ivfat][imRoll] = 0;
        recHitY[idx][ivfat][imRoll] = 0;
        recHitZ[idx][ivfat][imRoll] = 0;

        int floor = idx%10;
        floorHitX[floor] = gtrp.x();
        floorHitY[floor] = gtrp.y();
        floorHitZ[floor] = gtrp.z();

        a_nNumTrajHitChDenominator[idx]++;
        a_nNumTrajHit++;
        nTrajHit++;
        
        if ( a_dMinX > gtrp.x() ) a_dMinX = gtrp.x();
        if ( a_dMaxX < gtrp.x() ) a_dMaxX = gtrp.x();
        if ( a_dMinY > gtrp.y() ) a_dMinY = gtrp.y();
        if ( a_dMaxY < gtrp.y() ) a_dMaxY = gtrp.y();
        if ( a_dMinZ > gtrp.z() ) a_dMinZ = gtrp.z();
        if ( a_dMaxZ < gtrp.z() ) a_dMaxZ = gtrp.z();
          

        Global3DPoint recHitGP;
        double maxR = 99.9;
        shared_ptr<MuonTransientTrackingRecHit> tmpRecHit;
        
        for (auto hit : testRecHits)
        {
          GEMDetId hitID(hit->rawId());
          if (hitID.chamberId() != ch.id()) continue;
          GlobalPoint hitGP = hit->globalPosition();
          int nIdxSch = int(hitID.chamber()+hitID.layer()-2)/2;
          
          double Dx = trueDx[nIdxSch];
          double Rz = trueRz[nIdxSch]; // [degrees]
          double phi = -Rz*3.14159/180; // [radiants]

          double Dx2 = shiftX[nIdxSch];
          double Rz2 = rotationZ[nIdxSch]; // [degrees]
          double phi2 = Rz2*3.14159/180; // [radiants]

          int columnFactor = (nIdxSch*2)/10 - 1;
          double centerOfColumn = 56;
          double gx1 = hitGP.x() + centerOfColumn*columnFactor;
          double gy1 = hitGP.y();
          double gx2 = gx1*cos(phi+phi2) - gy1*sin(phi+phi2);
          double Dx_Rz = gx1-gx2;
          double GPdx = hitGP.x();
          double GPdy = hitGP.y();
          double GPdz = hitGP.z();
          
          hitGP = GlobalPoint(GPdx-Dx+Dx2 -Dx_Rz, GPdy, GPdz);
          
          if (fabs(hitGP.x() - gtrp.x()) > maxRes+0.5) continue;
          if (abs(hitID.roll() - mRoll)>1) continue;
          
          double deltaR = (hitGP - gtrp).mag();
          if (maxR > deltaR)
          {
            tmpRecHit = hit;
            maxR = deltaR;
            recHitGP = hitGP;
          }
        }

        if(tmpRecHit)
        {
          a_nNumTrajHitChNumerator[idx]++;
          a_nNumMatched++;
          nTrajRecHit++;

          recHitX[idx][ivfat][imRoll] = recHitGP.x();
          recHitY[idx][ivfat][imRoll] = recHitGP.y();
          recHitZ[idx][ivfat][imRoll] = recHitGP.z();
          vfatF[idx][ivfat][imRoll]=1;
          chF[idx]=1;
          chRecHitX[idx] = recHitGP.x();
          chRecHitY[idx] = recHitGP.y();
          chRecHitZ[idx] = recHitGP.z();
          hvfatHit_numerator->Fill(n1,n2,n3);
          chEtaRecHitX[idx][imRoll] = recHitGP.x();
          chEtaRecHitY[idx][imRoll] = recHitGP.y();
          chEtaRecHitZ[idx][imRoll] = recHitGP.z();
          
          double x1 = trajX;
          double y1 = trajY;
          double z1 = trajZ;
          XYZVector r1(x1, y1, z1);
          XYZVector r2(chRecHitX[idx], chRecHitY[idx], chRecHitZ[idx]);
          double u1 = gtrp.x()-x1;
          double u2 = gtrp.y()-y1;
          double u3 = gtrp.z()-z1;
          XYZVector u(u1, u2, u3);
          XYZVector u0 = u.Unit();
          double AH = sqrt(((r2-r1).Cross(u0)).Mag2());
          double PA = sqrt((r2-r1).Mag2());
          double PH = sqrt(PA*PA - AH*AH);
          double du = sqrt(u.Mag2());
          chCHitX[idx] = x1 + PH*u1/du;
          chCHitY[idx] = y1 + PH*u2/du;
          chCHitZ[idx] = z1 + PH*u3/du;
          chCHitD[idx] = AH;
          chPHitZ[idx] = z1 + (chRecHitX[idx] - x1)*u3/u1;
        }
      }
    }
  }

  tree->Fill();
  hev->Fill(2);
  a_nNumTest++;

}
