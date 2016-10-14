#include "CondTools/GEM/interface/GEMEMapSourceHandler.h"
#include "FWCore/ParameterSet/interface/ParameterSetfwd.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"

#include <fstream>
#include <cstdlib> 
popcon::GEMEMapSourceHandler::GEMEMapSourceHandler(const edm::ParameterSet& ps) :
  m_name(ps.getUntrackedParameter<std::string>("name","GEMEMapSourceHandler")),
  m_dummy(ps.getUntrackedParameter<int>("WriteDummy",0)),
  m_validate(ps.getUntrackedParameter<int>("Validate",1)),
  m_connect(ps.getUntrackedParameter<std::string>("OnlineConn","")),
  m_authpath(ps.getUntrackedParameter<std::string>("OnlineAuthPath","."))
{
}

popcon::GEMEMapSourceHandler::~GEMEMapSourceHandler()
{
}

void popcon::GEMEMapSourceHandler::getNewObjects()
{

//	std::cout << "GEMEMapSourceHandler: GEMEMapSourceHandler::getNewObjects begins\n";

  edm::Service<cond::service::PoolDBOutputService> mydbservice;

// first check what is already there in offline DB
  Ref payload;
  if(m_validate==1 && tagInfo().size>0) {
    std::cout<<" Validation was requested, so will check present contents"<<std::endl;
    std::cout<<"Name of tag : "<<tagInfo().name << ", tag size : " << tagInfo().size 
            << ", last object valid since " 
	    << tagInfo().lastInterval.first << std::endl;  
    payload = lastPayload();
  }

  // now construct new cabling map from online DB
  time_t rawtime;
  time(&rawtime); //time since January 1, 1970
  tm * ptm = gmtime(&rawtime);//GMT time
  char buffer[20];
  strftime(buffer,20,"%d/%m/%Y_%H:%M:%S",ptm);
  std::string eMap_version=(std::string)buffer;
  std::cout <<" eMap version = "<<eMap_version<<std::endl;

  eMap =  new GEMEMap(eMap_version);

  std::string baseCMS = std::string(getenv("CMSSW_BASE"))+std::string("/src/CondTools/GEM/data/");  
  std::vector<std::string> mapfiles;
  // mapfiles.push_back("v2b_schema_chips0-1.csv");
  // mapfiles.push_back("v2b_schema_chips2-15.csv");
  // mapfiles.push_back("v2b_schema_chips16-17.csv");
  // mapfiles.push_back("v2b_schema_chips18-23.csv");

  // mapfiles.push_back("GE1M01_D1_StripToPxToVFATChanMap.csv");
  // mapfiles.push_back("GE1M01_D2_StripToPxToVFATChanMap.csv");

  mapfiles.push_back("GEM_GE1M_Depth1_ChannelsFromDB_Sept_01_2016.csv");
  mapfiles.push_back("GEM_GE1M_Depth2_ChannelsFromDB_Sept_01_2016.csv");
  mapfiles.push_back("GEM_GE1P_Depth1_ChannelsFromDB_Sept_01_2016.csv");
  mapfiles.push_back("GEM_GE1P_Depth2_ChannelsFromDB_Sept_01_2016.csv");

  for (unsigned int ifm=0;ifm<mapfiles.size();ifm++){  
    GEMEMap::GEMVFatMaptype vmtype;
    std::string filename(baseCMS+mapfiles[ifm]);
    std::cout <<"Filename = "<<filename<<std::endl;
    vmtype.VFATmapTypeId=ifm+1;//this is 1 and 2 if there are two input files
    std::ifstream maptype(filename.c_str());
    std::string buf("");


    std::string field, line;
    while(std::getline(maptype, line)){
      //mapping v1:      VFAT_POSN	Z	IETA	IPHI	DEPTH	Detector Strip Number	VFAT channel Number	Px Connector Pin #
      //mapping v2:      SUBDET	SECTOR	TYPE	ZPOSN	IETA	IPHI	DEPTH	VFAT_POSN	DET_STRIP	VFAT_CHAN	CONN_PIN

      std::string sdet, sec, typ;
      int vfat_pos, z_dir, ieta, iphi, dep, str_num, vfat_chn_num, px_con_pin;
      std::stringstream ssline(line);

      getline( ssline, field, ',' );
      std::stringstream Sdet(field);
      getline( ssline, field, ',' );
      std::stringstream Sec(field);
      getline( ssline, field, ',' );
      std::stringstream Typ(field);
      getline( ssline, field, ',' );
      std::stringstream Z_dir(field);
      getline( ssline, field, ',' );
      std::stringstream Ieta(field);
      getline( ssline, field, ',' );
      std::stringstream Iphi(field);
      getline( ssline, field, ',' );
      std::stringstream Dep(field);
      getline( ssline, field, ',' );
      std::stringstream Vfat_pos(field);
      getline( ssline, field, ',' );
      std::stringstream Str_num(field);
      getline( ssline, field, ',' );
      std::stringstream Vfat_chn_num(field);
      getline( ssline, field, ',' );
      std::stringstream Px_con_pin(field);

      Sdet >> sdet; Sec >> sec; Typ >> typ; Z_dir >> z_dir; Ieta >> ieta; Iphi >> iphi; Dep >> dep; Vfat_pos >> vfat_pos; Str_num >> str_num; Vfat_chn_num >> vfat_chn_num; Px_con_pin >>  px_con_pin;

      std::cout<<"Subdet="<<sdet<<" Sector="<<sec<<" Type="<<typ<<" z_direction="<<z_dir<<" ieta="<<ieta<<" iphi="<<iphi<<" depth="<<dep<<" vfat position="<<vfat_pos<<" strip no.="<<str_num<<" vfat channel no.="<<vfat_chn_num<<" Px connector pin="<<px_con_pin<<std::endl;

      vmtype.subdet.push_back(sdet);
      vmtype.sector.push_back(sec);
      vmtype.type.push_back(typ);
      vmtype.vfat_position.push_back(vfat_pos);
      vmtype.z_direction.push_back(z_dir);
      vmtype.iEta.push_back(ieta);
      vmtype.iPhi.push_back(iphi);
      vmtype.depth.push_back(dep);
      vmtype.strip_number.push_back(str_num);
      vmtype.vfat_chnnel_number.push_back(vfat_chn_num);
      vmtype.px_connector_pin.push_back(px_con_pin);
    }
    eMap->theVFatMaptype.push_back(vmtype);
  }
  
  // GEMEMap::GEMVFatMapInPos inPos;
  // for (int p=0;p<2;p++){
  //   inPos.position = p;
  //   inPos.VFATmapTypeId = 1;
  //   eMap->theVFatMapInPos.push_back(inPos);
  // }
  // for (int p=2;p<16;p++){
  //   inPos.position = p;
  //   inPos.VFATmapTypeId = 2;
  //   eMap->theVFatMapInPos.push_back(inPos);
  // }
  // for (int p=16;p<18;p++){
  //   inPos.position = p;
  //   inPos.VFATmapTypeId = 3;
  //   eMap->theVFatMapInPos.push_back(inPos);
  // }
  // for (int p=18;p<24;p++){
  //   inPos.position = p;
  //   inPos.VFATmapTypeId = 4;
  //   eMap->theVFatMapInPos.push_back(inPos);
  // }

  // GEMEMapItem mp;  
  // mp.ChamberID = 31;
  // mp.VFatIDs.push_back(0xe9f);
  // mp.positions.push_back(2);
  // mp.VFatIDs.push_back(0xe7b);
  // mp.positions.push_back(3);
  // mp.VFatIDs.push_back(0xe5b);
  // mp.positions.push_back(4);
  // mp.VFatIDs.push_back(0xe8b);
  // mp.positions.push_back(10);
  // mp.VFatIDs.push_back(0xaaf);
  // mp.positions.push_back(11);
  // mp.VFatIDs.push_back(0xb44);
  // mp.positions.push_back(12);
  // mp.VFatIDs.push_back(0xea7);
  // mp.positions.push_back(13);
  // mp.VFatIDs.push_back(0xe78);
  // mp.positions.push_back(18);
  // mp.VFatIDs.push_back(0xe94);
  // mp.positions.push_back(19);
  // mp.VFatIDs.push_back(0xb40);
  // mp.positions.push_back(20);
  // mp.VFatIDs.push_back(0xe98);
  // mp.positions.push_back(21);

  // eMap->theEMapItem.push_back(mp);
  
  cond::Time_t snc=mydbservice->currentTime();
  
  // look for recent changes
  int difference=1;
  if (difference==1) {
    m_to_transfer.push_back(std::make_pair((GEMEMap*)eMap,snc));
  }
  
  std::cout << "GEMEMapSourceHandler: GEMEMapSourceHandler::getNewObjects ends\n";
}

