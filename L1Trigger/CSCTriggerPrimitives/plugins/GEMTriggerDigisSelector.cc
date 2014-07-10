#include "L1Trigger/CSCTriggerPrimitives/plugins/GEMTriggerDigisSelector.h"

#include "DataFormats/GEMDigi/interface/GEMCSCPadDigiCollection.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"

GEMTriggerDigisSelector::GEMTriggerDigisSelector(const edm::ParameterSet& conf) 
{
  gemDigiProducer_ = conf.getParameter<edm::InputTag>("GEMPadDigiProducer");

//   edm::Service<edm::RandomNumberGenerator> rng;
//   if (!rng.isAvailable()){
//     throw cms::Exception("Configuration")
//       << "GEMDigiProducer::GEMDigiProducer() - RandomNumberGeneratorService is not present in configuration file.\n"
//       << "Add the service in the configuration file or remove the modules that require it.";
//   }
//   CLHEP::HepRandomEngine& engine = rng->getEngine(); 

  // register what this produces
  produces<GEMCSCPadDigiCollection>();
}

GEMTriggerDigisSelector::~GEMTriggerDigisSelector() 
{
}

void GEMTriggerDigisSelector::produce(edm::Event& ev, const edm::EventSetup& setup) 
{
  LogDebug("GEMTriggerDigisSelector") << "start filtering the bad GEM pad digis";

  edm::ESHandle<GEMGeometry> h;
  setup.get<MuonGeometryRecord>().get(h);
  geometry_ = &*h;

  edm::Handle<GEMCSCPadDigiCollection> padDigis;
  ev.getByLabel(gemDigiProducer_.label(), gemDigiProducer_.instance(), padDigis);

  std::auto_ptr<GEMCSCPadDigiCollection> oc_pad(new GEMCSCPadDigiCollection);

  if (!padDigis.isValid()) {
    edm::LogWarning("GEMTriggerDigisSelector")
      << "+++ Warning: Collection of pad digis with label "
      << gemDigiProducer_.label()
      << " requested in configuration, but not found in the event..."
      << " Skipping filtering of GEM pad digis +++\n";
  }

  std::cout << "Studying the Strip failures" << std::endl;
  for (GEMCSCPadDigiCollection::DigiRangeIterator adetUnitIt = padDigis->begin(); adetUnitIt != padDigis->end(); adetUnitIt++){
    const GEMDetId& id = (*adetUnitIt).first;
    auto range((*adetUnitIt).second);
    for (GEMCSCPadDigiCollection::const_iterator digiIt = range.first; digiIt != range.second; digiIt++) {
      auto digi(*digiIt);
      std::cout << id << " " << digi << std::endl;
      (*oc_pad).insertDigi(id,digi);
    }
  }
  
  // Put collections in event.
  ev.put(oc_pad);
}
