import FWCore.ParameterSet.Config as cms

process = cms.Process("RAW")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(5000) )


# This should be changed to another kind of source, to make the event assignation more logical
process.source = cms.Source("EmptySource"
#    setRunNumber=cms.untracked.uint32(999)
)


process.TBData = cms.EDProducer('GEMFedRawProducer'
)

process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('myOutputFile.root')
)

  
process.p = cms.Path(process.TBData)

process.e = cms.EndPath(process.out)
