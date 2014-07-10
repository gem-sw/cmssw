import FWCore.ParameterSet.Config as cms

simGemTriggerDigisSelector = cms.EDProducer("GEMTriggerDigisSelector",
    GEMPadDigiProducer = cms.InputTag("simMuonGEMCSCPadDigis"),
    ## on average how many bad half-strips per chamber
    badStrips = cms.double(0),
    ## on average how many bad partitions per chamber
    badRolls = cms.double(0),
    ## on average how many bad layers per chamber
    badLayers = cms.double(0),
    ## on average how many bad chambers
    badChambers = cms.double(0) 
)
