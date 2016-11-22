import FWCore.ParameterSet.Config as cms

from CondCore.CondDB.CondDB_cfi import *
CondDBSetup.DBParameters.authenticationPath = cms.untracked.string('/afs/cern.ch/cms/DB/conddb')
GEMCabling = cms.ESSource("PoolDBESSource",
    CondDBSetup,
    toGet = cms.VPSet(cms.PSet(
        record = cms.string('GEMEMapRcd'),
        tag = cms.string('GEMEMap_v2')
    )),
    connect = cms.string('sqlite_file:GEMEMap_181007.db')
)


