# Auto generated configuration file
# using: 
# Revision: 1.19 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: SingleMuPt100_cfi -s GEN,SIM,DIGI,L1,DIGI2RAW,RAW2DIGI,L1Reco,RECO --conditions auto:run2_mc --magField 38T_PostLS1 --datatier GEN-SIM --geometry GEMCosmicStand --eventcontent FEVTDEBUGHLT --era phase2_muon -n 100 --fileout out_reco.root
import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras

process = cms.Process('RECO',eras.phase2_muon)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Geometry.GEMGeometry.GeometryGEMCosmicStand_cff')
process.load('Configuration.StandardSequences.MagneticField_0T_cff')
#process.load('Configuration.StandardSequences.Generator_cff')
#process.load('IOMC.EventVertexGenerators.VtxSmearedRealistic50ns13TeVCollision_cfi')
#process.load('GeneratorInterface.Core.genFilterSummary_cff')
#process.load('Configuration.StandardSequences.SimIdeal_cff')
#process.load('Configuration.StandardSequences.Digi_cff')
#process.load('Configuration.StandardSequences.SimL1Emulator_cff')
#process.load('Configuration.StandardSequences.DigiToRaw_cff')
#process.load('Configuration.StandardSequences.RawToDigi_cff')
#process.load('Configuration.StandardSequences.L1Reco_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('SimMuon.GEMDigitizer.muonGEMDigi_cff')
process.load('RecoLocalMuon.GEMRecHit.gemLocalReco_cff')
#process.load('Configuration.StandardSequences.Validation_cff')

process.options = cms.untracked.PSet()

# Input source
process.load("EventFilter.GEMRawToDigi.GEMSQLiteCabling_cfi")

SLOTLIST=[]
VFATLIST=[]
COLUMNLIST=[]
ROWLIST=[]
LAYERLIST=[]
chamberName=[]
columnStand=[]
rowStand=[]
layerSC=[]
#import Validation.GEMCR.configureRun_cfi
import configureRun_cfi as runConfig
runConfig.configureRun(SLOTLIST,VFATLIST,COLUMNLIST,ROWLIST,LAYERLIST,chamberName,columnStand, rowStand, layerSC)

#adding in custom geometry
for i in range(len(chamberName)):
    if layerSC[i] == 2: continue
    row = rowStand[i]
    column = columnStand[i]
    addChamber = 'gem11{}_c{}_r{}.xml'.format(chamberName[i][10],column,row)
    print 'adding chamber', addChamber
    process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/'+addChamber)
    
process.GEMCabling.connect = 'sqlite_file:'+runConfig.sqlite_file

process.source = cms.Source("EmptySource",
      firstEvent = cms.untracked.uint32(1),
      firstRun = cms.untracked.uint32(runConfig.RunNumber)
)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(runConfig.MaxEvents))

process.muonGEMDigis = cms.EDProducer('GEMCosmicStandUnpacker',
      inputFileName=cms.string(runConfig.RAWFileName),
      slotVector=  cms.vint32(SLOTLIST),
      vfatVector=cms.vuint64(VFATLIST),
      columnVector=cms.vint32(COLUMNLIST),
      rowVector   =cms.vint32(ROWLIST),
      layerVector = cms.vint32(LAYERLIST),
      ##verbose=cms.untracked.bool(True)
)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('CosmicMuonGenerator nevts:100'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Output definition
process.FEVTDEBUGHLToutput = cms.OutputModule("PoolOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('RECO'),
        filterName = cms.untracked.string('')
    ),
    eventAutoFlushCompressedSize = cms.untracked.int32(10485760),
    fileName = cms.untracked.string(runConfig.OutputFileName),
    #outputCommands = process.FEVTDEBUGHLTEventContent.outputCommands,
    outputCommands = cms.untracked.vstring( ('keep *')),
    splitLevel = cms.untracked.int32(0)
)

# Additional output definition
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')


process.load('RecoMuon.TrackingTools.MuonServiceProxy_cff')
process.MuonServiceProxy.ServiceParameters.Propagators.append('StraightLinePropagator')

process.gemRecHits.gemDigiLabel = cms.InputTag("muonGEMDigis","","RECO")
process.gemRecHits.maskvecfile = cms.FileInPath(runConfig.GEMMask)
process.gemRecHits.deadvecfile = cms.FileInPath(runConfig.GEMHot)
process.GEMCosmicMuon = cms.EDProducer("GEMCosmicMuon",
                                       process.MuonServiceProxy,
                                       gemRecHitLabel = cms.InputTag("gemRecHits"),
                                       maxClusterSize = cms.double(runConfig.maxClusterSize),
                                       minClusterSize = cms.double(runConfig.minClusterSize),
                                       trackChi2 = cms.double(runConfig.trackChi2),
                                       trackResX = cms.double(runConfig.trackResX),
                                       trackResY = cms.double(runConfig.trackResY),
                                       MuonSmootherParameters = cms.PSet(
        PropagatorAlong = cms.string('SteppingHelixPropagatorAny'),
        PropagatorOpposite = cms.string('SteppingHelixPropagatorAny'),
        RescalingFactor = cms.double(5.0)
        ),
                                       #Reference Chambers
                                       #refChambers=cms.vint32( 1, 11, 21, 3, 13, 23, 5, 15, 25, 7, 17, 27, 9, 19, 29)
                                       refChambers=cms.vint32( 11, 20)
                                       )
process.GEMCosmicMuon.ServiceParameters.GEMLayers = cms.untracked.bool(True)

process.GEMCosmicMuon.ServiceParameters.RPCLayers = cms.bool(False)
#process.GEMCosmicMuon.ServiceParameters.UseMuonNavigation = cms.untracked.bool(False)

process.gemcrValidation = cms.EDAnalyzer('gemcrValidation',
                                         process.MuonServiceProxy,
                                         verboseSimHit = cms.untracked.int32(1),
                                         simInputLabel = cms.InputTag('g4SimHits',"MuonGEMHits"),
                                         recHitsInputLabel = cms.InputTag('gemRecHits'),
                                         tracksInputLabel = cms.InputTag('GEMCosmicMuon','','RECO'),
                                         seedInputLabel = cms.InputTag('GEMCosmicMuon','','RECO'),
                                         gemDigiLabel = cms.InputTag("muonGEMDigis","","RECO"),
                                         #genParticleLabel = cms.InputTag('genParticles','','RECO'),
                                         # st1, st2_short, st2_long of xbin, st1,st2_short,st2_long of ybin
                                         nBinGlobalZR = cms.untracked.vdouble(200,200,200,150,180,250),
                                         # st1 xmin, xmax, st2_short xmin, xmax, st2_long xmin, xmax, st1 ymin, ymax...
                                         RangeGlobalZR = cms.untracked.vdouble(564,572,786,794,786,802,110,260,170,350,100,350),
                                         #nBinGlobalXY = cms.untracked.int32(720),
                                         #detailPlot = cms.bool(True),
                                         #detailPlot = cms.bool(False),
                                         maxClusterSize = cms.double(runConfig.maxClusterSize),
                                         minClusterSize = cms.double(runConfig.minClusterSize),
                                         maxResidual = cms.double(runConfig.maxResidual),
                                         makeTrack = cms.bool(runConfig.makeTrack),
                                         isMC = cms.bool(False),
                                         trackChi2 = cms.double(runConfig.trackChi2),
                                         trackResX = cms.double(runConfig.trackResX),
                                         trackResY = cms.double(runConfig.trackResY),
                                         MuonSmootherParameters = cms.PSet(
                                                                           PropagatorAlong = cms.string('SteppingHelixPropagatorAny'),
                                                                           PropagatorOpposite = cms.string('SteppingHelixPropagatorAny'),
                                                                           RescalingFactor = cms.double(5.0)),
                                         #Reference Chambers
                                         #refChambers=cms.vint32( 1, 11, 21, 3, 13, 23, 5, 15, 25, 7, 17, 27, 9, 19, 29)
                                         refChambers=cms.vint32( 11, 20)
                                         
                                         )

# Path and EndPath definitions
process.digi_step    = cms.Path(process.muonGEMDigis)
if runConfig.makeTrack: process.reconstruction_step    = cms.Path(process.gemLocalReco+process.GEMCosmicMuon)
else : process.reconstruction_step    = cms.Path(process.gemLocalReco)
#process.reconstruction_step    = cms.Path(process.gemLocalReco+process.GEMCosmicMuon)
#process.reconstruction_step    = cms.Path(process.gemLocalReco)

process.validation_step = cms.Path(process.gemcrValidation)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGHLToutput_step = cms.EndPath(process.FEVTDEBUGHLToutput)

# Schedule definition
process.schedule = cms.Schedule(process.digi_step,
                                process.reconstruction_step,
                                process.validation_step,
                                process.endjob_step,
                                process.FEVTDEBUGHLToutput_step,
                                )
process.gemSegments.maxRecHitsInCluster = cms.int32(10)
process.gemSegments.minHitsPerSegment = cms.uint32(3)
process.gemSegments.clusterOnlySameBXRecHits = cms.bool(True)
process.gemSegments.dEtaChainBoxMax = cms.double(1.05)
process.gemSegments.dPhiChainBoxMax = cms.double(1.12)
process.gemSegments.dXclusBoxMax = cms.double(10.0)
process.gemSegments.dYclusBoxMax = cms.double(50.0)
process.gemSegments.preClustering = cms.bool(False)
process.gemSegments.preClusteringUseChaining = cms.bool(False)
