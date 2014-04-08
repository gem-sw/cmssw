#ifndef GEMBaseValidation_H
#define GEMBaseValidation_H

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Utilities/interface/InputTag.h"

class GEMBaseValidation
{
public:
  GEMBaseValidation(DQMStore* dbe,
                         const edm::InputTag & inputTag);
  virtual ~GEMBaseValidation();
  void setGeometry(const GEMGeometry* geom);
  virtual void bookHisto(const GEMGeometry* geom) = 0 ;
  virtual void analyze(const edm::Event& e, const edm::EventSetup&) = 0 ;
  MonitorElement* BookHistZR( const char* name, const char* label, unsigned int region_num, unsigned int station_num, unsigned int layer_num); 
protected:

  std::vector< std::string > regionLabel;
  std::vector< std::string > layerLabel;
  std::vector< std::string > stationLabel;
  double* xy_range;
  double* zr_range;

  DQMStore* dbe_;
  edm::InputTag theInputTag;
  const GEMGeometry* theGEMGeometry;
};

#endif
