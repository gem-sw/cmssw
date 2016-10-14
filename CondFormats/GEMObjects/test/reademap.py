import FWCore.ParameterSet.Config as cms

process = cms.Process("DBTest")
process.load("CondCore.DBCommon.CondDBSetup_cfi")

process.source = cms.Source("EmptySource")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)
process.GEMCabling = cms.ESSource("PoolDBESSource",
    process.CondDBSetup,
    toGet = cms.VPSet(cms.PSet(
        record = cms.string('GEMEMapRcd'),
        tag = cms.string('GEMEMap_v2')
    )),
    connect = cms.string('sqlite_file:GEMEMap.db')
)

process.reader = cms.EDAnalyzer("GEMReadOutMapAnalyzer",
    useNewEMap = cms.untracked.bool(True)
)

process.p1 = cms.Path(process.reader)


