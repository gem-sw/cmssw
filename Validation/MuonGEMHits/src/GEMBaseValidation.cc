#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
#include "DQMServices/Core/interface/DQMStore.h"

using namespace std;
GEMBaseValidation::GEMBaseValidation(DQMStore* dbe,
                                               const edm::InputTag & inputTag)
{
  dbe_ = dbe;
  theInputTag = inputTag;
  regionLabel.push_back("-1");
  regionLabel.push_back("1" );

  stationLabel.push_back("1");
  stationLabel.push_back("2_short");
  stationLabel.push_back("2_long");

  layerLabel.push_back("1");
  layerLabel.push_back("2");

  double temp_zr_range[] = { -572,-564,-794,-786,-802,-794,564,572,786,794,794,802} ;
  double temp_xy_range[] = { -200,200,-400,400,-400,400} ;
  zr_range = temp_zr_range;
  xy_range = temp_xy_range;

}


GEMBaseValidation::~GEMBaseValidation() {
}
void GEMBaseValidation::setGeometry(const GEMGeometry* geom)
{ 
    theGEMGeometry = geom;
}

MonitorElement* GEMBaseValidation::BookHistZR( const char* name, const char* label, unsigned int region_num, unsigned int station_num, unsigned int layer_num) {
  double zr_range[2][3][6] = { { { 200,-572,-564, 110,130,240 },  {200,-794,-786,140,190,330}, {200,-802,-794,210,120,330}},{{200,564,572,110,130,240},{200,786,794,140,190,330},{200,794,802,210,120,330}}  } ;
	string hist_name  = name+string("_zr_r") + regionLabel[region_num]+"_st"+stationLabel[station_num]+"_l"+layerLabel[layer_num];
  string hist_label = label+string(" occupancy : region")+regionLabel[region_num]+" station "+stationLabel[station_num]+" layer "+layerLabel[layer_num]+" "+" ; globalZ [cm]; globalR[cm]";

  int xbin = (int)zr_range[region_num][station_num][0]; 
  double xmin = zr_range[region_num][station_num][1]; 
  double xmax = zr_range[region_num][station_num][2]; 
  int ybin = (int)zr_range[region_num][station_num][3];
  double ymin = zr_range[region_num][station_num][4]; 
  double ymax = zr_range[region_num][station_num][5]; 
 
  return dbe_->book2D( hist_name, hist_label, xbin, xmin, xmax, ybin,ymin, ymax);


}
