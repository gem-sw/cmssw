// -*- C++ -*-
//
// Package:    GEMDigiReader
// Class:      GEMDigiReader
// 
/**\class GEMDigiReader GEMDigiReader.cc EventFilter/GEMDigiReader/plugins/GEMDigiReader.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Marcello Maggi
//         Created:  Thu, 03 Dec 2015 15:49:32 GMT
// $Id$
//
//


// system include files
#include <memory>
#include <string>
// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
//
// class declaration
//

class GEMDigiReader : public edm::EDAnalyzer {
   public:
      explicit GEMDigiReader(const edm::ParameterSet&);
      ~GEMDigiReader();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
      //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

      // ----------member data ---------------------------
  private:
  edm::EDGetTokenT<GEMDigiCollection> digiLabel_;

};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
GEMDigiReader::GEMDigiReader(const edm::ParameterSet& pset)

{
   //now do what ever initialization is needed  
//  digiLabel = iConfig.getUntrackedParameter<std::string> ("digiLabel");
    digiLabel_ = consumes<GEMDigiCollection>(pset.getParameter<edm::InputTag>("InputLabel"));
}


GEMDigiReader::~GEMDigiReader()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
GEMDigiReader::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{

   edm::Handle<GEMDigiCollection> gemDigis;
   if(iEvent.getByToken(digiLabel_,gemDigis)){

   //loop over Digis
   GEMDigiCollection::DigiRangeIterator detUnitIt;
   for (detUnitIt = gemDigis->begin(); detUnitIt != gemDigis->end(); ++detUnitIt)  {
     const GEMDetId gemid = (*detUnitIt).first;
     std::cout <<" Det ID "<<gemid<<std::endl;
     const GEMDigiCollection::Range& range = (*detUnitIt).second;
     double ndigi = 0;
     for (GEMDigiCollection::const_iterator digiIt = range.first; digiIt != range.second; ++digiIt) {
//       std::cout <<"  strip "<<digiIt->strip()<<"   BX relative to :LV1A 0x"<< std::hex << digiIt->bx() << std::dec << std::endl;
       ndigi++;
     }
     std::cout <<"  Number of DIGI "<<ndigi<<std::endl;
   }

   }

   /*   
   76   os <<  " Re "<<id.region()
   77      << " Ri "<<id.ring()
   78      << " St "<<id.station()
   79      << " La "<<id.layer()
   80      << " Ch "<<id.chamber()
   81      << " Ro "<<id.roll()
   82      <<" ";*/

}


// ------------ method called once each job just before starting event loop  ------------
void 
GEMDigiReader::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
GEMDigiReader::endJob() 
{
}

// ------------ method called when starting to processes a run  ------------
/*
void 
GEMDigiReader::beginRun(edm::Run const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when ending the processing of a run  ------------
/*
void 
GEMDigiReader::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when starting to processes a luminosity block  ------------
/*
void 
GEMDigiReader::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when ending the processing of a luminosity block  ------------
/*
void 
GEMDigiReader::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
GEMDigiReader::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(GEMDigiReader);
