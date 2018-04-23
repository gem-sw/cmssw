#ifndef gemcrValidation_H
#define gemcrValidation_H

#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"
#include <DataFormats/GEMRecHit/interface/GEMRecHit.h>
#include <DataFormats/GEMRecHit/interface/GEMRecHitCollection.h>
#include <DataFormats/TrackReco/interface/Track.h>
#include <DataFormats/TrackingRecHit/interface/TrackingRecHit.h>
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "Geometry/GEMGeometry/interface/GEMChamber.h"

#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "RecoMuon/CosmicMuonProducer/interface/CosmicMuonSmoother.h"
#include "TrackingTools/KalmanUpdators/interface/KFUpdator.h"

#include "RecoMuon/TransientTrackingRecHit/interface/MuonTransientTrackingRecHit.h"
#include "RecoMuon/StandAloneTrackFinder/interface/StandAloneMuonSmoother.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"

#include "SimDataFormats/Track/interface/SimTrack.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include <TFile.h>
#include <TTree.h>


class gemcrValidation : public GEMBaseValidation
{
public:
  explicit gemcrValidation( const edm::ParameterSet& );
  ~gemcrValidation();
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
  void analyze(const edm::Event& e, const edm::EventSetup&) override;
  int findIndex(GEMDetId id_);
  int findVFAT(float x, float a, float b);
  const GEMGeometry* initGeometry(edm::EventSetup const & iSetup);
  double maxCLS, minCLS,maxRes, trackChi2, trackResY, trackResX, MulSigmaOnWindow;
  std::vector<std::string> SuperChamType;
  std::vector<double> vecChamType;
  bool makeTrack, isMC;
private:
  const GEMGeometry* GEMGeometry_;

  std::vector<GEMChamber> gemChambers;
  int n_ch;
  MuonServiceProxy* theService;
  CosmicMuonSmoother* theSmoother;
  KFUpdator* theUpdator;
  edm::EDGetToken InputTagToken_, InputTagToken_RH, InputTagToken_TR, InputTagToken_TS, InputTagToken_TJ, InputTagToken_TI, InputTagToken_TT, InputTagToken_DG, InputTagToken_US; 

  TH1D *hev;
  TH3D *hvfatHit_numerator;
  TH3D *hvfatHit_denominator;
  TTree *tree;
  TTree *genTree;
  int run;
  int lumi;
  int nev;
  float genMuPt;
  float genMuTheta;
  float genMuPhi;
  float genMuX;
  float genMuY;
  float genMuZ;

  int nrecHit;
  int nrecHitGP;
  const static int maxRecHit = 100;
  float grecHitX[maxRecHit];
  float grecHitY[maxRecHit];
  float grecHitZ[maxRecHit];

  bool onlyOneBestTraj = true;

  const static int maxnTraj = 30;
  int nTraj;
  float trajTheta;
  float trajPhi;
  float trajX;
  float trajY;
  float trajZ;
  float trajPx;
  float trajPy;
  float trajPz;
  int ntrajHit;
  int ntrajRecHit;
  const static int maxNlayer = 30;
  const static int maxNphi = 3;

  float fitamin;
  float fitTheta;
  float fitPhi;
  float fitX;
  float fitY;
  float fitZ;
  float fitPx;
  float fitPy;
  float fitPz;

  const static int maxNeta = 8;
  int vfatI[maxNlayer][maxNphi][maxNeta];
  int vfatF[maxNlayer][maxNphi][maxNeta];
  float trajHitX[maxNlayer][maxNphi][maxNeta];
  float trajHitY[maxNlayer][maxNphi][maxNeta];
  float trajHitZ[maxNlayer][maxNphi][maxNeta];
  float recHitX[maxNlayer][maxNphi][maxNeta];
  float recHitY[maxNlayer][maxNphi][maxNeta];
  float recHitZ[maxNlayer][maxNphi][maxNeta];
  float genHitX[maxNlayer][maxNphi][maxNeta];
  float genHitY[maxNlayer][maxNphi][maxNeta];
  float genHitZ[maxNlayer][maxNphi][maxNeta];

  const static int maxNfloor = 10;
  float floorHitX[maxNfloor];
  float floorHitY[maxNfloor];
  float floorHitZ[maxNfloor];
};

#endif
