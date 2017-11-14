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


class MuonRecHitContainerLayered : public MuonTransientTrackingRecHit::MuonRecHitContainer {
 protected:
  
 public:
  std::vector<int> Layers;
  MuonRecHitContainerLayered( MuonTransientTrackingRecHit::MuonRecHitContainer &rechit, std::vector<int> _Layers );
  ~MuonRecHitContainerLayered( void );
  int GetLayer( unsigned int _L );
  // MuonTransientTrackingRecHit::MuonRecHitContaine& GetmuRecHits(void);
};


class gemcrValidation : public GEMBaseValidation
{
public:
  explicit gemcrValidation( const edm::ParameterSet& );
  ~gemcrValidation();
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
  void analyze(const edm::Event& e, const edm::EventSetup&) override;
  int findIndex(GEMDetId id_);
  int findvfat(float x, float a, float b);
  const GEMGeometry* initGeometry(edm::EventSetup const & iSetup);
  double maxCLS, minCLS,maxRes, trackChi2, trackResY, trackResX;
  bool makeTrack, isMC;
private:
  const GEMGeometry* GEMGeometry_;
  std::vector<MonitorElement*> gem_chamber_x_y;
  std::vector<MonitorElement*> gem_chamber_cl_size;
  std::vector<MonitorElement*> gem_chamber_bx;
  std::vector<MonitorElement*> gem_chamber_tr2D_eff;
  std::vector<MonitorElement*> gem_chamber_th2D_eff;
  std::vector<MonitorElement*> gem_chamber_trxroll_eff;
  std::vector<MonitorElement*> gem_chamber_thxroll_eff;
  std::vector<MonitorElement*> gem_chamber_trxy_eff;
  std::vector<MonitorElement*> gem_chamber_thxy_eff;
  std::vector<MonitorElement*> gem_chamber_residual;
  std::vector<MonitorElement*> gem_chamber_local_x;
  std::vector<MonitorElement*> gem_chamber_digi_digi;
  std::vector<MonitorElement*> gem_chamber_digi_recHit;
  std::vector<MonitorElement*> gem_chamber_digi_CLS;
  std::vector<MonitorElement*> gem_chamber_hitMul;
  std::vector<MonitorElement*> gem_chamber_vfatHitMul;
  std::vector<MonitorElement*> gem_chamber_stripHitMul;
  std::vector<MonitorElement*> gem_chamber_bestChi2;
  std::vector<MonitorElement*> gem_chamber_track;

  MonitorElement* gemcr_g;
  MonitorElement* gem_cls_tot;
  MonitorElement* gem_bx_tot;
  MonitorElement* tr_size;
  MonitorElement* tr_hit_size;
  MonitorElement* tr_chamber;
  MonitorElement* th_chamber;
  MonitorElement* rh_chamber;
  MonitorElement* rh1_chamber;
  MonitorElement* rh2_chamber;
  MonitorElement* rh3_chamber;
  MonitorElement* trajectoryh;
  MonitorElement* firedMul;
  MonitorElement* firedChamber;
  MonitorElement* NumberOfSeeds;



  std::vector<GEMChamber> gemChambers;
  int n_ch;
  MuonServiceProxy* theService;
  CosmicMuonSmoother* theSmoother;
  KFUpdator* theUpdator;
  std::vector<int> refChambers;

  std::auto_ptr<std::vector<TrajectorySeed> > findSeeds(MuonRecHitContainerLayered &muRecHits);
  Trajectory makeTrajectory(TrajectorySeed seed, MuonTransientTrackingRecHit::MuonRecHitContainer &muRecHits, std::vector<GEMChamber> gemChambers, GEMChamber testChamber);
  edm::EDGetToken InputTagToken_, InputTagToken_RH, InputTagToken_TR, InputTagToken_TS, InputTagToken_DG;

  // ----------member data ---------------------------      

  //   static unsigned int SeedsNumber; 

};

#endif
