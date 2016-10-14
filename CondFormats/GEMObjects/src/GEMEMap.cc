#include "CondFormats/GEMObjects/interface/GEMEMap.h"
#include "CondFormats/GEMObjects/interface/GEMROmap.h"

GEMROmap* GEMEMap::convert() const{
  GEMROmap* romap=new GEMROmap();
  
  for (std::vector<GEMEMapItem>::const_iterator i=this->theEMapItem.begin(); i<this->theEMapItem.end(); i++){
    
    std::vector<int>::const_iterator p=i->positions.begin();
    for (std::vector<int>::const_iterator v=i->VFatIDs.begin(); v<i->VFatIDs.end(); v++,p++){
      int position = *p;
      int sPhi = position/8;
      
      std::vector<GEMEMap::GEMVFatMapInPos>::const_iterator ipos;
      int mid=-1;
      for (ipos=this->theVFatMapInPos.begin();ipos<this->theVFatMapInPos.end();ipos++){
	if (ipos->position == position){
	  mid = ipos->VFATmapTypeId;
	  
	  std::vector<GEMEMap::GEMVFatMaptype>::const_iterator imap;
	  for (imap=this->theVFatMaptype.begin(); imap<this->theVFatMaptype.end();imap++){
	    if (mid == imap->VFATmapTypeId){
	      for (unsigned int ix=0;ix<imap->strip_number.size();ix++){
		GEMROmap::eCoord ec;
		ec.chamberId=i->ChamberID;
		ec.vfatId=*v;
		ec.channelId=imap->vfat_chnnel_number[ix];
		GEMROmap::dCoord dc;
		dc.etaId = 8-position%8;
		dc.stripId = imap->strip_number[ix]+sPhi*128;
		romap->add(ec,dc);
	      }
	    }
	  }
	}
      }
    }
  }
  return romap;
}
