#include <iostream>

#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CondFormats/GEMObjects/interface/GEMReadOutMapping.h"
#include "CondFormats/DataRecord/interface/GEMReadOutMappingRcd.h"
#include "CondFormats/GEMObjects/interface/GEMEMap.h"
#include "CondFormats/DataRecord/interface/GEMEMapRcd.h"


using namespace std;
using namespace edm;

// class declaration
class GEMReadOutMapAnalyzer : public edm::EDAnalyzer {
   public:
      explicit GEMReadOutMapAnalyzer( const edm::ParameterSet& );
      ~GEMReadOutMapAnalyzer();
      virtual void analyze( const edm::Event&, const edm::EventSetup& );
   private:
   bool m_flag;
};


GEMReadOutMapAnalyzer::GEMReadOutMapAnalyzer( const edm::ParameterSet& iConfig )
: m_flag(iConfig.getUntrackedParameter<bool>("useNewEMap",false))
{
  ::putenv(const_cast<char*>(std::string("CORAL_AUTH_USER konec").c_str()));
  ::putenv(const_cast<char*>(std::string("CORAL_AUTH_PASSWORD konecPass").c_str()));
}


GEMReadOutMapAnalyzer::~GEMReadOutMapAnalyzer(){}

void GEMReadOutMapAnalyzer::analyze( const edm::Event& iEvent, const edm::EventSetup& iSetup ) {

   std::cout << "====== GEMReadOutMapAnalyzer" << std::endl;

   const GEMReadOutMapping* map;
   if (m_flag) {
     edm::ESHandle<GEMEMap> readoutMapping;
     iSetup.get<GEMEMapRcd>().get(readoutMapping);
     const GEMEMap* eMap=readoutMapping.product();
//     GEMReadOutMapping* map = eMap->convert();
     map = eMap->convert();
   } else {
     edm::ESHandle<GEMReadOutMapping> map0;
     iSetup.get<GEMReadOutMappingRcd>().get(map0);
     map=map0.product();
   }
   cout << "version: " << map->version() << endl;

   pair<int,int> dccRange = map->dccNumberRange();
   cout <<" dcc range: " << dccRange.first <<" to "<<dccRange.second<<endl; 
   vector<const DccSpec *> dccs = map->dccList();
   typedef vector<const DccSpec *>::const_iterator IDCC;
   for (IDCC idcc = dccs.begin(); idcc != dccs.end(); idcc++) cout <<(**idcc).print(4);

   cout <<"--- --- --- --- --- --- --- --- ---"<<endl; 
   cout <<"--- --- --- --- --- --- --- --- ---"<<endl; 
/*
   ChamberRawDataSpec linkboard;
   linkboard.dccId = 790;
   linkboard.dccInputChannelNum = 1;
   linkboard.tbLinkInputNum = 1;
   linkboard.lbNumInLink = 2;

   int febInputNum=3;
   int stripPinNum=5;

   const LinkBoardSpec *location = map->location(linkboard);
   if (location) {
     location->print();
     const FebConnectorSpec * feb = location->feb( febInputNum);
     const ChamberStripSpec * strip = feb->strip(stripPinNum);
     feb->print();
     strip->print();
     cout <<" DETID: " << endl;
     uint32_t id = feb->rawId();
     cout << "uint32_t: " << id << endl;
     GEMDetId rpcDetId(id);
     cout << rpcDetId << endl;
  }
     
*/

   
}

//define this as a plug-in
DEFINE_FWK_MODULE(GEMReadOutMapAnalyzer);
