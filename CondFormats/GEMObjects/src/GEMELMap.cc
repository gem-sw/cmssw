#include "CondFormats/GEMObjects/interface/GEMELMap.h"
#include "CondFormats/GEMObjects/interface/GEMROmap.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"

GEMELMap::GEMELMap():
  theVersion("") {}

GEMELMap::GEMELMap(const std::string & version):
  theVersion(version) {}

GEMELMap::~GEMELMap() {}

const std::string & GEMELMap::version() const{
  return theVersion;
}

void GEMELMap::convert(GEMROmap & romap) {
  // amc->geb->vfat mapping to GEMDetId
  for (auto imap : theVFatMap_){
    for (unsigned int ix=0;ix<imap.vfatId.size();ix++) {
      if (imap.vfatType[ix] < 10) {
	// vfat v2
	GEMROmap::eCoord ec;
	ec.vfatId = imap.vfatId[ix] & chipIdMask_;
	ec.gebId = imap.gebId[ix];
	ec.amcId = imap.amcId[ix];

	int st = std::abs(imap.z_direction[ix]);
	GEMROmap::dCoord dc;
	dc.gemDetId = GEMDetId(imap.z_direction[ix], 1, st, imap.depth[ix], imap.sec[ix], imap.iEta[ix]);
	dc.vfatType = imap.vfatType[ix];
	dc.locPhi = (imap.iPhi[ix]-1)%maxVFatGE11_;
	romap.add(ec,dc);
	romap.add(dc,ec);
	
	GEMROmap::eCoord ecGEB;
	ecGEB.vfatId = 0;
	ecGEB.gebId = ec.gebId;
	ecGEB.amcId = ec.amcId;
	// this is for backward compatablity for v2
	if (!romap.isValidChipID(ecGEB)) {
	  GEMROmap::dCoord dcGEB;
	  dcGEB.gemDetId = dc.gemDetId.chamberId();
	  dcGEB.vfatType = dc.vfatType;
	  romap.add(ecGEB,dcGEB);
	  romap.add(dcGEB,ecGEB);
	}
      }
      else {
	// vfat v3
	GEMROmap::eCoord ec;
	ec.vfatId = imap.vfatId[ix]; // vfatId is chip position in v3
	ec.gebId = imap.gebId[ix];
	ec.amcId = imap.amcId[ix];

	int st = std::abs(imap.z_direction[ix]);
	GEMROmap::dCoord dc;
	dc.gemDetId = GEMDetId(imap.z_direction[ix], 1, st, imap.depth[ix], imap.sec[ix], imap.iEta[ix]);
	dc.vfatType = imap.vfatType[ix];
	int maxVFat = maxVFatGE11_;
	if (st == 2) maxVFat = maxVFatGE21_;      
	if (st == 0) maxVFat = maxVFatGE0_;
	dc.locPhi = (imap.iPhi[ix]-1)%maxVFat;
	romap.add(ec,dc);
	romap.add(dc,ec);
      }
    }
  }

  // channel mapping
  for (auto imap : theStripMap_){
    for (unsigned int ix=0;ix<imap.vfatType.size();ix++){
      GEMROmap::channelNum cMap;
      cMap.vfatType = imap.vfatType[ix];
      cMap.chNum = imap.vfatCh[ix];

      GEMROmap::stripNum sMap;
      sMap.vfatType = imap.vfatType[ix];
      sMap.stNum = imap.vfatStrip[ix];

      romap.add(cMap, sMap);
      romap.add(sMap, cMap);
    }
  }
}

void GEMELMap::convertDummy(GEMROmap & romap) {
  // 12 bits for vfat, 5 bits for geb, 8 bit long GLIB serial number
  uint16_t amcId = 1; //amc
  uint16_t gebId = 0; 
  
  for (int re = -1; re <= 1; re = re+2) {
    for (int st = GEMDetId::minStationId; st<=GEMDetId::maxStationId; ++st) {
      int maxVFat = maxVFatGE11_;
      if (st == 2) maxVFat = maxVFatGE21_;      
      if (st == 0) maxVFat = maxVFatGE0_;
      
      for (int ch = 1; ch<=GEMDetId::maxChamberId; ++ch) {
	for (int ly = 1; ly<=GEMDetId::maxLayerId; ++ly) {
	  // 1 geb per chamber
	  gebId++;
	  uint16_t chipPos = 0;
	  for (int lphi = 0; lphi < maxVFat; ++lphi) {
	    for (int roll = 1; roll<=maxEtaPartition_; ++roll) {
	    
	      GEMDetId gemId(re, 1, st, ly, ch, roll);
		
	      GEMROmap::eCoord ec;
	      ec.vfatId = chipPos;
	      ec.gebId = gebId;
	      ec.amcId = amcId;
		
	      GEMROmap::dCoord dc;
	      dc.gemDetId = gemId;
	      dc.vfatType = 11;// > 10 is vfat v3
	      dc.locPhi = lphi;
		
	      romap.add(ec,dc);
	      romap.add(dc,ec);
	      chipPos++;
	    }
	  }
	
	  // 5 bits for gebId 
	  if (gebId == maxGEBs_){
	    // 24 gebs per amc
	    gebId = 0;
	    amcId++;
	  }
	}
      }
    }
  }
  
  for (int i = 0; i < maxChan_; ++i) {
    // only 1 vfat type for dummy map
    GEMROmap::channelNum cMap;
    cMap.vfatType = 11;
    cMap.chNum = i;

    GEMROmap::stripNum sMap;
    sMap.vfatType = 11;
    sMap.stNum = i;

    romap.add(cMap, sMap);
    romap.add(sMap, cMap);
  }
}
