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
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
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
  edm::EDGetTokenT<GEMDigiCollection> digiLabel;
  TFile* hfile;
  TH1F* hpa5;
  TH1F* hpa6;
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
GEMDigiReader::GEMDigiReader(const edm::ParameterSet& iConfig)

{
   //now do what ever initialization is needed  
  digiLabel = consumes<GEMDigiCollection>(iConfig.getParameter<edm::InputTag>("DigiLabel"));
  hfile=new TFile("Ciccio.root","RECREATE");
  hpa5=new TH1F("Partition 5","Partition 5",384,0.5,384.5);
  hpa6=new TH1F("Partition 5","Partition 5",384,0.5,384.5);
}


GEMDigiReader::~GEMDigiReader()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
  hfile->Write();
  hfile->Close();
}


//
// member functions
//

// ------------ method called for each event  ------------
void
GEMDigiReader::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{


   edm::Handle<GEMDigiCollection> gemDigis;
   iEvent.getByToken(digiLabel, gemDigis);

   //loop over Digis
   GEMDigiCollection::DigiRangeIterator detUnitIt;
   for (detUnitIt = gemDigis->begin(); detUnitIt != gemDigis->end(); ++detUnitIt)  {
     const GEMDetId gemid = (*detUnitIt).first;
     std::cout <<" Det ID "<<gemid<<"  --> "<<gemid.roll()<<std::endl;
     const GEMDigiCollection::Range& range = (*detUnitIt).second;
     double ndigi = 0;
     for (GEMDigiCollection::const_iterator digiIt = range.first; digiIt != range.second; ++digiIt) {
       std::cout <<"  strip "<<digiIt->strip()<<"   bx relative to lv1A "<<digiIt->bx()<<std::endl;
       ndigi++;
       std::cout <<" ++ "<<digiIt->strip()<<" "<<gemid.roll()<<std::endl;
       if (gemid.roll()==5) {
	 hpa5->Fill(digiIt->strip());
       }
     }
     std::cout <<"  Number of DIGI "<<ndigi<<std::endl;
   }
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
