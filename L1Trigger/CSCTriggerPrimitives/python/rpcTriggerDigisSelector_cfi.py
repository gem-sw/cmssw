import FWCore.ParameterSet.Config as cms

simRpcTriggerDigisSelector = cms.EDProducer("RPCTriggerDigisSelector",
    RPCDigiDigiProducer = cms.InputTag("simMuonRPCDigis"),
    ## on average how many bad half-strips per chamber
    badStrips = cms.double(0),
    ## on average how many bad wiregroups per chamber
    badRolls = cms.double(0),
    ## on average how many bad chambers
    badChambers = cms.double(0),
)

