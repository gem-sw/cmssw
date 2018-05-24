#ifndef GEMCosmicMuonStandEfficiency_H
#define GEMCosmicMuonStandEfficiency_H

#include "FWCore/PluginManager/interface/ModuleDef.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrajectorySeed/interface/PropagationDirection.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeed.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHit.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"

#include "TrackingTools/PatternTools/interface/Trajectory.h"

using namespace std;

class GEMCosmicMuonStandEfficiency : public DQMEDAnalyzer
{
public:
explicit GEMCosmicMuonStandEfficiency( const edm::ParameterSet& );
void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
void analyze(const edm::Event& e, const edm::EventSetup&) override;
MonitorElement* BookHist1D( DQMStore::IBooker &, const char* name, const char* label, unsigned int row, unsigned int coll, unsigned int layer_num, unsigned int vfat_num, const unsigned int Nbin, const Float_t xMin, const Float_t xMax);
MonitorElement* BookHist1D( DQMStore::IBooker &, const char* name, const char* label, const unsigned int Nbin, const Float_t xMin, const Float_t xMax);

void analyzeTrk(bool, const GEMGeometry*, const edm::Handle<reco::TrackCollection> &, const edm::Handle<vector<Trajectory>> &, const edm::Handle<vector<TrajectorySeed>> &);

private:

MonitorElement* gem_vfat_eff[15][2];
MonitorElement* gem_vfat_tot[15][2];

MonitorElement* chi2[2];
MonitorElement* residualX_tot;
MonitorElement* residualY_tot;
MonitorElement* pullX_tot;
MonitorElement* pullY_tot;
MonitorElement* residualX_[15][2][24];
MonitorElement* residualY_[15][2][24];
MonitorElement* pullX_[15][2][24];
MonitorElement* pullY_[15][2][24];

edm::EDGetTokenT<reco::TrackCollection> insideOutTracks_, outsideInTracks_;
edm::EDGetTokenT<vector<Trajectory>> insideOutTrajs_, outsideInTrajs_;
edm::EDGetTokenT<vector<TrajectorySeed>> seedInside_, seedOutside_;

};

DEFINE_FWK_MODULE (GEMCosmicMuonStandEfficiency) ;
#endif

