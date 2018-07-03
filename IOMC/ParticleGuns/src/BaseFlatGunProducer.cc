/*
 *  \author Julia Yarba
 */

#include <ostream>
#include <memory>

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"

#include "SimGeneral/HepPDTRecord/interface/ParticleDataTable.h"

#include "IOMC/ParticleGuns/interface/BaseFlatGunProducer.h"


#include <iostream>

using namespace edm;
using namespace std;
using namespace CLHEP;

BaseFlatGunProducer::BaseFlatGunProducer( const ParameterSet& pset ) :
   fEvt(nullptr)
{
   Service<RandomNumberGenerator> rng;
   if(!rng.isAvailable()) {
    throw cms::Exception("Configuration")
       << "The RandomNumberProducer module requires the RandomNumberGeneratorService\n"
          "which appears to be absent.  Please add that service to your configuration\n"
          "or remove the modules that require it.";
   }

   ParameterSet pgun_params = pset.getParameter<ParameterSet>("PGunParameters") ;
  
   fPartIDs  = pgun_params.getParameter<std::vector<int> >("PartID");
   fMinPhi      = pgun_params.getParameter<double>("MinPhi");
   fMaxPhi      = pgun_params.getParameter<double>("MaxPhi");
   //fMinEta      = pgun_params.getParameter<double>("MinEta");
   //fMaxEta      = pgun_params.getParameter<double>("MaxEta");  
   fMinTheta    = pgun_params.getParameter<double>("MinTheta");
   fMaxTheta    = pgun_params.getParameter<double>("MaxTheta");
   fIsThetaFlat = pgun_params.getParameter<bool>("IsThetaFlat"); // If 'True': theta distribution is flat. If 'False': theta distribution is a cos^2
   
   fVerbosity = pset.getUntrackedParameter<int>( "Verbosity",0 ) ;
   fAddAntiParticle = pset.getParameter<bool>("AddAntiParticle") ;

   produces<GenRunInfoProduct, Transition::EndRun>();
}

BaseFlatGunProducer::~BaseFlatGunProducer()
{  
}


void BaseFlatGunProducer::beginRun(const edm::Run & r, const EventSetup& es )
{
   es.getData( fPDGTable ) ;
   return ;

}
void BaseFlatGunProducer::endRun(const Run &run, const EventSetup& es ) {
}


void BaseFlatGunProducer::endRunProduce(Run &run, const EventSetup& es )
{
   unique_ptr<GenRunInfoProduct> genRunInfo( new GenRunInfoProduct() );
   run.put(std::move(genRunInfo));
}

