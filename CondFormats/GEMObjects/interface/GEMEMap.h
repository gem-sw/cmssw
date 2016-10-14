#ifndef GEMEMap_H
#define GEMEMap_H
#include <map>
#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <boost/cstdint.hpp>

struct GEMEMapItem {
  int ChamberID;
  std::vector<int> VFatIDs;
  std::vector<int> positions;
};

class GEMROmap;
class GEMEMap {
public:
  GEMEMap(const std::string & version = "")
    : theVersion(version) { }
  virtual ~GEMEMap(){}

  const std::string & version() const{
    return theVersion;
  }

  GEMROmap* convert() const;

  struct GEMVFatMaptype {
    int VFATmapTypeId;
    std::vector<std::string> subdet;
    std::vector<std::string> sector;
    std::vector<std::string> type;
    std::vector<int> vfat_position;
    std::vector<int> z_direction;
    std::vector<int> iEta;
    std::vector<int> iPhi;
    std::vector<int> depth;
    std::vector<int> strip_number;//localStrip;
    std::vector<int> vfat_chnnel_number;//channel;
    std::vector<int> px_connector_pin;
  };
  struct GEMVFatMapInPos {
    int position;
    int VFATmapTypeId;
  };

  std::vector<GEMEMapItem>     theEMapItem;
  std::vector<GEMVFatMaptype>  theVFatMaptype;
  std::vector<GEMVFatMapInPos> theVFatMapInPos;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    { ar & deg; }
  int deg;
  
private:
  std::string theVersion;
};

#endif // GEMEMap_H
