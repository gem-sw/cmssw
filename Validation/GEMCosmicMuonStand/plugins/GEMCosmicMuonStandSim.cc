#include "GEMCosmicMuonStandSim.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "SimDataFormats/TrackingHit/interface/PSimHit.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "DataFormats/GeometryVector/interface/LocalPoint.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHit.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartition.h"

#include "TMath.h"

#include <vector>
#include <iostream>
#include <iterator> // std::distance
#include <algorithm> // std::find, std::transform, std::replace_if, std::min
#include <numeric> // std::iota
#include <string>
#include <cmath> // std::remainder, std::fabs

using namespace std;

GEMCosmicMuonStandSim::GEMCosmicMuonStandSim(const edm::ParameterSet& cfg)
{
  sim_hit_token_ = consumes<edm::PSimHitContainer>(cfg.getParameter<edm::InputTag>("simHitToken"));
  rec_hit_token_ = consumes<GEMRecHitCollection>(cfg.getParameter<edm::InputTag>("recHitToken"));
}


GEMCosmicMuonStandSim::~GEMCosmicMuonStandSim() {}


TString GEMCosmicMuonStandSim::ConvertTitleToName(TString const& title)
{
  std::string tmp_name = title.Data();
  std::replace_if(tmp_name.begin(), tmp_name.end(), ::ispunct, '_');
  std::replace_if(tmp_name.begin(), tmp_name.end(), ::isspace, '_');
  std::transform(tmp_name.begin(), tmp_name.end(), tmp_name.begin(), ::tolower);
  TString name = tmp_name;
  name.ReplaceAll("__", "_");
  name = name.Strip(TString::EStripType::kBoth, '_');
  return name;
}


MonitorElement* GEMCosmicMuonStandSim::BookHist1D(DQMStore::IBooker &ibooker,
                                                  TString title,
                                                  Int_t nchX, Double_t lowX, Double_t highX)
{
  TString name = ConvertTitleToName(title);
  return ibooker.book1D(name, title, nchX, lowX, highX);
}


MonitorElement* GEMCosmicMuonStandSim::BookHist1D(DQMStore::IBooker &ibooker,
                                                  TString title,
                                                  Int_t lowX, Int_t highX)
{
  Int_t nchX = highX - lowX + 1;
  TString name = ConvertTitleToName(title);
  return ibooker.book1D(name, title, nchX, lowX - 0.5, highX + 0.5);
}


MonitorElement* GEMCosmicMuonStandSim::BookHist2D(DQMStore::IBooker &ibooker,
                                                TString title,
                                                Int_t nchX, Double_t lowX, Double_t highX,
                                                Int_t nchY, Double_t lowY, Double_t highY)
{
  TString name = ConvertTitleToName(title);
  return ibooker.book2D(name, title, nchX, lowX, highX, nchY, lowY, highY);
}


MonitorElement* GEMCosmicMuonStandSim::BookHist2D(DQMStore::IBooker &ibooker,
                                                TString title,
                                                Int_t lowX, Int_t highX,
                                                Int_t lowY, Int_t highY)
{
  Int_t nchX = highX - lowX + 1;
  Int_t nchY = highY - lowY + 1;
  TString name = ConvertTitleToName(title);
  return ibooker.book2D(name, title, nchX, lowX - 0.5, highX + 0.5, nchY, lowY - 0.5, highY + 0.5);
}

void GEMCosmicMuonStandSim::bookHistograms(DQMStore::IBooker & ibooker,
                                           edm::Run const & Run,
                                           edm::EventSetup const & iSetup)
{
  edm::ESHandle<GEMGeometry> hGeom;
  iSetup.get<MuonGeometryRecord>().get(hGeom);

  const GEMGeometry*  GEMGeometry_ = &*hGeom;
  if ( GEMGeometry_ == nullptr) return ;  

  LogDebug("GEMCosmicMuonStandSim") << "Geometry is acquired from MuonGeometryRecord\n";
  ibooker.setCurrentFolder("GEMCosmicMuonStandSim");

  LogDebug("GEMCosmicMuonStandSim") << "ibooker set current folder\n";

  me_vfat_total_overall_ = BookHist2D(ibooker, "The Number Of Total Events", 0, 14, 1, 3*8);
  me_vfat_passed_overall_ = BookHist2D(ibooker, "The Number Of Passed Events", 0, 14, 1, 3*8);

  me_occ_overall_ = BookHist2D(ibooker, "Occupancy Ver1", 0, 14, 1, 3*8);
  me_occ_overall_v2_ = BookHist2D(ibooker, "Occupancy Ver2", 1, 384*3, 1, 8*5*2);

  for(Int_t chamber_id = kMinChamberId_; chamber_id <= kMaxChamberId_; chamber_id += 2)
  {
    Int_t chamber_index = GetChamberIndex(chamber_id); // chamber index

    for(Int_t layer_id : {1, 2})
    {
      Int_t layer_index = layer_id - 1;

      TString det_suffix = TString::Format(" (Chamber %d Layer %d)", chamber_id, layer_id);

      me_vfat_total_[chamber_index][layer_index] = BookHist2D(ibooker,
       "The Number Of Total Events" + det_suffix,
        kMinVFATId_, kMaxVFATId_,
        kMinRollId_, kMaxRollId_);

      me_vfat_passed_[chamber_index][layer_index] = BookHist2D(ibooker,
        "The Number Of Passed Events" + det_suffix,
        kMinVFATId_, kMaxVFATId_,
        kMinRollId_, kMaxRollId_);

      me_vfat_occ_[chamber_index][layer_index] = BookHist2D(ibooker,
        "Occupancy" + det_suffix,
        1, 384,
        kMinRollId_, kMaxRollId_);
    }
  }

  // Residual, Error and Pull
  me_res_x_overall_ = BookHist1D(ibooker, "Residual X", 101, -1, 1);
  me_res_y_overall_ = BookHist1D(ibooker, "Residual Y", 101, -10, 10);
  me_res_phi_overall_ = BookHist1D(ibooker, "Residual Phi", 101, -0.1*k1Deg_, 0.1*k1Deg_);

  me_error_x_overall_ = BookHist1D(ibooker, "Error X", 100, 0, 0.3);
  me_error_y_overall_ = BookHist1D(ibooker, "Error Y", 5000, 0, 500);

  me_pull_x_overall_ = BookHist1D(ibooker, "Pull X", 101, -30, 30);
  me_pull_y_overall_ = BookHist1D(ibooker, "Pull Y", 101, -0.6, 0.6);

  for(Int_t chamber_id = kMinChamberId_; chamber_id <= kMaxChamberId_; chamber_id += 2)
  {
    Int_t chamber_index = GetChamberIndex(chamber_id); // chamber index

    for(Int_t layer_id : {1, 2})
    {
      Int_t layer_index = layer_id - 1;

      for(Int_t roll_id = kMinRollId_; roll_id <= kMaxRollId_; ++roll_id)
      {
        Int_t roll_index = roll_id - 1;

        for(Int_t vfat_id = kMinVFATId_; vfat_id <= kMaxVFATId_; ++vfat_id)
        {
          Int_t vfat_index = vfat_id - 1;

          TString det_suffix = TString::Format(" (Chamber %d Layer %d Roll %d VFAT %d)",
                                           chamber_id, layer_id, roll_id, vfat_id);

          me_res_x_[chamber_index][layer_index][roll_index][vfat_index] = BookHist1D(
            ibooker, "Residual X" + det_suffix, 101, -1, 1);

          me_res_y_[chamber_index][layer_index][roll_index][vfat_index] = BookHist1D(
            ibooker, "Residual Y" + det_suffix, 101, -10, 10);

          me_res_phi_[chamber_index][layer_index][roll_index][vfat_index] = BookHist1D(
            ibooker, "Residual Phi" + det_suffix, 101, -0.1*k1Deg_, 0.1*k1Deg_);

          me_pull_x_[chamber_index][layer_index][roll_index][vfat_index] = BookHist1D(
            ibooker, "Pull X" + det_suffix, 101, -30, 30);

          me_pull_y_[chamber_index][layer_index][roll_index][vfat_index] = BookHist1D(
            ibooker, "Pull Y" + det_suffix, 101, -0.6, 0.6);
        }
      }
    }
  }

  me_cls_ = BookHist1D(ibooker, "The Cluster Size of RecHit", 1, 10);
  me_cls_vs_chamber_ = BookHist2D(ibooker, "CLS vs Chamber", 1, 15, 1, 10);
  me_num_clusters_ = BookHist1D(ibooker, "The Number of Clusters", 1, 8);
  me_num_clusters_vs_chamber_ = BookHist2D(ibooker, "Number of Cluster vs Chamber", 1, 15, 1, 10);

  me_num_sim_hits_ = BookHist1D(ibooker, "The Number of SimHits", 1, 25);
  me_num_rec_hits_ = BookHist1D(ibooker, "The Number of RecHits", 1, 25);

  me_sim_phi_ = BookHist1D(ibooker, "Local Phi of SimHits", 25, -1 * TMath::Pi() / 18, TMath::Pi() / 18);
  me_rec_phi_ = BookHist1D(ibooker, "Local Phi of RecHits", 25, -1 * TMath::Pi() / 18, TMath::Pi() / 18);

  me_sim_strip_ = BookHist1D(ibooker, "Sim Fired Strip", 0 , 384); // 0 ~ 384
  me_rec_first_strip_ = BookHist1D(ibooker, "Rec First Fired Strip", 0 , 384);
  // me_strip_diff_ = BookHist1D(ibooker, "Difference between RecStrip and SimStrip", 101, -50.5, 50.5);

  LogDebug("GEMCosmicMuonStandSim") << "Booking End.\n";
}


Int_t GEMCosmicMuonStandSim::GetVFATId(Float_t x, const GEMEtaPartition* roll)
{
  Int_t nstrips = roll->nstrips();
  Float_t x_min = roll->centreOfStrip(1).x(); // - strip width
  Float_t x_max = roll->centreOfStrip(nstrips).x(); // + strip width

  Float_t x0 = std::min(x_min, x_max);

  Float_t width = std::fabs(x_max - x_min) / kNumVFAT_;

  if (x < x0 + width)        return 1;
  else if (x < x0 + 2 * width) return 2;
  else if (x < x0 + 3 * width) return 3;
  else                       return -1;
}


void GEMCosmicMuonStandSim::analyze(const edm::Event& e,
                                    const edm::EventSetup& iSetup)
{
  edm::ESHandle<GEMGeometry> hGeom;
  iSetup.get<MuonGeometryRecord>().get(hGeom);
  const GEMGeometry*  GEMGeometry_ = &*hGeom;
  if ( GEMGeometry_ == nullptr) return ;  

  edm::Handle<GEMRecHitCollection> gem_rec_hits;
  e.getByToken(rec_hit_token_, gem_rec_hits);
  edm::Handle<edm::PSimHitContainer> gem_sim_hits;
  e.getByToken(sim_hit_token_, gem_sim_hits);
  
  if (not gem_rec_hits.isValid())
  {
    edm::LogError("GEMCosmicMuonStandSim") << "Cannot get strips by Token RecHits Token.\n";
    return ;
  }

  // if( isMC) 
  Int_t num_sim_hits = gem_sim_hits->size();
  me_num_sim_hits_->Fill(num_sim_hits);
  if(num_sim_hits == 0) return ;

  Int_t num_rec_hits = std::distance(gem_rec_hits->begin(), gem_rec_hits->end());
  me_num_rec_hits_->Fill(num_rec_hits);

  for(edm::PSimHitContainer::const_iterator sim_hit = gem_sim_hits->begin(); sim_hit != gem_sim_hits->end(); ++sim_hit)
  {
    Local3DPoint sim_hit_lp = sim_hit->localPosition();
    GEMDetId sim_det_id(sim_hit->detUnitId());

    Int_t sim_fired_strip = GEMGeometry_->etaPartition(sim_det_id)->strip(sim_hit_lp) + 1;
    me_sim_strip_->Fill(sim_fired_strip);

    const GEMEtaPartition* kSimRoll = GEMGeometry_->etaPartition(sim_det_id);
    Int_t sim_vfat_id = GetVFATId(sim_hit_lp.x(), kSimRoll);

    Int_t chamber_index = GetChamberIndex(sim_det_id.chamber()); // 0 ~ 14
    Int_t layer_index = sim_det_id.layer() - 1;
    Int_t roll_index = sim_det_id.roll() - 1;
    Int_t vfat_index = sim_vfat_id - 1;

    Int_t y_overall_vfat_plot = GetOverallVFATPlotY(sim_det_id.roll(), sim_vfat_id);
    me_vfat_total_overall_->Fill(chamber_index, y_overall_vfat_plot);
    me_vfat_total_[chamber_index][layer_index]->Fill(sim_vfat_id, sim_det_id.roll());

    GEMRecHitCollection::range range = gem_rec_hits->get(sim_det_id);
    Int_t num_clusters = std::distance(range.first, range.second);
    me_num_clusters_->Fill(num_clusters);

    for(GEMRecHitCollection::const_iterator rec_hit = range.first; rec_hit != range.second; ++rec_hit)
    {
      Int_t rec_cls = rec_hit->clusterSize();

      // Checkt whether a sim. fired strip is in a rec. cluster strips.
      Bool_t is_matched_hit;
      if ( rec_cls == 1 )
      {
        is_matched_hit = sim_fired_strip == rec_hit->firstClusterStrip();
        me_rec_first_strip_->Fill(sim_fired_strip);
      }
      else
      {
        Int_t rec_first_fired_strip = rec_hit->firstClusterStrip();
        Int_t rec_last_fired_strip = rec_first_fired_strip + rec_cls - 1;
        is_matched_hit = (sim_fired_strip >= rec_first_fired_strip) and (sim_fired_strip <= rec_last_fired_strip);
        me_rec_first_strip_->Fill(rec_first_fired_strip);
      }

      if( is_matched_hit )
      {
        LocalPoint rec_hit_lp = rec_hit->localPosition();
        GEMDetId rec_det_id = rec_hit->gemId();

        const GEMEtaPartition* kRecRoll = GEMGeometry_->etaPartition(rec_det_id);
        
        me_vfat_passed_overall_->Fill(chamber_index, y_overall_vfat_plot);
        me_vfat_passed_[chamber_index][layer_index]->Fill(sim_vfat_id, sim_det_id.roll());

        // Residual
        Float_t res_x = rec_hit_lp.x() - sim_hit_lp.x();
        Float_t res_y = rec_hit_lp.y() - sim_hit_lp.y();
        // Residual Phi
        Float_t sim_hit_local_phi = GetLocalPhi(kRecRoll->strip(sim_hit_lp));
        Float_t rec_hit_local_phi = GetLocalPhi(kRecRoll->strip(rec_hit_lp));
        Float_t res_phi = rec_hit_local_phi - sim_hit_local_phi;
        // Local Position Error
        Float_t error_x = rec_hit->localPositionError().xx();
        Float_t error_y = rec_hit->localPositionError().yy();
        // Pull
        Float_t pull_x = res_x / error_x;
        Float_t pull_y = res_y / error_y;

        me_res_x_overall_->Fill(res_x);
        me_res_y_overall_->Fill(res_y);
        me_res_phi_overall_->Fill(res_phi);
        me_error_x_overall_->Fill(error_x);
        me_error_y_overall_->Fill(error_y);
        me_pull_x_overall_->Fill(pull_x);
        me_pull_y_overall_->Fill(pull_y);

        me_res_x_[chamber_index][layer_index][roll_index][vfat_index]->Fill(res_x);
        me_res_y_[chamber_index][layer_index][roll_index][vfat_index]->Fill(res_y);
        me_res_phi_[chamber_index][layer_index][roll_index][vfat_index]->Fill(res_phi);
        me_pull_x_[chamber_index][layer_index][roll_index][vfat_index]->Fill(pull_x);
        me_pull_y_[chamber_index][layer_index][roll_index][vfat_index]->Fill(pull_y);

        me_cls_->Fill(rec_cls);
        me_cls_vs_chamber_->Fill(chamber_index, rec_cls);
        me_num_clusters_vs_chamber_->Fill(chamber_index, num_clusters);
        me_sim_phi_->Fill(sim_hit_local_phi);
        me_rec_phi_->Fill(rec_hit_local_phi);

        break;
      }
    } // rechit loop end
  } // simhit loop end

  for(GEMRecHitCollection::const_iterator rec_hit = gem_rec_hits->begin(); rec_hit != gem_rec_hits->end(); ++rec_hit)
  {
    LocalPoint rec_hit_lp = rec_hit->localPosition();
    GEMDetId rec_det_id = rec_hit->gemId();

    const GEMEtaPartition* kRecRoll = GEMGeometry_->etaPartition(rec_det_id);

    Int_t chamber_index = GetChamberIndex(rec_det_id.chamber());
    Int_t layer_index = rec_det_id.layer() - 1;

    Int_t rec_vfat_id = GetVFATId(rec_hit_lp.x(), kRecRoll);

    Int_t y_overall_vfat_plot = GetOverallVFATPlotY(rec_det_id.roll(), rec_vfat_id);
    me_occ_overall_->Fill(chamber_index, y_overall_vfat_plot);

    Int_t rec_first_fired_strip = rec_hit->firstClusterStrip();
    Int_t rec_cls = rec_hit->clusterSize();
    Int_t rec_last_fired_strip = rec_first_fired_strip + rec_cls - 1;

    for(Int_t strip=rec_first_fired_strip; strip <= rec_last_fired_strip; strip++)
    {
      me_vfat_occ_[chamber_index][layer_index]->Fill(strip, rec_det_id.roll());

      Int_t x_overall_v2 = kNumStrip_ * (rec_det_id.chamber() / 10) + strip;
      Int_t y_overall_v2 = 2 * kNumRoll_ * ( ( (rec_det_id.chamber() % 10) - 1) / 2 ) \
                           + kNumRoll_ * (rec_det_id.layer() - 1) \
                           + rec_det_id.roll();

      me_occ_overall_v2_->Fill(x_overall_v2, y_overall_v2);
    }
  }
} // analyze end
