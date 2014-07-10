#ifndef CSCTriggerPrimitives_CSCTriggerDigisSelector_h
#define CSCTriggerPrimitives_CSCTriggerDigisSelector_h

/** \class CSCTriggerDigisSelector
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

class CSCTriggerDigisSelector : public edm::EDProducer
{
 public:
  explicit CSCTriggerDigisSelector(const edm::ParameterSet&);
  ~CSCTriggerDigisSelector();

  //virtual void beginRun(const edm::EventSetup& setup);
  virtual void produce(edm::Event&, const edm::EventSetup&);

 private:
  edm::InputTag compDigiProducer_;
  edm::InputTag wireDigiProducer_;
};

#endif
