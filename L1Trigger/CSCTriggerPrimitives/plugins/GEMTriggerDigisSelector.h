#ifndef CSCTriggerPrimitives_GEMTriggerDigisSelector_h
#define CSCTriggerPrimitives_GEMTriggerDigisSelector_h

/** \class GEMTriggerDigisSelector
 *
 * Class to simulate failures of CFEBs, AFEBs, a single layer 
 * or an entire TMB 
 *
 * Produces collections of "Good" (i.e. not broken) wire digis 
 * and comparator digis useful for further processing.
 * 
 * \author Sven Dildick, TAMU-UGent
 *
 */

#include <FWCore/Framework/interface/Frameworkfwd.h>
#include <FWCore/Framework/interface/EDProducer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include "CLHEP/Random/RandomEngine.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"

class GEMGeometry;

class GEMTriggerDigisSelector : public edm::EDProducer
{
 public:
  explicit GEMTriggerDigisSelector(const edm::ParameterSet&);
  ~GEMTriggerDigisSelector();

  //virtual void beginRun(const edm::EventSetup& setup);
  virtual void produce(edm::Event&, const edm::EventSetup&);

 private:
  const GEMGeometry* geometry_;
  edm::InputTag gemDigiProducer_;
};

#endif
