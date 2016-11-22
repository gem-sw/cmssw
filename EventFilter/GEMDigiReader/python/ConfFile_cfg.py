import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1200)) # 370001

process.source = cms.Source("PoolSource",
    # replace 'myGEMDigiFile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
        'file:myGEMFedRawDataFile.root'
    )
)

process.demo = cms.EDAnalyzer('GEMDigiReader',
  # Label to retrieve Digis from the event 
    InputLabel = cms.InputTag('gemDigis')
)


process.p = cms.Path(process.demo)
