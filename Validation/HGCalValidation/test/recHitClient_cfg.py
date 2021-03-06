import FWCore.ParameterSet.Config as cms
import os

process = cms.Process("CLIENT")

process.load("Configuration.StandardSequences.Reconstruction_cff") #### ???????
process.load('Configuration.Geometry.GeometryExtended2023HGCalMuonReco_cff')
process.load ('Configuration.Geometry.GeometryExtended2023HGCalMuon_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')

process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 1

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.autoCond import autoCond
process.GlobalTag.globaltag = autoCond['upgradePLS3']


process.load("Validation.HGCalValidation.HGCalRecHitsClient_cfi")
process.hgcalRecHitClientEE.Verbosity     = 2
process.hgcalRecHitClientHEF = process.hgcalRecHitClientEE.clone(
    DetectorName = cms.string("HGCalHESiliconSensitive"))
process.hgcalRecHitClientHEB = process.hgcalRecHitClientEE.clone(
    DetectorName = cms.string("HGCalHEScintillatorSensitive"))


process.load("DQMServices.Core.DQM_cfg")
process.DQM.collectorHost = ''

# summary
process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) ) ## ??????

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
        )

process.source = cms.Source("PoolSource",
                            fileNames = cms.untracked.vstring('file:./test_output_rechitVal.root')
                            )


process.load("Configuration.StandardSequences.EDMtoMEAtRunEnd_cff")
process.dqmSaver.referenceHandling = cms.untracked.string('all')

cmssw_version = os.environ.get('CMSSW_VERSION','CMSSW_X_Y_Z')
Workflow = '/HGCalValidation/'+'Harvesting/'+str(cmssw_version)
process.dqmSaver.workflow = Workflow

process.load("Validation.HGCalValidation.HGCalRecHitsClient_cfi")
process.hgcalRecHitClientEE.outputFile = 'HGCalRecHitsHarvestingEE.root'
process.hgcalRecHitClientHEF.outputFile = 'HGCalRecHitsHarvestingHEF.root'
process.hgcalRecHitClientHEB.outputFile = 'HGCalRecHitsHarvestingHEB.root'

process.p = cms.Path(process.EDMtoME *
                     process.hgcalRecHitClientEE *
                     process.hgcalRecHitClientHEF *
                     process.hgcalRecHitClientHEB *
                     process.dqmSaver)
