import FWCore.ParameterSet.Config as cms

process = cms.Process("Write2DB")
process.load("CondCore.DBCommon.CondDBCommon_cfi")
process.CondDBCommon.connect = 'sqlite_file:GEMEMap.db'

process.MessageLogger = cms.Service("MessageLogger",
    cout = cms.untracked.PSet(
        default = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        )
    ),
    destinations = cms.untracked.vstring('cout')
)

process.source = cms.Source("EmptyIOVSource",
    lastValue = cms.uint64(1),
    timetype = cms.string('runnumber'),
    firstValue = cms.uint64(1),
    interval = cms.uint64(1)
)

process.PoolDBOutputService = cms.Service("PoolDBOutputService",
    process.CondDBCommon,
    logconnect = cms.untracked.string('sqlite_file:GEMEMap_log.db'),
    toPut = cms.VPSet(cms.PSet(
        record = cms.string('GEMEMapRcd'),
        tag = cms.string('GEMEMap_v2')
    ))
)

process.WriteInDB = cms.EDAnalyzer("GEMEMapDBWriter",
    SinceAppendMode = cms.bool(True),
    record = cms.string('GEMEMapRcd'),
    loggingOn = cms.untracked.bool(False),
    Source = cms.PSet(
        OnlineAuthPath = cms.untracked.string('.'),
        Validate = cms.untracked.int32(0),
        # OnlineConn = cms.untracked.string('oracle://cms_omds_lb/CMS_GEM_CONF')
    )
)

process.p = cms.Path(process.WriteInDB)

