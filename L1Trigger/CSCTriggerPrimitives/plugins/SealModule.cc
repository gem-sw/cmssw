#include "FWCore/Framework/interface/MakerMacros.h"

#include <L1Trigger/CSCTriggerPrimitives/plugins/CSCTriggerPrimitivesProducer.h>
DEFINE_FWK_MODULE(CSCTriggerPrimitivesProducer);

#include <L1Trigger/CSCTriggerPrimitives/plugins/CSCTriggerDigisSelector.h>
DEFINE_FWK_MODULE(CSCTriggerDigisSelector);

#include <L1Trigger/CSCTriggerPrimitives/plugins/GEMTriggerDigisSelector.h>
DEFINE_FWK_MODULE(GEMTriggerDigisSelector);

#include <L1Trigger/CSCTriggerPrimitives/plugins/RPCTriggerDigisSelector.h>
DEFINE_FWK_MODULE(RPCTriggerDigisSelector);
