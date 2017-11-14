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
process.load('Configuration.StandardSequences.Generator_cff')
process.load('IOMC.EventVertexGenerators.VtxSmearedRealistic50ns13TeVCollision_cfi')
process.load('GeneratorInterface.Core.genFilterSummary_cff')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('Configuration.StandardSequences.Digi_cff')
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

process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c1_r1.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c1_r2.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c1_r3.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c1_r4.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c1_r5.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c2_r1.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c2_r2.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c2_r3.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c2_r4.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c2_r5.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c3_r1.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c3_r2.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c3_r3.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c3_r4.xml')
process.XMLIdealGeometryESSource.geomXMLFiles.append('Geometry/MuonCommonData/data/cosmic1/gem11L_c3_r5.xml')

#process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(20000))
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(500))

# Input source
process.source = cms.Source("EmptySource")
process.options = cms.untracked.PSet()

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
    fileName = cms.untracked.string('out_reco_MC.root'),
    #outputCommands = process.FEVTDEBUGHLTEventContent.outputCommands,
    outputCommands = cms.untracked.vstring( ('keep *')),
    splitLevel = cms.untracked.int32(0)
)

# Additional output definition
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')

## # Cosmic Muon generator
## process.load("GeneratorInterface.CosmicMuonGenerator.CMSCGENproducer_cfi")
## process.generator.MaxTheta = 84.
## process.generator.ElossScaleFactor = 0.0
## process.generator.TrackerOnly = True
## process.generator.MinP = 100

#process.genstepfilter.triggerConditions=cms.vstring("generation_step")
process.generator = cms.EDProducer("FlatRandomPtGunProducer",
    AddAntiParticle = cms.bool(True),
    PGunParameters = cms.PSet(
        MaxEta = cms.double(0.3),
        MaxPhi = cms.double(1.5707963267948966+0.3),
        MaxPt = cms.double(100.01),
        MinEta = cms.double(-0.3),
        MinPhi = cms.double(1.5707963267948966-0.3),
        MinPt = cms.double(99.99),
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

import configureRun_cfi as runConfig

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
                                       refChambers=cms.vint32(11,20)
                                       )
process.GEMCosmicMuon.ServiceParameters.GEMLayers = cms.untracked.bool(True)
process.GEMCosmicMuon.ServiceParameters.CSCLayers = cms.untracked.bool(False)
process.GEMCosmicMuon.ServiceParameters.RPCLayers = cms.bool(False)
#process.GEMCosmicMuon.ServiceParameters.UseMuonNavigation = cms.untracked.bool(False)

process.gemcrValidation = cms.EDAnalyzer('gemcrValidation',
                                         process.MuonServiceProxy,
                                         verboseSimHit = cms.untracked.int32(1),
                                         simInputLabel = cms.InputTag('g4SimHits',"MuonGEMHits"),
                                         recHitsInputLabel = cms.InputTag('gemRecHits'),
                                         tracksInputLabel = cms.InputTag('GEMCosmicMuon','','RECO'),
                                         seedInputLabel = cms.InputTag('GEMCosmicMuon','','RECO'),
                                         genParticleLabel = cms.InputTag('genParticles','','RECO'),
                                         gemDigiLabel = cms.InputTag("muonGEMDigis","","RECO"),
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
                                         isMC = cms.bool(True),
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

# Path and EndPath definitions
process.generation_step = cms.Path(process.generator+process.pgen)
process.simulation_step = cms.Path(process.psim)
#process.L1simulation_step = cms.Path(process.SimL1Emulator)
#process.digi2raw_step = cms.Path(process.DigiToRaw)
#process.raw2digi_step = cms.Path(process.RawToDigi)
#process.L1Reco_step = cms.Path(process.L1Reco)
process.digitisation_step = cms.Path(process.pdigi)
#process.reconstruction_step = cms.Path(process.reconstruction)
#process.digitisation_step = cms.Path(process.muonGEMDigi)
process.reconstruction_step    = cms.Path(process.gemLocalReco+process.GEMCosmicMuon)
#process.reconstruction_step    = cms.Path(process.gemLocalReco)
process.genfiltersummary_step = cms.EndPath(process.genFilterSummary)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGHLToutput_step = cms.EndPath(process.FEVTDEBUGHLToutput)

process.validation_step = cms.Path(process.gemcrValidation)

process.digitisation_step.remove(process.simMuonME0Digis)
process.digitisation_step.remove(process.simMuonME0ReDigis)
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
process.schedule = cms.Schedule(process.generation_step,process.genfiltersummary_step,process.simulation_step,
                                process.digitisation_step,#process.L1simulation_step,
                                #process.digi2raw_step,process.raw2digi_step,#process.L1Reco_step,
                                process.reconstruction_step,
                                process.validation_step,
                                process.endjob_step,
                                process.FEVTDEBUGHLToutput_step,
                                )
# filter all path with the production filter sequence
#for path in process.paths:
#	getattr(process,path)._seq = process.generator * getattr(process,path)._seq 

process.RandomNumberGeneratorService.generator = cms.PSet(
    initialSeed = cms.untracked.uint32(123456789),
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
process.simMuonGEMDigis.doBkgNoise = cms.bool(False)
process.simMuonGEMDigis.doNoiseCLS = cms.bool(False)
process.simMuonGEMDigis.simulateElectronBkg = cms.bool(False)


#process.SteppingHelixPropagatorAny.debug = cms.bool(True)
#process.SteppingHelixPropagatorAny.sendLogWarning = cms.bool(True)
#process.SteppingHelixPropagatorAny.useInTeslaFromMagField = cms.bool(False)
#process.SteppingHelixPropagatorAny.useMagVolumes = cms.bool(False)


