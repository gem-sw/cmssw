#ifndef CSCTriggerPrimitives_RPCTriggerDigisSelector_h
#define CSCTriggerPrimitives_RPCTriggerDigisSelector_h

/** \class RPCTriggerDigisSelector
 *
 * Class to simulate failures of RPC hardware failures 
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

class RPCGeometry;

class RPCTriggerDigisSelector : public edm::EDProducer
{
 public:
  explicit RPCTriggerDigisSelector(const edm::ParameterSet&);
  ~RPCTriggerDigisSelector();

  //virtual void beginRun(const edm::EventSetup& setup);
  virtual void produce(edm::Event&, const edm::EventSetup&);

 private:
  const RPCGeometry* geometry_;
  edm::InputTag rpcDigiProducer_;
};

#endif
