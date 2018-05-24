#ifndef GEMCosmicMuonStandSim_H
#define GEMCosmicMuonStandSim_H

#include "FWCore/PluginManager/interface/ModuleDef.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHit.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"

#include "TFile.h"
#include "TTree.h"

#include <vector>

class GEMCosmicMuonStandSim : public DQMEDAnalyzer {
 public:
  explicit GEMCosmicMuonStandSim( const edm::ParameterSet& );
  ~GEMCosmicMuonStandSim();
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;

  void analyze(const edm::Event& e, const edm::EventSetup&) override;

  TString ConvertTitleToName(TString const& title);

  MonitorElement* BookHist1D(DQMStore::IBooker &ibooker,
                             TString title,
                             Int_t nchX, Double_t lowX, Double_t highX);


  MonitorElement* BookHist1D(DQMStore::IBooker &ibooker,
                             TString title,
                             Int_t lowX, Int_t highX);

  MonitorElement* BookHist2D(DQMStore::IBooker &ibooker,
                             TString title,
                             Int_t nchX, Double_t lowX, Double_t highX,
                             Int_t nchY, Double_t lowY, Double_t highY);

  MonitorElement* BookHist2D(DQMStore::IBooker &ibooker,
                             TString title,
                             Int_t lowX, Int_t highX,
                             Int_t lowY, Int_t highY);

  // Utility functions
  Int_t GetVFATId(Float_t x, const GEMEtaPartition* roll);
  Int_t GetChamberIndex(Int_t chamber_id) {return (chamber_id - 1) / 2; }
  Int_t GetOverallVFATPlotY(Int_t roll_id, Int_t vfat_id) {return 3 * roll_id + vfat_id - 3;}
  // conversion_factor = 10 deg * ( TMath::Pi() / 180 deg ) / 384 = 0.00045451283
  Float_t GetLocalPhi(Float_t strip) {return 0.00045451283 * (strip - 192);}

 private:
  edm::EDGetTokenT<edm::PSimHitContainer> sim_hit_token_;
  edm::EDGetTokenT<GEMRecHitCollection> rec_hit_token_;

  // Global constants
  const Int_t kMinChamberId_ = 1, kMaxChamberId_ = 29;
  const Int_t kMinRollId_ = 1, kMaxRollId_ = 8;
  const Int_t kMinVFATId_ = 1, kMaxVFATId_ = 3;

  const Int_t kNumRow_ = 5, kNumColumn_ = 3; // # of rows and columns in the stand
  const Int_t kNumSuperchamber_ = 15; // # of superchambers in the stand
  const Int_t kNumLayer_ = 2; // # of layers (chambers) in each superchamber
  const Int_t kNumRoll_ = 8; // # of roll (eta partition) in each chamber
  const Int_t kNumVFAT_ = 3; // # of VFATs in each roll.
  const Int_t kNumStrip_ = 384; // # of strips in each roll.

  const Int_t k1Deg_ = TMath::Pi() / 180.0;


  // Monitor Element

  // passed and total for efficiency
  MonitorElement *me_vfat_passed_overall_, *me_vfat_total_overall_;
  MonitorElement *me_vfat_passed_[15][2]; // [superchamber][layer]
  MonitorElement *me_vfat_total_[15][2];

  // occupancy
  MonitorElement *me_occ_overall_, *me_occ_overall_v2_;
  MonitorElement *me_vfat_occ_[15][2];

  // residual, error, pull
  MonitorElement *me_res_x_overall_, *me_res_y_overall_, *me_res_phi_overall_;
  MonitorElement *me_error_x_overall_, *me_error_y_overall_;
  MonitorElement *me_pull_x_overall_, *me_pull_y_overall_;

  MonitorElement *me_res_x_[15][2][8][3];
  MonitorElement *me_res_y_[15][2][8][3];
  MonitorElement *me_res_phi_[15][2][8][3];
  MonitorElement *me_pull_x_[15][2][8][3];
  MonitorElement *me_pull_y_[15][2][8][3];

  // extra
  MonitorElement *me_cls_, *me_cls_vs_chamber_;
  MonitorElement *me_num_clusters_, *me_num_clusters_vs_chamber_;
  MonitorElement *me_num_sim_hits_, *me_num_rec_hits_;
  MonitorElement *me_sim_phi_, *me_rec_phi_;
  MonitorElement *me_sim_rec_distance_;
  MonitorElement *me_sim_strip_, *me_rec_first_strip_; //, *me_strip_diff_, ;
};

DEFINE_FWK_MODULE (GEMCosmicMuonStandSim) ;
#endif
