# Auto generated configuration file
# using: 
# Revision: 1.19 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: SingleMuPt100_cfi -s GEN,SIM,DIGI,L1,DIGI2RAW,RAW2DIGI,L1Reco,RECO --conditions auto:run2_mc --magField 38T_PostLS1 --datatier GEN-SIM --geometry GEMCosmicStand --eventcontent FEVTDEBUGHLT --era phase2_muon -n 100 --fileout out_reco.root
import datetime
print datetime.datetime.now()
import FWCore.ParameterSet.Config as cms
import configureRun_cfi as runConfig

# options
import FWCore.ParameterSet.VarParsing as VarParsing
options = VarParsing.VarParsing('analysis')

options.register("runNum",1,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Run number")
                 
options.register("eventsPerJob",10000,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "The number of events (in each file)")
                 
options.register("idxJob","-1",
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "The index of this root file")

options.parseArguments()

# The superchambers in the 15 slots
SuperChType = runConfig.StandConfiguration

print(SuperChType)

# Calculation of SuperChSeedingLayers from SuperChType
SuperChSeedingLayers = []

for i in range (0,30):
	SuperChSeedingLayers.append(0)

for j in range (0,3):
	for i in range (5*j,5*(j+1)):
		if (SuperChType[i]!='0'):
			SuperChSeedingLayers[i*2]=1
			SuperChSeedingLayers[i*2+1]=3
			break
	for i in range (5*(j+1)-1,5*j-1,-1):
		if (SuperChType[i]!='0'):
			SuperChSeedingLayers[i*2]=4
			SuperChSeedingLayers[i*2+1]=2
			break
			
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
process.load('Configuration.StandardSequences.Generator_cff')
process.load('IOMC.EventVertexGenerators.VtxSmearedRealistic50ns13TeVCollision_cfi')
process.load('GeneratorInterface.Core.genFilterSummary_cff')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('Configuration.StandardSequences.Digi_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('EventFilter.GEMRawToDigi.gemPacker_cfi')
process.load('EventFilter.RawDataCollector.rawDataCollector_cfi')
process.load('EventFilter.GEMRawToDigi.muonGEMDigis_cfi')
process.load('SimMuon.GEMCosmicMuon.muonGEMDigi_cff')
process.load('RecoLocalMuon.GEMRecHit.gemLocalReco_cff')

# DEFINITION OF THE SUPERCHAMBERS INSIDE THE STAND
for i in xrange(len(SuperChType)):
    column_row = '_c%d_r%d' % ((i/5)+1, i%5+1)
    if SuperChType[i]=='L' : size = 'L'
    if SuperChType[i]=='S' : size = 'S'
    if SuperChType[i]!='0' : geomFile = 'Geometry/MuonCommonData/data/GEMQC8/gem11'+size+column_row+'.xml'
    print(geomFile)
    if SuperChType[i]!='0' : process.XMLIdealGeometryESSource.geomXMLFiles.append(geomFile)
    print('-> Appended')

# Config importation & settings
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.eventsPerJob))
nIdxJob = int(options.idxJob)
strOutput = "out_reco_MC.root" if nIdxJob >= 0 else runConfig.OutputFileName
if nIdxJob < 0: nIdxJob = 0

# Input source
process.source = cms.Source("EmptySource", 
    firstRun = cms.untracked.uint32(options.runNum), 
    firstEvent = cms.untracked.uint32(options.eventsPerJob * nIdxJob + 1), 
    firstLuminosityBlock = cms.untracked.uint32(nIdxJob + 1), 
)
process.options = cms.untracked.PSet()

process.MessageLogger.cerr.FwkReport.reportEvery = 1000

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('CosmicMuonGenerator nevts:100'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Output definition
process.FEVTDEBUGHLToutput = cms.OutputModule("PoolOutputModule",
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('generation_step')
    ),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('GEN-SIM'),
        filterName = cms.untracked.string('')
    ),
    eventAutoFlushCompressedSize = cms.untracked.int32(10485760),
    fileName = cms.untracked.string('file:'+strOutput),
    outputCommands = cms.untracked.vstring( ('drop *')),
    splitLevel = cms.untracked.int32(0)
)

# Additional output definition
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')

# Cosmic Muon generator
process.generator = cms.EDProducer("CosmicGun",
    AddAntiParticle = cms.bool(True),
    PGunParameters = cms.PSet(
        MinPt = cms.double(99.99),
        MaxPt = cms.double(100.01),
        MinPhi = cms.double(3.141592),
        MaxPhi = cms.double(-3.141592),
        MinTheta = cms.double(1.570796),
        MaxTheta = cms.double(3.141592),
        IsThetaFlat = cms.bool(False), # If 'True': theta distribution is flat. If 'False': theta distribution is a cos^2
        PartID = cms.vint32(-13)
    ),
    Verbosity = cms.untracked.int32(0),
    firstRun = cms.untracked.uint32(1),
    psethack = cms.string('single mu pt 100')
)

process.mix = cms.EDProducer("MixingModule",
    LabelPlayback = cms.string(''),
    bunchspace = cms.int32(450),
    maxBunch = cms.int32(3),
    minBunch = cms.int32(-5),
    mixProdStep1 = cms.bool(False),
    mixProdStep2 = cms.bool(False),
    playback = cms.untracked.bool(False),
    useCurrentProcessOnly = cms.bool(False),
    digitizers = cms.PSet(),
    
    mixObjects = cms.PSet(
        mixSH = cms.PSet(
            crossingFrames = cms.untracked.vstring('MuonGEMHits'),
            input = cms.VInputTag(cms.InputTag("g4SimHits","MuonGEMHits")),
            type = cms.string('PSimHit'),
            subdets = cms.vstring('MuonGEMHits'),            
            )
        ),
    mixTracks = cms.PSet(
        input = cms.VInputTag(cms.InputTag("g4SimHits")),
        makeCrossingFrame = cms.untracked.bool(True),
        type = cms.string('SimTrack')
    ),
)

process.g4SimHits.UseMagneticField = cms.bool(False)
process.simCastorDigis = cms.EDAlias()
process.simEcalUnsuppressedDigis = cms.EDAlias()
process.simHcalUnsuppressedDigis = cms.EDAlias()
process.simSiPixelDigis = cms.EDAlias()
process.simSiStripDigis = cms.EDAlias()

process.load('RecoMuon.TrackingTools.MuonServiceProxy_cff')
process.MuonServiceProxy.ServiceParameters.Propagators.append('StraightLinePropagator')

process.GEMCosmicMuonForQC8 = cms.EDProducer("GEMCosmicMuonForQC8",
                                       process.MuonServiceProxy,
                                       gemRecHitLabel = cms.InputTag("gemRecHits"),
                                       maxClusterSize = cms.double(runConfig.maxClusterSize),
                                       minClusterSize = cms.double(runConfig.minClusterSize),
                                       trackChi2 = cms.double(runConfig.trackChi2),
                                       trackResX = cms.double(runConfig.trackResX),
                                       trackResY = cms.double(runConfig.trackResY),
                                       MulSigmaOnWindow = cms.double(runConfig.MulSigmaOnWindow),
                                       SuperChamberType = cms.vstring(SuperChType),
                                       SuperChamberSeedingLayers = cms.vdouble(SuperChSeedingLayers),
                                       MuonSmootherParameters = cms.PSet(
                                           PropagatorAlong = cms.string('SteppingHelixPropagatorAny'),
                                           PropagatorOpposite = cms.string('SteppingHelixPropagatorAny'),
                                           RescalingFactor = cms.double(5.0)
                                           ),
                                       )
process.GEMCosmicMuonForQC8.ServiceParameters.GEMLayers = cms.untracked.bool(True)
process.GEMCosmicMuonForQC8.ServiceParameters.CSCLayers = cms.untracked.bool(False)
process.GEMCosmicMuonForQC8.ServiceParameters.RPCLayers = cms.bool(False)

fScale = 1.0

process.gemcrValidation = cms.EDProducer('gemcrValidation',
    process.MuonServiceProxy,
    verboseSimHit = cms.untracked.int32(1),
    simInputLabel = cms.InputTag('g4SimHits',"MuonGEMHits"),
    genVtx = cms.InputTag("generator","unsmeared", "RECO"),
    recHitsInputLabel = cms.InputTag('gemRecHits'),
    tracksInputLabel = cms.InputTag('GEMCosmicMuonForQC8','','RECO'),
    seedInputLabel = cms.InputTag('GEMCosmicMuonForQC8','','RECO'),
    trajInputLabel = cms.InputTag('GEMCosmicMuonForQC8','','RECO'),
    chNoInputLabel = cms.InputTag('GEMCosmicMuonForQC8','','RECO'),
    seedTypeInputLabel = cms.InputTag('GEMCosmicMuonForQC8','','RECO'),
    genParticleLabel = cms.InputTag('genParticles','','RECO'),
    gemDigiLabel = cms.InputTag("muonGEMDigis","","RECO"),
    nBinGlobalZR = cms.untracked.vdouble(200,200,200,150,180,250),
    RangeGlobalZR = cms.untracked.vdouble(564,572,786,794,786,802,110,260,170,350,100,350),
    maxClusterSize = cms.double(runConfig.maxClusterSize),
    minClusterSize = cms.double(runConfig.minClusterSize),
    maxResidual = cms.double(runConfig.maxResidual),
    isMC = cms.bool(True),
    SuperChamberType = cms.vstring(SuperChType),
    SuperChamberSeedingLayers = cms.vdouble(SuperChSeedingLayers),
    MuonSmootherParameters = cms.PSet(
                      PropagatorAlong = cms.string('SteppingHelixPropagatorAny'),
                      PropagatorOpposite = cms.string('SteppingHelixPropagatorAny'),
                      RescalingFactor = cms.double(5.0)
                      ),
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string('temp_'+strOutput)
)

process.rawDataCollector.RawCollectionList = cms.VInputTag(cms.InputTag("gemPacker"))
# Path and EndPath definitions
process.generation_step = cms.Path(process.generator+process.pgen)
process.simulation_step = cms.Path(process.psim)
process.digitisation_step = cms.Path(process.mix+process.simMuonGEMDigis)
process.reconstruction_step = cms.Path(process.gemPacker+process.rawDataCollector+process.muonGEMDigis+process.gemLocalReco+process.GEMCosmicMuonForQC8)
process.genfiltersummary_step = cms.EndPath(process.genFilterSummary)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGHLToutput_step = cms.EndPath(process.FEVTDEBUGHLToutput)
process.validation_step = cms.Path(process.gemcrValidation)
process.digitisation_step.remove(process.simEcalTriggerPrimitiveDigis)
process.digitisation_step.remove(process.simEcalDigis)
process.digitisation_step.remove(process.simEcalPreshowerDigis)
process.digitisation_step.remove(process.simHcalTriggerPrimitiveDigis)
process.digitisation_step.remove(process.simHcalDigis)
process.digitisation_step.remove(process.simHcalTTPDigis)
process.digitisation_step.remove(process.simMuonCSCDigis)
process.digitisation_step.remove(process.simMuonRPCDigis)
process.digitisation_step.remove(process.addPileupInfo)
process.digitisation_step.remove(process.simMuonDTDigis)

# Schedule definition
process.schedule = cms.Schedule(process.generation_step,
								process.genfiltersummary_step,
								process.simulation_step,
                                process.digitisation_step,
                                process.reconstruction_step,
                                process.validation_step,
                                process.endjob_step,
                                process.FEVTDEBUGHLToutput_step,
                                )

process.RandomNumberGeneratorService.generator = cms.PSet(
    initialSeed = cms.untracked.uint32( ( nIdxJob + 1 ) + options.runNum*10000),
    engineName = cms.untracked.string('HepJamesRandom')
)
process.RandomNumberGeneratorService.simMuonGEMDigis = process.RandomNumberGeneratorService.generator
process.RandomNumberGeneratorService.VtxSmeared = process.RandomNumberGeneratorService.generator
process.RandomNumberGeneratorService.g4SimHits = process.RandomNumberGeneratorService.generator

process.gemSegments.maxRecHitsInCluster = cms.int32(10)
process.gemSegments.minHitsPerSegment = cms.uint32(3)
process.gemSegments.clusterOnlySameBXRecHits = cms.bool(True)
process.gemSegments.dEtaChainBoxMax = cms.double(1.05)
process.gemSegments.dPhiChainBoxMax = cms.double(1.12)
process.gemSegments.dXclusBoxMax = cms.double(10.0)
process.gemSegments.dYclusBoxMax = cms.double(50.0)
process.gemSegments.preClustering = cms.bool(False)
process.gemSegments.preClusteringUseChaining = cms.bool(False)

process.simMuonGEMDigis.averageEfficiency = cms.double(0.98)
process.simMuonGEMDigis.averageNoiseRate = cms.double(0.0)
process.simMuonGEMDigis.simulateIntrinsicNoise = cms.bool(False)
process.simMuonGEMDigis.doBkgNoise = cms.bool(False)
process.simMuonGEMDigis.doNoiseCLS = cms.bool(False)
process.simMuonGEMDigis.simulateElectronBkg = cms.bool(False)
