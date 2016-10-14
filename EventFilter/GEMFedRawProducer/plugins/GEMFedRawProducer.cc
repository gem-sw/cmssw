// -*- C++ -*-
//
// Package:    GEMFedRawProducer
// Class:      GEMFedRawProducer
// 
/**\class GEMFedRawProducer GEMFedRawProducer.cc work/GEMFedRawProducer/plugins/GEMFedRawProducer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Sergey Baranov
//         Created:  Mon, 23 Nov 2015 14:33:52 GMT
// $Id$
//
//


// system include files
#include <memory>
#include <iomanip> 
#include <iostream>
#include <fstream>
#include <vector>

#include "EventFilter/GEMRawToDigi/interface/GEMDataAMCformat.h"
#include "EventFilter/GEMRawToDigi/interface/GEMslotContents.h"
#include "EventFilter/GEMRawToDigi/interface/GEMDataChecker.h"

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
//
// class declaration
//

class GEMFedRawProducer : public edm::EDProducer {
   public:
      explicit GEMFedRawProducer(const edm::ParameterSet&);
      ~GEMFedRawProducer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void beginJob() override;
      virtual void produce(edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;
      
      virtual void ByteVector(std::vector<unsigned char>&, uint64_t&);

      //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
      //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

      // ----------member data ---------------------------

      // This needs to be changed to read from a file 
      std::string inputFileName_;  //= "/afs/cern.ch/work/c/cepeda/performance/GEM/CMSSW_8_1_0_pre12/src/EventFilter/GEMFedRawProducer/data/GEMDQMRawData.dat";
      //std::string slot_file_ = "/afs/cern.ch/work/c/cepeda/performance/GEM/CMSSW_8_1_0_pre12/src/EventFilter/GEMFedRawProducer/data/slot_table.csv";  // this is not really used anymore?
      std::string InpType_ = "Binary";

      std::ifstream inpf_;
      std::unique_ptr<GEMslotContents> slotInfo_;

};

//
// constants, enums and typedefs
//
typedef GEMDataAMCformat::GEMData  AMCGEMData;
typedef GEMDataAMCformat::GEBData  AMCGEBData;
typedef GEMDataAMCformat::VFATData AMCVFATData;

//
// static data member definitions
//

//
// constructors and destructor
//
GEMFedRawProducer::GEMFedRawProducer(const edm::ParameterSet& iConfig)
{
   //register your products if do put with a label
   produces<FEDRawDataCollection>("GEMTBData");
   inputFileName_ = iConfig.getParameter<std::string>("inputFileName");

   // GEM Raw Data file
   std::ifstream inpf_(inputFileName_.c_str(), std::ios::in|std::ios::binary);

}


GEMFedRawProducer::~GEMFedRawProducer()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//

// ------------ method called once each job just before starting event loop  ------------
void 
GEMFedRawProducer::beginJob()
{

  // Checking of Hex/binary type
  char c = inpf_.get();
  inpf_.close();
  if ( c != 1 ) InpType_ = "Hex";
  std::cout << " Input File file " << inputFileName_ << " has type " << c << " " << "  " << InpType_ << std::endl;

  inpf_.open(inputFileName_.c_str(), std::ios::in|std::ios::binary);
  if(!inpf_.is_open()) {
    std::cout << "\nThe GEM file: " << inputFileName_.c_str() << " is missing.\n" << std::endl;
  };

}


// ------------ method called to produce the data  ------------
void
GEMFedRawProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

   AMCGEMData  gem;
   AMCGEBData  geb;
   AMCVFATData vfat;

   std::vector<unsigned char> byteVec; 

   if(inpf_.eof()) return;
   if(!inpf_.good()) return;

  /*
   * GEM Headers Data level
   */
   if (InpType_ == "Hex") {
     if(!GEMDataAMCformat::readGEMhd1(inpf_, gem)) return;
     GEMFedRawProducer::ByteVector(byteVec, gem.header1);
     if(!GEMDataAMCformat::readGEMhd2(inpf_, gem)) return;
     GEMFedRawProducer::ByteVector(byteVec, gem.header2);
     if(!GEMDataAMCformat::readGEMhd3(inpf_, gem)) return;
     GEMFedRawProducer::ByteVector(byteVec, gem.header3);
     if(!GEMDataAMCformat::readGEBheader(inpf_, geb)) return;
     GEMFedRawProducer::ByteVector(byteVec, geb.header);
     if(!GEMDataAMCformat::readGEBrunhed(inpf_, geb)) return;
     GEMFedRawProducer::ByteVector(byteVec, geb.runhed);
   } else {
     if(!GEMDataAMCformat::readGEMhd1Binary(inpf_, gem)) return;
     GEMFedRawProducer::ByteVector(byteVec, gem.header1);
     if(!GEMDataAMCformat::readGEMhd2Binary(inpf_, gem)) return;
     GEMFedRawProducer::ByteVector(byteVec, gem.header2);
     if(!GEMDataAMCformat::readGEMhd3Binary(inpf_, gem)) return;
     GEMFedRawProducer::ByteVector(byteVec, gem.header3);
     if(!GEMDataAMCformat::readGEBheaderBinary(inpf_, geb)) return;
     GEMFedRawProducer::ByteVector(byteVec, geb.header);
     if(!GEMDataAMCformat::readGEBrunhedBinary(inpf_, geb)) return;
     GEMFedRawProducer::ByteVector(byteVec, geb.runhed);
   }

   uint64_t sumVFAT = (0x000000000fffffff & geb.header);
   uint64_t LV1ID   = (0x00ffffff00000000 & gem.header1) >> 32; 

  /*
   * GEM Pay Load Data
   */
   for(uint64_t ivfat=1; ivfat <= sumVFAT; ivfat++){
     if (InpType_ == "Hex") {
       if(!GEMDataAMCformat::readVFATdata(inpf_, ivfat, vfat)) return;
     } else {
       if(!GEMDataAMCformat::readVFATdataBinary(inpf_, ivfat, vfat)) return;
     }
     //GEMDataAMCformat::printVFATdataBits(ivfat, vfat);

     uint64_t w1 = vfat.BC;
     uint64_t w2 = vfat.EC;
     uint64_t w3 = vfat.ChipID;
     uint64_t ui64bits = (w1 << 48)|(w2 << 32)|(w3 << 16)|(vfat.crc);
     GEMFedRawProducer::ByteVector(byteVec, ui64bits);

     uint64_t BX = vfat.BXfrOH;    
     GEMFedRawProducer::ByteVector(byteVec, BX);
     GEMFedRawProducer::ByteVector(byteVec, vfat.lsData);
     GEMFedRawProducer::ByteVector(byteVec, vfat.msData);

   }// end VFATs

  /*
   *  GEB Trailers Data level
   */
   if (InpType_ == "Hex") {
     if(!GEMDataAMCformat::readGEBtrailer(inpf_, geb)) return;
     GEMFedRawProducer::ByteVector(byteVec, geb.trailer);
     if(!GEMDataAMCformat::readGEMtr2(inpf_, gem)) return;
     GEMFedRawProducer::ByteVector(byteVec, gem.trailer2);
     if(!GEMDataAMCformat::readGEMtr1(inpf_, gem)) return;
     GEMFedRawProducer::ByteVector(byteVec, gem.trailer1);
   } else {
     if(!GEMDataAMCformat::readGEBtrailerBinary(inpf_, geb)) return;
     GEMFedRawProducer::ByteVector(byteVec, geb.trailer);
     if(!GEMDataAMCformat::readGEMtr2Binary(inpf_, gem)) return;
     GEMFedRawProducer::ByteVector(byteVec, gem.trailer2);
     if(!GEMDataAMCformat::readGEMtr1Binary(inpf_, gem)) return;
     GEMFedRawProducer::ByteVector(byteVec, gem.trailer1);
   }
   uint64_t OneEventBytes = byteVec.size();
   if (OneEventBytes > 0 ) 
     {
     FEDRawData f1(OneEventBytes); // One Event has been allocated
     for (uint64_t i=0; i<OneEventBytes; i++){
       f1.data()[i] = byteVec[i];
     }
  
     auto pOut = std::make_unique<FEDRawDataCollection>(); 
//     std::auto_ptr<FEDRawDataCollection> pOut(new FEDRawDataCollection());
     pOut->FEDData(999) = f1;
     iEvent.put(std::move(pOut),"GEMTBData"); 
 
     std::cout <<" Run "<< iEvent.eventAuxiliary().run()
               <<" ui64 words "<< f1.size()/8 <<" VFATs "<< (f1.size()/8-7)/3 <<" sumVFAT "<< sumVFAT //<<" f1.size() "<< f1.size() 
               <<" LV1ID "<< LV1ID <<" iEvent "<< iEvent.eventAuxiliary().event() << std::endl;
     }
}

// ------------ method called once each 64 Bits data word and keep in vector ------------
void 
GEMFedRawProducer::ByteVector(std::vector<unsigned char>& byteVec, uint64_t& word64ui) {

  union{uint64_t ui64; unsigned char byte[8];} U;

  U.ui64 = word64ui;
  for (int iChar=0; iChar<8; ++iChar){
    byteVec.push_back(U.byte[iChar]);  
  }
  //std::cout << " word64ui 0x" << std::hex << U.ui64 << std::dec << " byteVec.size() " << byteVec.size() << std::endl; 

}


// ------------ method called once each job just after ending the event loop  ------------
void 
GEMFedRawProducer::endJob() {
  // Data GEM Stream close
  inpf_.close();
}

// ------------ method called when starting to processes a run  ------------
/*
void
GEMFedRawProducer::beginRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a run  ------------
/*
void
GEMFedRawProducer::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when starting to processes a luminosity block  ------------
/*
void
GEMFedRawProducer::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
GEMFedRawProducer::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
GEMFedRawProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(GEMFedRawProducer);
