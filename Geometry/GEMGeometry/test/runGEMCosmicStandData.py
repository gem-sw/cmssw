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
#process.load('SimMuon.GEMDigitizer.muonGEMDigi_cff')
process.load('RecoLocalMuon.GEMRecHit.gemLocalReco_cff')

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
      'file:CRStandTestData.root'
    ),
    skipBadFiles = cms.untracked.bool(True), 
)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('SingleMuPt100_cfi nevts:100'),
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
    fileName = cms.untracked.string('out_reco.root'),
    #outputCommands = process.FEVTDEBUGHLTEventContent.outputCommands,
    outputCommands = cms.untracked.vstring( ('keep *')),
    splitLevel = cms.untracked.int32(0)
)

# Additional output definition

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')

process.gemRecHits.gemDigiLabel = cms.InputTag("gemunpacker","","GEMR2D")

process.load('RecoMuon.TrackingTools.MuonServiceProxy_cff')
process.MuonServiceProxy.ServiceParameters.Propagators.append('StraightLinePropagator')

process.GEMCosmicMuon = cms.EDProducer("GEMCosmicMuon",
                                       process.MuonServiceProxy,
                                       gemRecHitLabel = cms.InputTag("gemRecHits"),
                                       MuonSmootherParameters = cms.PSet(
                                           PropagatorAlong = cms.string('SteppingHelixPropagatorAny'),
                                           PropagatorOpposite = cms.string('SteppingHelixPropagatorAny'),
                                           RescalingFactor = cms.double(5.0)
                                           ),
                                       )
process.GEMCosmicMuon.ServiceParameters.GEMLayers = cms.untracked.bool(True)
process.GEMCosmicMuon.ServiceParameters.CSCLayers = cms.untracked.bool(False)
process.GEMCosmicMuon.ServiceParameters.RPCLayers = cms.bool(False)
#process.GEMCosmicMuon.ServiceParameters.UseMuonNavigation = cms.untracked.bool(False)

# Path and EndPath definitions
process.reconstruction_step    = cms.Path(process.gemLocalReco+process.GEMCosmicMuon)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGHLToutput_step = cms.EndPath(process.FEVTDEBUGHLToutput)


# Schedule definition
process.schedule = cms.Schedule(process.reconstruction_step,
                                process.endjob_step,process.FEVTDEBUGHLToutput_step)

process.gemSegments.maxRecHitsInCluster = cms.int32(10)
process.gemSegments.minHitsPerSegment = cms.uint32(3)
process.gemSegments.clusterOnlySameBXRecHits = cms.bool(True)
process.gemSegments.dEtaChainBoxMax = cms.double(1.05)
process.gemSegments.dPhiChainBoxMax = cms.double(1.12)
process.gemSegments.dXclusBoxMax = cms.double(10.0)
process.gemSegments.dYclusBoxMax = cms.double(50.0)
process.gemSegments.preClustering = cms.bool(False)
process.gemSegments.preClusteringUseChaining = cms.bool(False)


process.MessageLogger.categories.append("GEMGeometryBuilderFromDDD")
process.MessageLogger.categories.append("GEMSegmentBuilder")
process.MessageLogger.categories.append("GEMSegmentAlgorithm")
process.MessageLogger.categories.append("MuonSegFit")
process.MessageLogger.categories.append("MuonTrackFinder")
process.MessageLogger.categories.append("MuonTrackLoader")
process.MessageLogger.categories.append("SteppingHelixPropagatorESProducer")
process.MessageLogger.categories.append("CosmicMuonSmoother")
process.MessageLogger.categories.append("SteppingHelixPropagator")
process.MessageLogger.debugModules = cms.untracked.vstring("*")
process.MessageLogger.destinations = cms.untracked.vstring("cout","junk")
process.MessageLogger.cout = cms.untracked.PSet(
    threshold = cms.untracked.string("DEBUG"),
    default   = cms.untracked.PSet( limit = cms.untracked.int32(0)  ),
    FwkReport = cms.untracked.PSet( limit = cms.untracked.int32(-1) ),
    MuonTrackFinder = cms.untracked.PSet( limit = cms.untracked.int32(-1)),
    CosmicMuonSmoother = cms.untracked.PSet( limit = cms.untracked.int32(-1)),
    MuonTrackLoader = cms.untracked.PSet( limit = cms.untracked.int32(-1)),
    SteppingHelixPropagatorESProducer = cms.untracked.PSet( limit = cms.untracked.int32(-1)),
    SteppingHelixPropagator = cms.untracked.PSet( limit = cms.untracked.int32(-1)),
    ## MuonSegFit = cms.untracked.PSet( limit = cms.untracked.int32(-1)),
    ## GEMGeometryBuilderFromDDD = cms.untracked.PSet( limit = cms.untracked.int32(-1) ),
    #GEMSegmentBuilder = cms.untracked.PSet( limit = cms.untracked.int32(-1)),
    ## GEMSegmentAlgorithm = cms.untracked.PSet( limit = cms.untracked.int32(-1) ),
)

#process.SteppingHelixPropagatorAny.debug = cms.bool(True)
#process.SteppingHelixPropagatorAny.sendLogWarning = cms.bool(True)
#process.SteppingHelixPropagatorAny.useInTeslaFromMagField = cms.bool(False)
#process.SteppingHelixPropagatorAny.useMagVolumes = cms.bool(False)
