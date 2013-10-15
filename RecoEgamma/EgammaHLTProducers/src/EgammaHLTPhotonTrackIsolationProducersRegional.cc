/** \class EgammaHLTPhotonTrackIsolationProducersRegional
 *
 *  \author Monica Vazquez Acosta (CERN)
 * 
 * $Id:
 *
 */

#include "RecoEgamma/EgammaHLTProducers/interface/EgammaHLTPhotonTrackIsolationProducersRegional.h"
#include "DataFormats/RecoCandidate/interface/RecoEcalCandidateIsolation.h"

#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

EgammaHLTPhotonTrackIsolationProducersRegional::EgammaHLTPhotonTrackIsolationProducersRegional(const edm::ParameterSet& config) : conf_(config)
{

  recoEcalCandidateProducer_    = consumes<reco::RecoEcalCandidateCollection> (conf_.getParameter<edm::InputTag>("recoEcalCandidateProducer"));
  trackProducer_                = consumes<reco::TrackCollection>(conf_.getParameter<edm::InputTag>("trackProducer"));

  countTracks_                  = conf_.getParameter<bool>("countTracks");

  egTrkIsoPtMin_                = conf_.getParameter<double>("egTrkIsoPtMin");
  egTrkIsoConeSize_             = conf_.getParameter<double>("egTrkIsoConeSize");
  egTrkIsoZSpan_                = conf_.getParameter<double>("egTrkIsoZSpan");
  egTrkIsoRSpan_                = conf_.getParameter<double>("egTrkIsoRSpan");
  egTrkIsoVetoConeSize_         = conf_.getParameter<double>("egTrkIsoVetoConeSize");
  double egTrkIsoStripBarrel    = conf_.getParameter<double>("egTrkIsoStripBarrel");
  double egTrkIsoStripEndcap    = conf_.getParameter<double>("egTrkIsoStripEndcap");
  
  test_ = new EgammaHLTTrackIsolation(egTrkIsoPtMin_,egTrkIsoConeSize_,
				      egTrkIsoZSpan_,egTrkIsoRSpan_,egTrkIsoVetoConeSize_,
				      egTrkIsoStripBarrel,egTrkIsoStripEndcap);


  //register your products
  produces < reco::RecoEcalCandidateIsolationMap >();

}

EgammaHLTPhotonTrackIsolationProducersRegional::~EgammaHLTPhotonTrackIsolationProducersRegional(){delete test_;}

void EgammaHLTPhotonTrackIsolationProducersRegional::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>(("recoEcalCandidateProducer"), edm::InputTag(""));
  desc.add<edm::InputTag>(("trackProducer"), edm::InputTag(""));
  desc.add<bool>(("countTracks"), true);
  desc.add<double>(("egTrkIsoPtMin"), 0);
  desc.add<double>(("egTrkIsoConeSize"), 0);
  desc.add<double>(("egTrkIsoZSpan"), 0);
  desc.add<double>(("egTrkIsoRSpan"), 0);
  desc.add<double>(("egTrkIsoVetoConeSize"), 0);
  desc.add<double>(("egTrkIsoStripBarrel"), 0);
  desc.add<double>(("egTrkIsoStripEndcap"), 0);
  descriptions.add(("hltEgammaHLTPhotonTrackIsolationProducersRegional"), desc);  
}
  

// ------------ method called to produce the data  ------------
void
EgammaHLTPhotonTrackIsolationProducersRegional::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  // Get the HLT filtered objects
  edm::Handle<reco::RecoEcalCandidateCollection> recoecalcandHandle;
  iEvent.getByToken(recoEcalCandidateProducer_,recoecalcandHandle);

 // Get the barrel hcal hits
  edm::Handle<reco::TrackCollection> trackHandle;
  iEvent.getByToken(trackProducer_, trackHandle);
  const reco::TrackCollection* trackCollection = trackHandle.product();

  reco::RecoEcalCandidateIsolationMap isoMap;
  
  for(unsigned int iRecoEcalCand=0; iRecoEcalCand<recoecalcandHandle->size(); iRecoEcalCand++) {
    
    reco::RecoEcalCandidateRef recoecalcandref(recoecalcandHandle, iRecoEcalCand);

    bool usePhotonVertex = false;
     
    float isol;
    if (countTracks_) {
      isol = test_->photonTrackCount(&(*recoecalcandref), trackCollection, usePhotonVertex);
    } else {
      isol = test_->photonPtSum(&(*recoecalcandref), trackCollection, usePhotonVertex);
    }
    
    isoMap.insert(recoecalcandref, isol);

  }

  std::auto_ptr<reco::RecoEcalCandidateIsolationMap> isolMap(new reco::RecoEcalCandidateIsolationMap(isoMap));
  iEvent.put(isolMap);

}
