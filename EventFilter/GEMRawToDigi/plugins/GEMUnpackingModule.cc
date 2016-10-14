#include "EventFilter/GEMRawToDigi/interface/GEMDataAMCformat.h"
#include "EventFilter/GEMRawToDigi/interface/GEMDataChecker.h"
#include "EventFilter/GEMRawToDigi/interface/GEMUnpackingModule.h"
#include "DataFormats/FEDRawData/interface/FEDRawData.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDHeader.h"
#include "DataFormats/FEDRawData/interface/FEDTrailer.h"

#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESTransientHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESWatcher.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "CondFormats/GEMObjects/interface/GEMROmap.h"
#include "CondFormats/GEMObjects/interface/GEMEMap.h"
#include "CondFormats/DataRecord/interface/GEMEMapRcd.h"

#include <sstream>
#include <bitset>
#include <vector>
#include <algorithm> 

//
// constants, enums and typedefs
//
typedef GEMDataAMCformat::GEMData  AMCGEMData;
typedef GEMDataAMCformat::GEBData  AMCGEBData;
typedef GEMDataAMCformat::VFATData AMCVFATData;
typedef uint64_t Word64;

GEMUnpackingModule::GEMUnpackingModule(const edm::ParameterSet& pset) 
{
  dataLabel_ = consumes<FEDRawDataCollection>(pset.getParameter<edm::InputTag>("InputLabel"));
  produces<GEMDigiCollection>();
}

GEMUnpackingModule::~GEMUnpackingModule()
{ 
}

void GEMUnpackingModule::beginRun(const edm::Run &run, const edm::EventSetup& es)
{
  if (gemMapWatcher.check(es)) {  
    LogTrace("") << "record has CHANGED!!, (re)initialise readout map!";
    edm::ESTransientHandle<GEMEMap> eMap;
    es.get<GEMEMapRcd>().get(eMap);
    romap = eMap->convert();
    LogTrace("") <<" GEM READOUT MAP VERSION: " << eMap->version() << std::endl;
  }
}


void GEMUnpackingModule::produce(edm::Event & ev, const edm::EventSetup& es)
{

    edm::ESTransientHandle<GEMEMap> eMap;
    es.get<GEMEMapRcd>().get(eMap);
    romap = eMap->convert();
    LogDebug("GEMUnpacker::produce") <<" GEM READOUT MAP VERSION: " << eMap->version() << std::endl;


//  std::auto_ptr<GEMDigiCollection> producedGEMDigis(new GEMDigiCollection);
    auto producedGEMDigis = std::make_unique<GEMDigiCollection>();  

  AMCGEMData  gem;
  AMCGEBData  geb;
  AMCVFATData vfat;

  static bool debug = edm::MessageDrop::instance()->debugEnabled;
  debug = true;
  if (debug) LogDebug ("GEMUnpacker::produce") <<"Beginning To Unpack Event: "<<eventCounter_;
 
  edm::Handle<FEDRawDataCollection> allFEDRawData; 
  ev.getByToken(dataLabel_,allFEDRawData);


  int status = 0;

  //  for (int fedId= FEDNumbering::MINGEMFEDID; fedId<=FEDNumbering::MAXGEMFEDID; ++fedId){  
  int fedId=999;
  const FEDRawData & rawData = allFEDRawData->FEDData(fedId);
  //  int triggerBX =0;
  int nWords = rawData.size()/sizeof(Word64);
  if (nWords>0) {
    // check headers // No FED header for the moment
    //
    /*
    const Word64* header = reinterpret_cast<const Word64* >(rawData.data()); header--;
    bool moreHeaders = true;
    while (moreHeaders) {
      header++;
      std::cout <<" Header "<< header<<std::endl;
      FEDHeader fedHeader( reinterpret_cast<const unsigned char*>(header));
      std::string fedh( reinterpret_cast<const char*>(header));
      std::cout <<" Header  "<< header<<"  |"<<fedh<<"|"<<std::endl;
      if (!fedHeader.check()) {
        if (debug) LogTrace("") <<" ** PROBLEM **, header.check() failed, break"; 
        break; 
      }
      if ( fedHeader.sourceID() != fedId) {

        if (debug) LogTrace ("") <<" ** PROBLEM **, fedHeader.sourceID() != fedId"
				 << "fedId = " << fedId<<" sourceID="<<fedHeader.sourceID(); 
      }
    }
    */
    //
    // check trailers // No FED Trailer for the moment
    //
    /*
    const Word64* trailer=reinterpret_cast<const Word64* >(rawData.data())+(nWords-1); trailer++;
    bool moreTrailers = true;
    while (moreTrailers) {
      trailer--;
      FEDTrailer fedTrailer(reinterpret_cast<const unsigned char*>(trailer));
      std::string fedt( reinterpret_cast<const char*>(trailer));
      std::cout <<" Trailer "<< trailer<<"  |"<<fedt<<"|"<<std::endl;
      if ( !fedTrailer.check()) {
        if (debug) LogTrace("") <<" ** PROBLEM **, trailer.check() failed, break";
        break;
      }
      if ( fedTrailer.lenght()!= nWords) {
        if (debug) LogTrace("")<<" ** PROBLEM **, fedTrailer.lenght()!= nWords, break";
        break;
      }
      moreTrailers = fedTrailer.moreTrailers();
    }
    */
    //
    // data records
    //
    // tmp in absence of the FED header and FED trailer
    const Word64* header  = reinterpret_cast<const Word64* >(rawData.data());
    const Word64* trailer = reinterpret_cast<const Word64* >(rawData.data())+(nWords-1); 

    if (debug) {
      std::ostringstream str;
      int line = 0, ivfat = 0;
      uint64_t LV1ID = 0, sumVFAT = 0, ui64 = 0;
      uint32_t bx = 0;
      vfat.lsData = 0;
      vfat.msData = 0;

      for (const Word64* word = header; word <= trailer; word++) {
        line++;
	std::string myd( reinterpret_cast<const char*>(word));
        //std::cout << " line " << line << " *word 0x" << std::hex << *word << std::dec <<"  Char "<< myd << std::endl;
        //str<<"    data: "<< *(reinterpret_cast<const std::bitset<64>*>(word)) << std::endl; 
	  //std::cout <<std::dec<<"line "<<line<<" 64 bits 0x"<<std::hex<<*word<<std::endl;
	switch (line){
        case 1: 
          gem.header1 = *word;
          LV1ID       = (0x00ffffff00000000 & gem.header1) >> 32;
            break;
        case 2: 
          gem.header2 = *word; 
            break;
        case 3: 
          gem.header3 = *word; 
            break;
        case 4: 
          geb.header  = *word; 
          sumVFAT     = (0x000000000fffffff & geb.header);
            break;
        case 5: 
          geb.runhed  = *word; 
            break;
        }//end case

        // VFATs
        if ( line-5 > 0 &&  line < 5+(int)sumVFAT*4+1 ){ 
          if ((line-5)%4 == 1) ivfat++;
          //std::cout <<" line "<< line <<" vfat in "<< (line-5)%3 <<" ivfat "<< ivfat << std::endl;

          if ((line-5)%4 == 1) ui64 = *word;
          if ((line-5)%4 == 2) vfat.BXfrOH = *word;
          if ((line-5)%4 == 3) vfat.lsData = *word;
          if ((line-5)%4 == 0){
            vfat.msData = *word;

            vfat.BC     = ( 0xffff000000000000 & ui64 ) >> 48;
            vfat.EC     = ( 0x0000ffff00000000 & ui64 ) >> 32;
	    vfat.ChipID = ( 0x00000000ffff0000 & ui64 ) >> 16;
            vfat.crc    =   0x000000000000ffff & ui64;
	    //            GEMDataAMCformat::printVFATdataBits(ivfat, vfat);
	    //std::cout <<std::hex<<" BC 0x"<<vfat.BC<<" EC 0x"<<vfat.EC<<" Chip 0x"<<vfat.ChipID<<" crc 0x"<<vfat.crc<<std::endl;

	    uint16_t ChipID = (0x0fff & vfat.ChipID);
	    //std::cout <<std::hex<<" ChipID 0x"<<ChipID<<std::dec<<std::endl;

            uint8_t chan0xf = 0;
            for(int chan = 0; chan < 128; ++chan) {
              if(chan < 64){
                chan0xf = ((vfat.lsData >> chan) & 0x1);
       	      } else {
                chan0xf = ((vfat.msData >> (chan-64)) & 0x1);
              }
              if (chan0xf != 0) {
		//std::cout <<" chan "<< chan <<" --"<< std::hex<<chan0xf <<"-- "<< std::endl; 

		//std::cout <<std::hex<<vfat.ChipID<<std::dec<<std::endl;
		GEMROmap::eCoord ec;
		ec.chamberId=31;
		ec.vfatId = ChipID;
		ec.channelId = chan;
		GEMROmap::dCoord dc = romap->hitPosition(ec);
		std::cout <<"Chamber "<<ec.chamberId<<" vfat 0x"<<std::hex<<ec.vfatId<<std::dec<<" chan="<<ec.channelId
			  <<" correspond to eta="<<dc.etaId<<" strip="<<dc.stripId<<std::endl;
		 
		GEMDigi digi(dc.stripId,bx);

		  // we can insert it to the collection defining first which GEMDetId it belongs
		producedGEMDigis.get()->insertDigi(GEMDetId(1,1,1,1,1,dc.etaId),digi);	    
	       
	      }
	    }

	  }
        }//end VFATs
      }//end for words

      std::cout <<"  Number of words found "<< nWords << " sumVFAT " << sumVFAT << " LV1ID " << LV1ID << std::endl;        
      LogTrace("") << str.str();
    }
//  if (triggerBX != 51) continue;
//  if (triggerBX != 2316) continue;
    for (const Word64* word = header; word <= trailer; word++) {
      for( int iRecord=1; iRecord<=4; iRecord++){
	//const DataRecord::Data* pRecord = reinterpret_cast<const DataRecord::Data* >(word+1)-iRecord;
	//producedGEMDigis.get(), producedRawDataCounts.get(), producedRawSynchoCounts.get()); 
      }
    }
  }
  
  if (status && debug) LogTrace("")<<" GEMUnpackingModule - There was unpacking PROBLEM in this event"<<std::endl;
  //if (debug) LogTrace("") << DebugDigisPrintout()(producedGEMDigis.get()) <<std::endl;
  ev.put(std::move(producedGEMDigis));
}
