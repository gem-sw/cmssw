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
#include <TFile.h>
#include <TEfficiency.h>
#include <TH1D.h>
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

private:
  const GEMGeometry* GEMGeometry_;
  std::vector<MonitorElement*> gem_chamber_x_y;
  std::vector<MonitorElement*> gem_chamber_cl_size;
  std::vector<MonitorElement*> gem_chamber_bx;
  std::vector<MonitorElement*> gem_chamber_firedStrip;
  std::vector<MonitorElement*> gem_chamber_tr2D_eff;
  std::vector<MonitorElement*> gem_chamber_th2D_eff;
  
  MonitorElement* gemcr_g;
  MonitorElement* gem_cls_tot;
  MonitorElement* gem_bx_tot;
  MonitorElement* tr_size;
  MonitorElement* tr_hit_size;
  MonitorElement* tr_chamber;
  MonitorElement* th_chamber;
  MonitorElement* rh_chamber;

  std::vector<GEMChamber> gemChambers;
  int n_ch;
  
  MuonServiceProxy* theService;

  edm::EDGetToken InputTagToken_, InputTagToken_RH, InputTagToken_TR, InputTagToken_TS, InputTagToken_GP;
};

#endif
