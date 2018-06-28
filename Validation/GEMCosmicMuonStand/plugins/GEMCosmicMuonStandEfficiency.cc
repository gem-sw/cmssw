#include "GEMCosmicMuonStandEfficiency.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "DataFormats/TrackingRecHit/interface/TrackingRecHitFwd.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"

#include "GEMResiduals.h"
#include <iostream>

using namespace std;

GEMCosmicMuonStandEfficiency::GEMCosmicMuonStandEfficiency(const edm::ParameterSet& cfg)
{
  insideOutTracks_ = consumes<reco::TrackCollection>(cfg.getParameter<edm::InputTag>("insideOutTracks"));
  outsideInTracks_ = consumes<reco::TrackCollection>(cfg.getParameter<edm::InputTag>("outsideInTracks"));
  insideOutTrajs_ = consumes<vector<Trajectory>>(cfg.getParameter<edm::InputTag>("insideOutTracks"));
  outsideInTrajs_ = consumes<vector<Trajectory>>(cfg.getParameter<edm::InputTag>("outsideInTracks"));
  seedInside_ = consumes<vector<TrajectorySeed>>(cfg.getParameter<edm::InputTag>("insideOutTracks"));
  seedOutside_ = consumes<vector<TrajectorySeed>>(cfg.getParameter<edm::InputTag>("outsideInTracks"));
}

MonitorElement* GEMCosmicMuonStandEfficiency::BookHist1D( DQMStore::IBooker &ibooker, const char* name, const char* label, unsigned int row, unsigned int coll, unsigned int layer_num, unsigned int vfat_num, const unsigned int Nbin, const Float_t xMin, const Float_t xMax)
{
  string hist_name = name+row+coll+layer_num+vfat_num;
  string hist_label;
  // hist_name.Format("{} {} {} {} {}", name, row, coll, layer_num, vfat_num);
  // hist_label.Format("{} {} {} {} {}", label, row, coll, layer_num, vfat_num);
  return ibooker.book1D( hist_name, hist_label,Nbin,xMin,xMax ); 
}

MonitorElement* GEMCosmicMuonStandEfficiency::BookHist1D( DQMStore::IBooker &ibooker, const char* name, const char* label, const unsigned int Nbin, const Float_t xMin, const Float_t xMax)
{
  return ibooker.book1D( name, label,Nbin,xMin,xMax );
}

void GEMCosmicMuonStandEfficiency::bookHistograms(DQMStore::IBooker & ibooker, edm::Run const & Run, edm::EventSetup const & iSetup )
{
  edm::ESHandle<GEMGeometry> hGeom;
  iSetup.get<MuonGeometryRecord>().get(hGeom);
  const GEMGeometry*  GEMGeometry_ = &*hGeom;
  if ( GEMGeometry_ == nullptr) return ;  

  LogDebug("GEMCosmicMuonStandEfficiency")<<"Geometry is acquired from MuonGeometryRecord\n";
  ibooker.setCurrentFolder("GEMCosmicMuonStandEfficiency");
  LogDebug("GEMCosmicMuonStandEfficiency")<<"ibooker set current folder\n";

  // for( auto& region : GEMGeometry_->regions() ){
  //   int re = region->region();
  // }
  for(int i = 0; i < 15; i++)
  {
    for(int j = 0; j < 2; j++)
    {
      std::string chamber = "Chamber" + to_string(2*i+1) + "_layer" + to_string(j+1);
      std::string temp1;
      temp1 = "Valid_Hit_" + chamber;
      gem_vfat_eff[i][j] = BookHist1D(ibooker, temp1.c_str(), temp1.c_str(), 24, -0.5, 23.5);
      temp1 = "Total_Hit_" + chamber;
      gem_vfat_tot[i][j] = BookHist1D(ibooker, temp1.c_str(), temp1.c_str(), 24, -0.5, 23.5);
      for(int k = 0; k < 24; k++)
      {
        std::string vfat = chamber + "_" + to_string(k);
        temp1 = "residualX_" + vfat;
        residualX_[i][j][k] = BookHist1D(ibooker, temp1.c_str(), temp1.c_str(), 100, -5, 5);
        temp1 = "residualY_" + vfat;
        residualY_[i][j][k] = BookHist1D(ibooker, temp1.c_str(), temp1.c_str(), 100, -15, 15);
        temp1 = "pullX_" + vfat;
        pullX_[i][j][k] = BookHist1D(ibooker, temp1.c_str(), temp1.c_str(), 100, -5, 5);
        temp1 = "pullY_" + vfat;
        pullY_[i][j][k] = BookHist1D(ibooker, temp1.c_str(), temp1.c_str(), 100, -5, 5);
      }
    }
  }

  chi2[0] = BookHist1D(ibooker, "insideChi2", "insideChi2", 1000, 0, 100);
  chi2[1] = BookHist1D(ibooker, "outsideChi2", "outsideChi2", 1000, 0, 100);
  residualX_tot = BookHist1D(ibooker, "residualX", "residualX", 1000, 0, 200);
  residualY_tot = BookHist1D(ibooker, "residualY", "residualY", 1000, 0, 200);
  pullX_tot = BookHist1D(ibooker, "pullX", "pullX", 1000, 0, 200);
  pullY_tot = BookHist1D(ibooker, "pullY", "pullY", 1000, 0, 200);

  LogDebug("GEMCosmicMuonStandEfficiency")<<"Booking End.\n";
}

void GEMCosmicMuonStandEfficiency::analyze(const edm::Event& e,const edm::EventSetup& iSetup)
{
  edm::ESHandle<GEMGeometry> hGeom;
  iSetup.get<MuonGeometryRecord>().get(hGeom);
  const GEMGeometry*  GEMGeometry_ = &*hGeom;
  if ( GEMGeometry_ == nullptr) return ;  

  edm::Handle<reco::TrackCollection> insideOutTracks;
  e.getByToken( insideOutTracks_, insideOutTracks);

  edm::Handle<reco::TrackCollection> outsideInTracks;
  e.getByToken( outsideInTracks_, outsideInTracks);

  edm::Handle<vector<Trajectory>> insideOutTrajs;
  e.getByToken( insideOutTrajs_, insideOutTrajs);

  edm::Handle<vector<Trajectory>> outsideInTrajs;
  e.getByToken( outsideInTrajs_, outsideInTrajs);
  
  edm::Handle<vector<TrajectorySeed>> seedInside;
  e.getByToken( seedInside_, seedInside);

  edm::Handle<vector<TrajectorySeed>> seedOutside;
  e.getByToken( seedOutside_, seedOutside);

  // Analysis inside out tracks 
  // analyzeTrk(true, GEMGeometry_, insideOutTracks, insideOutTrajs, seedInside);
  // Analysis outside in tracks 
  analyzeTrk(false, GEMGeometry_, outsideInTracks, outsideInTrajs, seedOutside);
}

void GEMCosmicMuonStandEfficiency::analyzeTrk(bool isInside, 
                                              const GEMGeometry* GEMGeometry_,
                                              const edm::Handle<reco::TrackCollection> &tracks, 
                                              const edm::Handle<vector<Trajectory>> &trajectorys, 
                                              const edm::Handle<vector<TrajectorySeed>> &seed)
{
  vector<TrajectorySeed>::const_iterator seeds = seed->begin();
  vector<Trajectory>::const_iterator traj = trajectorys->begin();

  for (std::vector<reco::Track>::const_iterator track = tracks->begin(); track != tracks->end(); ++track, ++seeds, ++traj)
  {
    auto residuals_ = GEMResidualCalc(*traj);
    
    auto seed = ((*seeds).recHits()).first;
    GEMDetId firstHit(seed->rawId());
    seed++;
    GEMDetId secondHit(seed->rawId());
    if((firstHit.chamber()-1)/10 != (secondHit.chamber()-1)/10) continue;

    if(isInside) chi2[0]->Fill(track->normalizedChi2());
    else         chi2[1]->Fill(track->normalizedChi2());

    int count = 0;
    for (trackingRecHit_iterator recHit = track->recHitsBegin(); recHit != track->recHitsEnd(); ++recHit, ++count)
    {
      GEMDetId gemId((*recHit)->rawId());
      auto etaPartition = GEMGeometry_->etaPartition(gemId);
      int chamber = etaPartition->id().chamber();
      int layer = etaPartition->id().layer();
      int roll = etaPartition->id().roll();
      int nStrips = etaPartition->nstrips();
      float strip = etaPartition->strip((*recHit)->localPosition());

      int idxChamber = (chamber-1)/2;
      int idxLayer = layer-1;
      int vfat = (8-roll)+int(strip/nStrips*3)*8;

      if(chamber == firstHit.chamber() and layer == firstHit.layer()) continue;
      if(chamber == secondHit.chamber() and layer == secondHit.layer()) continue;
      
      if((*recHit)->isValid()) 
      {
        gem_vfat_eff[idxChamber][idxLayer]->Fill(vfat);
        residualX_[idxChamber][idxLayer][vfat]->Fill(residuals_.residualX[count]);
        residualY_[idxChamber][idxLayer][vfat]->Fill(residuals_.residualY[count]);
        pullX_[idxChamber][idxLayer][vfat]->Fill(residuals_.pullX[count]);
        pullY_[idxChamber][idxLayer][vfat]->Fill(residuals_.pullY[count]);
        residualX_tot->Fill(std::abs(residuals_.residualX[count]));
        residualY_tot->Fill(std::abs(residuals_.residualY[count]));
        pullX_tot->Fill(std::abs(residuals_.pullX[count]));
        pullY_tot->Fill(std::abs(residuals_.pullY[count]));
      }
      gem_vfat_tot[idxChamber][idxLayer]->Fill(vfat);
    }
  }
}
