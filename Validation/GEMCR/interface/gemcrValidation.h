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
  const GEMGeometry* initGeometry(edm::EventSetup const & iSetup);

private:
  const GEMGeometry* GEMGeometry_;
  std::vector<MonitorElement*> gem_chamber_x_y;
  std::vector<MonitorElement*> gem_chamber_cl_size;
  std::vector<MonitorElement*> gem_chamber_bx;
  std::vector<MonitorElement*> gem_chamber_firedStrip;
  std::vector<MonitorElement*> gem_chamber_tr_eff;
  std::vector<MonitorElement*> gem_chamber_th_eff;
  std::vector<MonitorElement*> gem_chamber_tr2D_eff;
  std::vector<MonitorElement*> gem_chamber_th2D_eff;
  std::vector<MonitorElement*> gem_chamber_rec_eff;
  std::vector<MonitorElement*> gem_chamber_trroll_eff;
  std::vector<MonitorElement*> gem_chamber_throll_eff;
  
  MonitorElement* gemcr_g;
  
  MonitorElement* gem_cls_tot;
  MonitorElement* gem_bx_tot;
  MonitorElement* tr_size;
  MonitorElement* tr_hit_size;

  MonitorElement* tr_chamber;
  MonitorElement* th_chamber;
  MonitorElement* rh_chamber;
  MonitorElement* sh_chamber;

  MonitorElement* del_x;
  MonitorElement* del_y;

  MonitorElement* del_rx;
  MonitorElement* del_ry;

  TFile* effOut = new TFile("gemEff.root", "recreate");
  TEfficiency* bEff =  new  TEfficiency("biErr", "TR/TH", 10,0,10);
  TEfficiency* bEff2 =  new  TEfficiency("biErr2", "RH/TH", 10,0,10);
  TEfficiency* bEff3 =  new  TEfficiency("biErr3", "TR/RH", 10,0,10);
  std::vector<GEMChamber> gemChambers;
  int n_ch;
  MuonServiceProxy* theService;

  edm::EDGetToken InputTagToken_, InputTagToken_RH, InputTagToken_TR, InputTagToken_TS, InputTagToken_GP;
  
  double numSim1, numRec1, numSim2, numRec2, numSim3, numRec3, numTH, numTR, numTHfp, numTRfp, countNum;
};

#endif
