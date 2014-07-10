#include "L1Trigger/CSCTriggerPrimitives/plugins/CSCTriggerDigisSelector.h"

#include "DataFormats/CSCDigi/interface/CSCComparatorDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCWireDigiCollection.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "L1Trigger/CSCCommonTrigger/interface/CSCTriggerGeometry.h"

CSCTriggerDigisSelector::CSCTriggerDigisSelector(const edm::ParameterSet& conf) 
{
  wireDigiProducer_ = conf.getParameter<edm::InputTag>("CSCWireDigiProducer");
  compDigiProducer_ = conf.getParameter<edm::InputTag>("CSCComparatorDigiProducer");

//   edm::Service<edm::RandomNumberGenerator> rng;
//   if (!rng.isAvailable()){
//     throw cms::Exception("Configuration")
//       << "GEMDigiProducer::GEMDigiProducer() - RandomNumberGeneratorService is not present in configuration file.\n"
//       << "Add the service in the configuration file or remove the modules that require it.";
//   }
//   CLHEP::HepRandomEngine& engine = rng->getEngine(); 

  // register what this produces
  produces<CSCComparatorDigiCollection>();
  produces<CSCWireDigiCollection>();
}

CSCTriggerDigisSelector::~CSCTriggerDigisSelector() 
{
}

void CSCTriggerDigisSelector::produce(edm::Event& ev, const edm::EventSetup& setup) 
{
  LogDebug("CSCTriggerDigisSelector") << "start filtering the bad CSC Wire and Strip digis event ";
  // Find the geometry (& conditions?) for this event & cache it in 
  // CSCTriggerGeometry.
  {
    edm::ESHandle<CSCGeometry> h;
    setup.get<MuonGeometryRecord>().get(h);
    CSCTriggerGeometry::setGeometry(h);
  }

  // Get the collections of comparator & wire digis from event.
  edm::Handle<CSCComparatorDigiCollection> compDigis;
  edm::Handle<CSCWireDigiCollection>       wireDigis;
  ev.getByLabel(compDigiProducer_.label(), compDigiProducer_.instance(), compDigis);
  ev.getByLabel(wireDigiProducer_.label(), wireDigiProducer_.instance(), wireDigis);

  std::auto_ptr<CSCComparatorDigiCollection> oc_strip(new CSCComparatorDigiCollection);
  std::auto_ptr<CSCWireDigiCollection> oc_wire(new CSCWireDigiCollection);

  if (!wireDigis.isValid()) {
    edm::LogWarning("L1CSCTPEmulatorNoInputCollection")
      << "+++ Warning: Collection of wire digis with label "
      << wireDigiProducer_.label()
      << " requested in configuration, but not found in the event..."
      << " Skipping production of CSC TP digis +++\n";
  }
  if (!compDigis.isValid()) {
    edm::LogWarning("L1CSCTPEmulatorNoInputCollection")
      << "+++ Warning: Collection of comparator digis with label "
      << compDigiProducer_.label()
      << " requested in configuration, but not found in the event..."
      << " Skipping production of CSC TP digis +++\n";
  }
  std::cout << "Studying the Strip failures" << std::endl;
  // Filter the CSC Comparator digis 
  for (CSCComparatorDigiCollection::DigiRangeIterator adetUnitIt = compDigis->begin(); adetUnitIt != compDigis->end(); adetUnitIt++){
    const CSCDetId& id = (*adetUnitIt).first;
    //if (id.endcap() != 1) continue;
    auto range((*adetUnitIt).second);
    for (CSCComparatorDigiCollection::const_iterator digiIt = range.first; digiIt != range.second; digiIt++) {
      auto digi(*digiIt);
      std::cout << id << " " << digi << std::endl;
      (*oc_strip).insertDigi(id,digi);
    }
  }
  
  std::cout << "Studying the Wire failures" << std::endl;
  // Filter the CSC Wire digis 
  for (CSCWireDigiCollection::DigiRangeIterator adetUnitIt = wireDigis->begin(); adetUnitIt != wireDigis->end(); adetUnitIt++){
    const CSCDetId& id = (*adetUnitIt).first;
    //if (id.endcap() != 1) continue;
    auto range((*adetUnitIt).second);
    for (CSCWireDigiCollection::const_iterator digiIt = range.first; digiIt != range.second; digiIt++) {
      auto digi(*digiIt);
      std::cout << id << " " << digi << std::endl;
      (*oc_wire).insertDigi(id,digi);
    }
  }
  
  // Put collections in event.
  ev.put(oc_strip);
  ev.put(oc_wire);
}
