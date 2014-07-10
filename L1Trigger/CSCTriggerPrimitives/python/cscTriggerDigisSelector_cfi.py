import FWCore.ParameterSet.Config as cms

## filter to study the performance of the CSC system when at random wiregroups, half-strips or TMBs fail
simCscTriggerDigisSelector = cms.EDProducer("CSCTriggerDigisSelector",
    CSCComparatorDigiProducer = cms.InputTag("simMuonCSCDigis","MuonCSCComparatorDigi"),
    CSCWireDigiProducer = cms.InputTag("simMuonCSCDigis","MuonCSCWireDigi"),
    ## on average how many bad half-strips per chamber
    badStrips = cms.double(0),
    ## on average how many bad wiregroups per chamber
    badWGs = cms.double(0),
    ## on average how many bad layers per chamber
    badLayers = cms.double(0),
    ## fraction of bad CSC chambers
    badChambers = cms.double(0) 
)

