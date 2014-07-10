#include "L1Trigger/CSCTriggerPrimitives/plugins/RPCTriggerDigisSelector.h"

#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"

RPCTriggerDigisSelector::RPCTriggerDigisSelector(const edm::ParameterSet& conf) 
{
  rpcDigiProducer_ = conf.getParameter<edm::InputTag>("RPCDigiProducer");

//   edm::Service<edm::RandomNumberGenerator> rng;
//   if (!rng.isAvailable()){
//     throw cms::Exception("Configuration")
//       << "GEMDigiProducer::GEMDigiProducer() - RandomNumberGeneratorService is not present in configuration file.\n"
//       << "Add the service in the configuration file or remove the modules that require it.";
//   }
//   CLHEP::HepRandomEngine& engine = rng->getEngine(); 

  // register what this produces
  produces<RPCDigiCollection>();
}

RPCTriggerDigisSelector::~RPCTriggerDigisSelector() 
{
}

void RPCTriggerDigisSelector::produce(edm::Event& ev, const edm::EventSetup& setup) 
{
  LogDebug("RPCTriggerDigisSelector") << "start filtering the bad RPC strip digis";

  edm::ESHandle<RPCGeometry> h;
  setup.get<MuonGeometryRecord>().get(h);
  geometry_ = &*h;
 
  edm::Handle<RPCDigiCollection> stripDigis;
  ev.getByLabel(rpcDigiProducer_.label(), rpcDigiProducer_.instance(), stripDigis);

  std::auto_ptr<RPCDigiCollection> oc_strip(new RPCDigiCollection);

  std::cout << "Studying the Strip failures" << std::endl;
  for (RPCDigiCollection::DigiRangeIterator adetUnitIt = stripDigis->begin(); adetUnitIt != stripDigis->end(); adetUnitIt++){
    const RPCDetId& id = (*adetUnitIt).first;
    auto range((*adetUnitIt).second);
    for (RPCDigiCollection::const_iterator digiIt = range.first; digiIt != range.second; digiIt++) {
      auto digi(*digiIt);
      std::cout << id << " " << digi << std::endl;
      (*oc_strip).insertDigi(id,digi);
    }
  }
  
  // Put collections in event.
  ev.put(oc_strip);
}
