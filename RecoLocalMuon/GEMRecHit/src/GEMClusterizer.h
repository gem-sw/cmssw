#ifndef RecoLocalMuon_GEMClusterizer_h
#define RecoLocalMuon_GEMClusterizer_h
/** \class GEMClusterizer
 *  \author M. Maggi -- INFN Bari
 */

#include "GEMClusterContainer.h"
#include "GEMEtaPartitionMask.h"
#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"

class GEMCluster;
class GEMClusterizer{
 public:
  GEMClusterizer();
  ~GEMClusterizer();
  GEMClusterContainer doAction(const GEMDigiCollection::Range& digiRange, const EtaPartitionMask& mask);

 private:
  GEMClusterContainer cls;
};
#endif
