// -*- C++ -*-
//
// Package:    GEMCosmicStandUnpacker
// Class:      GEMCosmicStandUnpacker
// 
/**\class GEMCosmicStandUnpacker GEMCosmicStandUnpacker.cc work/GEMCosmicStandUnpacker/plugins/GEMCosmicStandUnpacker.cc

Description: [one line class summary]

Implementation:
[Notes on implementation]
*/


// system include files
#include <memory>
#include <iomanip> 
#include <iostream>
#include <fstream>
#include <vector>
#include <inttypes.h>

#include "EventFilter/GEMRawToDigi/interface/GEMAMC13EventFormat.h"
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

#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"

#include "CondFormats/GEMObjects/interface/GEMROmap.h"
#include "CondFormats/GEMObjects/interface/GEMEMap.h"
#include "CondFormats/DataRecord/interface/GEMEMapRcd.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESTransientHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESWatcher.h"

//
// class declaration
//

class GEMCosmicStandUnpacker : public edm::EDProducer {
        public:
                explicit GEMCosmicStandUnpacker(const edm::ParameterSet&);
                ~GEMCosmicStandUnpacker();

                static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

        private:
                void beginRun(const edm::Run &run, const edm::EventSetup& es) override;
                virtual void produce(edm::Event&, const edm::EventSetup&) override;
                virtual void endJob() override;

                virtual void ByteVector(std::vector<unsigned char>&, uint64_t&);

                int GetStripFromChannel(uint16_t ChipID, int chan);  
                int GetIEtaFromChipID(uint16_t ChipID);

                virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
                //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
                //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

                // ----------member data ---------------------------

                edm::ESWatcher<GEMEMapRcd> gemMapWatcher;
                GEMROmap* romap;
                GEMROmap* romapV2;


                std::string inputFileName_;// = "GEMDQMRawData.dat";
                std::ifstream inpf_;

                std::vector<int> slotVector_;
                std::vector<unsigned long long> vfatVector_;
                std::vector<int> rowVector_;
                std::vector<int> columnVector_;
                std::vector<int> layerVector_;
                std::vector<int> hitsVector_;
                std::vector<unsigned long long> missingVector_; 
                //std::vector<unsigned long long> missingPositionInVector_; // not useful


                bool checkQualityEvent_;
                bool verbose_;
                std::string FedKit_;

                uint64_t m_word;
                uint32_t m_word32;
                bool type;
                AMC13Event * m_AMC13Event;
                std::FILE *m_file;

                uint16_t nBC;

                bool filledDB;

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
GEMCosmicStandUnpacker::GEMCosmicStandUnpacker(const edm::ParameterSet& iConfig)
{
        //register your products if do put with a label
        produces<FEDRawDataCollection>("GEMTBData");
        produces<GEMDigiCollection>();
        produces<GEMDigiCollection>("WrongGEMDigis");

        inputFileName_ = iConfig.getParameter<std::string>("inputFileName");

        // GEM Raw Data file
        std::ifstream inpf_(inputFileName_.c_str(), std::ios::in|std::ios::binary);
        m_file = std::fopen(inputFileName_.c_str(), "rb");

        FedKit_ = iConfig.getUntrackedParameter<std::string>("FedKit","sdram");

        slotVector_ = iConfig.getParameter<std::vector<int> >("slotVector");  
        vfatVector_ = iConfig.getParameter<std::vector<unsigned long long> >("vfatVector");
        rowVector_ = iConfig.getParameter<std::vector<int> >("rowVector");     
        columnVector_ = iConfig.getParameter<std::vector<int> >("columnVector");     
        layerVector_ = iConfig.getParameter<std::vector<int> >("layerVector");

        verbose_ = iConfig.getUntrackedParameter<bool>("verbose",false);
        checkQualityEvent_ = iConfig.getUntrackedParameter<bool>("checkQualityEvent",true);

}


GEMCosmicStandUnpacker::~GEMCosmicStandUnpacker()
{
        // do anything here that needs to be done at desctruction time
        // (e.g. close files, deallocate resources etc.)
}


        void 
GEMCosmicStandUnpacker::beginRun(const edm::Run &run, const edm::EventSetup& iSetup)
{

        // Checking of Hex/binary type
        //char c = inpf_.get();
        inpf_.close();
        inpf_.open(inputFileName_.c_str(), std::ios::in|std::ios::binary);
        if(!inpf_.is_open()) {
                edm::LogError("") << "\nThe GEM file: " << inputFileName_.c_str() << " is missing.\n";
        };

        for (unsigned int i=0; i<vfatVector_.size(); i++){
                hitsVector_.push_back(0);  
        }


        if(slotVector_.size()!=vfatVector_.size() || slotVector_.size()!=layerVector_.size() || slotVector_.size()!=rowVector_.size() || slotVector_.size()!=columnVector_.size() ){
                edm::LogError("")<<"WRONG CONFIGURATION - arrays should be the same length. Double check the VFAT listing per chamber";
        }

        filledDB=false;

        if (!filledDB&&gemMapWatcher.check(iSetup)) {
                std::cout << "record has CHANGED!!, (re)initialise readout map!"<<std::endl;
                edm::ESTransientHandle<GEMEMap> eMap;
                iSetup.get<GEMEMapRcd>().get(eMap);
                romap = eMap->convertCS();
                std::cout <<" GEM READOUT MAP VERSION: " << eMap->version() << std::endl;
                filledDB=true;
                romapV2 = eMap->convertCSConfigurable(&vfatVector_,&slotVector_);

        }



}


// ------------ method called to produce the data  ------------
        void
GEMCosmicStandUnpacker::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{

        using namespace edm;

        AMCGEMData  gem;
        AMCGEBData  geb;
        //AMCVFATData vfat;

        //uint64_t ui64bits = 0;

        std::auto_ptr<FEDRawDataCollection> pOut(new FEDRawDataCollection());
        std::auto_ptr<GEMDigiCollection> producedGEMDigis(new GEMDigiCollection);
        std::auto_ptr<GEMDigiCollection> producedGEMDigisMissing(new GEMDigiCollection);

        if(inpf_.eof()) { inpf_.close(); iEvent.put(pOut,"GEMTBData");  std::cout<<"END OF FILE"<<std::endl;  return; } // We should put out a collection even if it is empty
        if(!inpf_.good()) { iEvent.put(pOut,"GEMTBData");  std::cout<<"EMPTY"<<std::endl;  return; }

        std::vector<unsigned char> byteVec;  

        // read and print FEROL headers
        if (FedKit_ == "ferol") {
                std::size_t sz = std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                if (sz == 0 ) return;
                if(verbose_) printf("%016lX\n", m_word);
                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                if(verbose_)  printf("%016lX\n", m_word);
                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                if(verbose_)  printf("%016lX\n", m_word);
                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                // ferol headers read and printed, now read CDF header
                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                if(verbose_)  printf("%016lX\n", m_word);
                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);

        } else {
                std::size_t sz = std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                if (sz == 0 ) return;
        }

        // read and print "BADC0FFEEBADCAFE" and another artificial header
        if(verbose_)  printf("%016lX\n", m_word);
        m_AMC13Event = new AMC13Event();
        m_AMC13Event->setCDFHeader(m_word);
        std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        if(verbose_)  printf("%016lX\n", m_word);
        GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
        m_AMC13Event->setAMC13header(m_word);
        if(verbose_)  std::cout << "n_AMC = " << m_AMC13Event->nAMC() << std::endl;

        // Readout out AMC headers
        for (unsigned short i = 0; i < m_AMC13Event->nAMC(); i++){
                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                if(verbose_)  printf("%016lX\n", m_word);
                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                m_AMC13Event->addAMCheader(m_word);
        }


        // Readout out AMC payloads
        for (unsigned short i = 0; i < m_AMC13Event->nAMC(); i++){
                AMCdata * m_amcdata = new AMCdata();
                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                if(verbose_)  {
                        printf("AMC HEADER1\n");
                        printf("%016lX\n", m_word);}
                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                m_amcdata->setAMCheader1(m_word);
                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                if(verbose_)  {printf("AMC HEADER2\n");
                        printf("%016lX\n", m_word);}
                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                m_amcdata->setAMCheader2(m_word);
                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                m_amcdata->setGEMeventHeader(m_word);
                if(verbose_)  {printf("GEM EVENT HEADER\n");
                        printf("%016lX\n", m_word);}
                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                if(verbose_)  std::cout<<"  --->"<<m_amcdata->BX()<<"   "<<m_amcdata->BID()<<std::endl;

                //std::cout<<std::dec<<m_amcdata->BX()<<"    "<<std::dec<<m_amcdata->L1A()<<std::endl;
                //L1A could be used for the event number. BX is dummy. 

                // fill the geb data here
                for (unsigned short j = 0; j < m_amcdata->GDcount(); j++){
                        GEBdata * m_gebdata = new GEBdata();
                        std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                        m_gebdata->setChamberHeader(m_word);
                        if(verbose_)   {printf("GEM CHAMBER HEADER\n");
                                printf("%016lX\n", m_word);} 
                        GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);

                        // fill the vfat data here
                        if(verbose_)  std::cout << "Number of VFAT words " << m_gebdata->Vwh() << std::endl;
                        int m_nvb = m_gebdata->Vwh() / 3; // number of VFAT2 blocks. Eventually add here sanity check
                        if(verbose_)  std::cout << "Number of VFAT blocks " << m_nvb << std::endl;



                        for (unsigned short k = 0; k < m_nvb; k++){
                                    
                                VFATdata * m_vfatdata = new VFATdata();
                                // read 3 vfat block words, totaly 192 bits
                                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                                if(verbose_){
                                        printf("VFAT WORD 1\n");
                                        printf("%016lX\n", m_word);
                                }
                                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                                m_vfatdata->read_fw(m_word);
                                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                                if(verbose_){
                                        printf("VFAT WORD 2\n");
                                        printf("%016lX\n", m_word);
                                }
                                m_vfatdata->read_sw(m_word);
                                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                                if(verbose_){
                                        printf("VFAT WORD 3\n");
                                        printf("%016lX\n", m_word);
                                }
                                m_vfatdata->read_tw(m_word);
                                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                                //
                                if(verbose_){
                                        printf("VFAT MS Data 3\n");
                                        printf("%016lX\n", m_vfatdata->msData());
                                        printf("VFAT LS Data 3\n");
                                        printf("%016lX\n", m_vfatdata->lsData());
                                }
                                m_gebdata->v_add(*m_vfatdata);

                                uint16_t bc=m_vfatdata->BC();
                                uint8_t ec=m_vfatdata->EC();
                                uint8_t b1010=m_vfatdata->b1010();
                                uint8_t b1100=m_vfatdata->b1100();
                                uint8_t b1110=m_vfatdata->b1110();
                                uint16_t  ChipID=m_vfatdata->ChipID();
                                //int slot=m_vfatdata->SlotNumber(); 

                                bool Quality = (b1010==10) && (b1100==12) && (b1110==14) ;      // add CRC? 

                                uint64_t converted=ChipID+0xf000;    
                                bool foundChip=false;
                                int column=1;
                                int row=1;
                                int chamberPosition=1;
                                int slot=0;
                                for(unsigned int i=0; i<vfatVector_.size(); i++) { 
                                        if( converted == vfatVector_[i]) {
                                                foundChip=true; 
                                                column=columnVector_[i]; 
                                                slot=slotVector_[i];
                                                row=rowVector_[i]; 
                                                chamberPosition=layerVector_[i]; 
                                                hitsVector_[i]++;
                                        }}
                                int schamberPosition=1+2*(row-1)+10*(column-1);
                                if (!foundChip) {
                                        bool alreadyin=false;
                                        for (unsigned int i=0; i<missingVector_.size(); i++) {
                                                if(converted==missingVector_[i]) alreadyin=true;   
                                        }
                                        if(!alreadyin) {
                                                missingVector_.push_back(converted);
                                                //missingPositionInVector_.push_back(k);
                                                std::cout<<"Unpacked VFAT not in the configuration - double check the settings"<<std::endl;
                                                std::cout<<" ---> VFAT--->"<<converted<<" (will only give this warning once)"<<std::endl;
                                        }
                                }
                  
                                if(verbose_){
                                        std::cout<<std::dec<<"  --->"<<m_amcdata->BX()<<"   "<<m_amcdata->BID()<<std::endl;  
                                        std::cout<<std::dec<<" SLOT? "<<slot<<std::endl;
                                        std::cout<<std::dec<<" ---> VFAT--->"<<(unsigned)bc<<" -   "<<(unsigned)ec<<" -   "<<std::hex<<(unsigned)ChipID<<std::endl;
                                        std::cout<<std::dec<<" --> COLUMN = "<<column<<"    ROW  = "<<row<<"     Layer:"<<chamberPosition<<" -->   SC:"<<schamberPosition<<std::endl;
                                }

                                if(!Quality && checkQualityEvent_) continue;

                                int bx=0;  
                                uint8_t chan0xf = 0;

                                for(int chan = 0; chan < 128; ++chan) {

                                        if(chan < 64){
                                                chan0xf = ((m_vfatdata->lsData() >> chan) & 0x1);
                                        } else {
                                                chan0xf = ((m_vfatdata->msData() >> (chan-64)) & 0x1);
                                        }

                                        if(chan0xf==0) continue;  

                                        GEMROmap::eCoord ec;
                                        ec.chamberId=31;
                                        ec.vfatId = ChipID+0xf000;
                                        ec.channelId = chan+1;
                                        GEMROmap::dCoord dc = romapV2->hitPosition(ec);
                                        if(verbose_){ std::cout <<"Full --> Chamber "<<ec.chamberId<<" vfat 0x"<<std::hex<<ec.vfatId<<std::dec<<" chan="<<ec.channelId
                                                <<" correspond to eta="<<dc.etaId<<" strip="<<dc.stripId<<std::endl;}

                                        if(verbose_) std::cout<<"----------->"<<chan<<"   "<<(unsigned)chan0xf<<std::endl;

                                        int strip=dc.stripId;//
                                        int etaP=dc.etaId;

                                        if(etaP == 0) {
                                                if(foundChip){
                                                std::cout<<"WARNING: wrong digi! DoubleCheck the configuration"<<std::endl;
                                                std::cout<<std::dec<<" --> COLUMN = "<<column<<"    ROW  = "<<row<<"     Layer:"<<chamberPosition<<" -->   SC:"<<schamberPosition<<std::endl;
                                                std::cout<<std::hex<<" ---> VFAT--->"<<ec.vfatId<<"   slot="<<slot<<std::endl;
                                                std::cout<<std::dec<<" chan="<<ec.channelId<<" correspond to eta="<<dc.etaId<<" strip="<<dc.stripId<<std::endl;
                                                }
                                                else{
                                                GEMDigi digi(chan,bx);
                                                producedGEMDigisMissing.get()->insertDigi(GEMDetId(1,1,1,1,1,0),digi);
                                                }      
                                        }
                                        else{  
                                                GEMDigi digi(strip,bx); 
                                                // bx is a single digi, where we should give 
                                                // in input the strip and bx relative to trigger
                                                producedGEMDigis.get()->insertDigi(GEMDetId(1,1,1,chamberPosition,schamberPosition,etaP),digi); 
                                        }
                                }
                                delete m_vfatdata;

                        }
                        std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                        GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                        m_gebdata->setChamberTrailer(m_word);

                        m_amcdata->g_add(*m_gebdata);
                        delete m_gebdata;
                }
                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                m_amcdata->setGEMeventTrailer(m_word);
                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                std::fread(&m_word, sizeof(uint64_t), 1, m_file);
                GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
                if(verbose_){
                        printf("AMC TRAILER\n");
                        printf("%016lX\n", m_word);
                }
                m_amcdata->setAMCTrailer(m_word);
                m_AMC13Event->addAMCpayload(*m_amcdata);
                delete m_amcdata;
        }
        std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        m_AMC13Event->setAMC13trailer(m_word);
        GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);
        std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        m_AMC13Event->setCDFTrailer(m_word);
        GEMCosmicStandUnpacker::ByteVector(byteVec, m_word);


        uint64_t OneEventBytes = byteVec.size();

        if (OneEventBytes> 0 ) 
        {
                FEDRawData f1(OneEventBytes); // One Event has been allocated

                for (uint64_t i=0; i<OneEventBytes; i++){
                        f1.data()[i] = byteVec[i];
                }

                //     std::auto_ptr<FEDRawDataCollection> pOut(new FEDRawDataCollection());
                pOut->FEDData(999) = f1;
                //     iEvent.put(pOut,"GEMTBData");

        }

        if(verbose_) std::cout <<" Run "<< iEvent.eventAuxiliary().run()<<"     "<<iEvent.eventAuxiliary().event()<<std::endl;
        iEvent.put(pOut,"GEMTBData"); iEvent.put(producedGEMDigis); iEvent.put(producedGEMDigisMissing,"WrongGEMDigis");
}

// ------------ method called once each 64 Bits data word and keep in vector ------------
void 
GEMCosmicStandUnpacker::ByteVector(std::vector<unsigned char>& byteVec, uint64_t& word64ui) {

        union{uint64_t ui64; unsigned char byte[8];} U;

        U.ui64 = word64ui;
        for (int iChar=0; iChar<8; ++iChar){
                byteVec.push_back(U.byte[iChar]);  
        }
        //std::cout << " word64ui 0x" << std::hex << U.ui64 << std::dec << " byteVec.size() " << byteVec.size() << std::endl; 

}


// ------------ method called once each job just after ending the event loop  ------------
void 
GEMCosmicStandUnpacker::endJob() {
        // Data GEM Stream close
        inpf_.close();
}

// ------------ method called when ending the processing of a run  ------------
        void
GEMCosmicStandUnpacker::endRun(edm::Run const&, edm::EventSetup const&)
{

        std::cout<<"FILLED VFATS"<<std::endl;
        std::cout<<"SLOT - VFAT - LAYER - COLUMN - ROW ---> Number of events with hits"<<std::endl;
        for (unsigned int i=0; i<vfatVector_.size(); i++){
                std::cout<<std::dec<<"\t"<<slotVector_.at(i)<<"\t"<<std::hex<<vfatVector_.at(i)<<"\t"<<layerVector_.at(i)<<"\t"<<columnVector_.at(i)<<"\t"<<rowVector_.at(i)<<"  -----> "<<std::dec<<hitsVector_.at(i)<<std::endl;
        }
        std::cout<<"MISSING VFATS"<<std::endl;
        for (unsigned int i=0; i<missingVector_.size(); i++){
                std::cout<<std::hex<<missingVector_[i]<<std::dec<<std::endl;
        }


}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
GEMCosmicStandUnpacker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
        //The following says we do not know what parameters are allowed so do no validation
        // Please change this to state exactly what you do use, even if it is no parameters
        edm::ParameterSetDescription desc;
        desc.setUnknown();
        descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(GEMCosmicStandUnpacker);
