import FWCore.ParameterSet.Config as cms

process = cms.Process("GEMR2D")

process.load("EventFilter.GEMRawToDigi.GEMSQLiteCabling_cfi")
process.GEMCabling.connect = 'sqlite_file:GEMEMap.db'
process.load("EventFilter.GEMRawToDigi.gemUnpacker_cfi")
process.gemunpacker.InputLabel = cms.InputTag("TBData","GEMTBData")


# set maxevents; -1 -> take all
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1))

process.source = cms.Source ("PoolSource",fileNames = cms.untracked.vstring( 'file:myOutputFile.root'))
#process.source = cms.Source ("NewEventStreamFileReader",fileNames = cms.untracked.vstring( 'file:myOutputFile.root'))


process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger = cms.Service("MessageLogger",
    debugModules = cms.untracked.vstring('gemunpacker'),
    destinations = cms.untracked.vstring('cout'),
    cout = cms.untracked.PSet( threshold = cms.untracked.string('WARNING'))
)

process.out = cms.OutputModule("PoolOutputModule",
    fileName =  cms.untracked.string('file:outPut.root'),
    outputCommands = cms.untracked.vstring("keep *")
)

process.p = cms.Path(process.gemunpacker)
process.ep = cms.EndPath(process.out)
