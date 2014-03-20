import FWCore.ParameterSet.Config as cms

def customise(process):
    if hasattr(process,'digitisation_step'):
        process=customise_Digi(process)
    if hasattr(process,'L1simulation_step'):
       process=customise_L1Emulator(process)
    if hasattr(process,'DigiToRaw'):
        process=customise_DigiToRaw(process)
    if hasattr(process,'RawToDigi'):
        process=customise_RawToDigi(process)
    if hasattr(process,'reconstruction'):
        process=customise_Reco(process)
    if hasattr(process,'dqmoffline_step'):
        process=customise_DQM(process)
    if hasattr(process,'dqmHarvesting'):
        process=customise_harvesting(process)
    if hasattr(process,'validation_step'):
        process=customise_Validation(process)

    return process

def customise_Digi(process):
    process.RandomNumberGeneratorService.simMuonGEMDigis = cms.PSet(
        initialSeed = cms.untracked.uint32(1234567),
        engineName = cms.untracked.string('HepJamesRandom')
    )

    process.mix.mixObjects.mixSH.crossingFrames.append('MuonGEMHits')
    process.mix.mixObjects.mixSH.input.append(cms.InputTag("g4SimHits","MuonGEMHits"))
    process.mix.mixObjects.mixSH.subdets.append('MuonGEMHits')

    process.load('SimMuon.GEMDigitizer.muonGEMDigi_cff')
    process.muonDigi += process.muonGEMDigi
    process=outputCustoms(process)
    return process

def customise_L1Emulator(process, ptdphi):
    process.simCscTriggerPrimitiveDigis.gemPadProducer =  cms.untracked.InputTag("simMuonGEMCSCPadDigis","")
    process.simCscTriggerPrimitiveDigis.clctSLHC.clctPidThreshPretrig = 2
    process.simCscTriggerPrimitiveDigis.clctParam07.clctPidThreshPretrig = 2

    tmb = process.simCscTriggerPrimitiveDigis.tmbSLHC
    tmb.me11ILT = cms.untracked.PSet(
        runUpgradeME11 = cms.untracked.bool(False)
    )
    tmb.me21ILT = cms.untracked.PSet(
        runUpgradeME21 = cms.untracked.bool(False),
        printAvailablePads = cms.untracked.bool(False),
        gemMatchDeltaEta = cms.untracked.double(0.08),
        gemMatchDeltaBX = cms.untracked.int32(1),
        maxDeltaBXPad = cms.untracked.int32(0),
        maxDeltaRollPad = cms.untracked.int32(0),
        maxDeltaPadPad = cms.untracked.int32(1),
        maxDeltaBXCoPad = cms.untracked.int32(0),
        maxDeltaRollCoPad = cms.untracked.int32(0),
        maxDeltaPadCoPad = cms.untracked.int32(1),
        useOldLCTDataFormatALCTGEM = cms.untracked.bool(True),
        dropLowQualityCLCTsNoGEMs = cms.untracked.bool(False),
        buildLCTfromALCTandGEM = cms.untracked.bool(False),
        doLCTGhostBustingWithGEMs = cms.untracked.bool(False),
    )
            
    """
    tmb.me11IntegratedLocalTrigger.PSet(
        maxDeltaBXInCoPad = cms.untracked.int32(1),
        maxDeltaRollInCoPad = cms.untracked.int32(0),
        maxDeltaPadInCoPad = cms.untracked.int32(0),
        useOldLCTDataFormatALCTGEM = cms.untracked.bool(True),
        
        ## GE1/1-ME1/1
        runGEMCSCILT = cms.untracked.bool(True),
        dphi_lct_pad98 = {
        'pt0'  : { 'odd' :  2.00000000 , 'even' :  2.00000000 },
        'pt05' : { 'odd' :  0.02203510 , 'even' :  0.00930056 },
        'pt06' : { 'odd' :  0.01825790 , 'even' :  0.00790009 },
        'pt10' : { 'odd' :  0.01066000 , 'even' :  0.00483286 },
        'pt15' : { 'odd' :  0.00722795 , 'even' :  0.00363230 },
        'pt20' : { 'odd' :  0.00562598 , 'even' :  0.00304879 },
        'pt30' : { 'odd' :  0.00416544 , 'even' :  0.00253782 },
        'pt40' : { 'odd' :  0.00342827 , 'even' :  0.00230833 }
        },
        
        printAvailablePads = cms.untracked.bool(False),
        dropLowQualityCLCTsNoGEMs_ME1a = cms.untracked.bool(False),
        dropLowQualityCLCTsNoGEMs_ME1b = cms.untracked.bool(False),
        buildLCTfromALCTandGEM_ME1a = cms.untracked.bool(False),
        buildLCTfromALCTandGEM_ME1b = cms.untracked.bool(False),
        doLCTGhostBustingWithGEMs = cms.untracked.bool(False),
        gemMatchDeltaEta = cms.untracked.double(0.08),
        gemMatchDeltaBX = cms.untracked.int32(1),
        maxDeltaBXPad = cms.untracked.int32(0),
        maxDeltaRollPad = cms.untracked.int32(0),
        maxDeltaPadPad = cms.untracked.int32(0),
        maxDeltaBXCoPad = cms.untracked.int32(0),
        maxDeltaRollCoPad = cms.untracked.int32(0),
        maxDeltaPadCoPad = cms.untracked.int32(0),
        gemMatchDeltaPhiOdd = cms.untracked.double(dphi_lct_pad98[ptdphi]['odd']),
        gemMatchDeltaPhiEven = cms.untracked.double(dphi_lct_pad98[ptdphi]['even']),
        gemClearNomatchLCTs = cms.untracked.bool(ptdphi == 'pt0' and False),
    )
    """
    ## Common
    tmb.maxDeltaBXInCoPad = cms.untracked.int32(1)
    tmb.maxDeltaRollInCoPad = cms.untracked.int32(0)
    tmb.maxDeltaPadInCoPad = cms.untracked.int32(0)
    tmb.useOldLCTDataFormatALCTGEM = cms.untracked.bool(True)
    
    ## GE1/1-ME1/1
    tmb.runGEMCSCILT = cms.untracked.bool(True)
    dphi_lct_pad98 = {
        'pt0'  : { 'odd' :  2.00000000 , 'even' :  2.00000000 },
        'pt05' : { 'odd' :  0.02203510 , 'even' :  0.00930056 },
        'pt06' : { 'odd' :  0.01825790 , 'even' :  0.00790009 },
        'pt10' : { 'odd' :  0.01066000 , 'even' :  0.00483286 },
        'pt15' : { 'odd' :  0.00722795 , 'even' :  0.00363230 },
        'pt20' : { 'odd' :  0.00562598 , 'even' :  0.00304879 },
        'pt30' : { 'odd' :  0.00416544 , 'even' :  0.00253782 },
        'pt40' : { 'odd' :  0.00342827 , 'even' :  0.00230833 }
    }

    tmb.printAvailablePads = cms.untracked.bool(False)
    tmb.dropLowQualityCLCTsNoGEMs_ME1a = cms.untracked.bool(False)
    tmb.dropLowQualityCLCTsNoGEMs_ME1b = cms.untracked.bool(False)
    tmb.buildLCTfromALCTandGEM_ME1a = cms.untracked.bool(False)
    tmb.buildLCTfromALCTandGEM_ME1b = cms.untracked.bool(False) 
    tmb.doLCTGhostBustingWithGEMs = cms.untracked.bool(False)
    tmb.gemMatchDeltaEta = cms.untracked.double(0.08)
    tmb.gemMatchDeltaBX = cms.untracked.int32(1)
    tmb.maxDeltaBXPad = cms.untracked.int32(0)
    tmb.maxDeltaRollPad = cms.untracked.int32(0)
    tmb.maxDeltaPadPad = cms.untracked.int32(1)
    tmb.maxDeltaBXCoPad = cms.untracked.int32(0)
    tmb.maxDeltaRollCoPad = cms.untracked.int32(0)
    tmb.maxDeltaPadCoPad = cms.untracked.int32(1)
    tmb.gemMatchDeltaPhiOdd = cms.untracked.double(dphi_lct_pad98[ptdphi]['odd'])
    tmb.gemMatchDeltaPhiEven = cms.untracked.double(dphi_lct_pad98[ptdphi]['even'])
    if ptdphi == 'pt0':
        tmb.gemClearNomatchLCTs = cms.untracked.bool(False)
    ## GE2/1-ME2/1
    tmb.runUpgradeME21 = cms.untracked.bool(False)
    tmb.dropLowQualityCLCTsNoGEMs_ME21 = cms.untracked.bool(False)
    tmb.buildLCTfromALCTandGEM_ME21 = cms.untracked.bool(False)
    return process

def customise_DigiToRaw(process):
    return process

def customise_RawToDigi(process):
    return process

def customise_Reco(process):
    process.load('RecoLocalMuon.GEMRecHit.gemRecHits_cfi')
    process.muonlocalreco += process.gemRecHits
    process.standAloneMuons.STATrajBuilderParameters.EnableGEMMeasurement = cms.bool(True)
    process.standAloneMuons.STATrajBuilderParameters.BWFilterParameters.EnableGEMMeasurement = cms.bool(True)
    process=outputCustoms(process)
    return process

def customise_DQM(process):
    return process

def customise_Validation(process):
    process.load('Validation.Configuration.gemSimValid_cff')
    process.genvalid_all += process.gemSimValid

    process.load('Validation.RecoMuon.MuonTrackValidator_cfi')
    process.load('SimMuon.MCTruth.MuonAssociatorByHits_cfi')
    process.muonAssociatorByHitsCommonParameters.useGEMs = cms.bool(True)
    process.muonTrackValidator.useGEMs = cms.bool(True)
    return process


def customise_harvesting(process):
    process.load('Validation.Configuration.gemPostValidation_cff')
    process.postValidation += process.gemPostValidation
    return process

def outputCustoms(process):
    alist=['AODSIM','RECOSIM','FEVTSIM','FEVTDEBUG','FEVTDEBUGHLT','RECODEBUG','RAWRECOSIMHLT','RAWRECODEBUGHLT']
    for a in alist:
        b=a+'output'
        if hasattr(process,b):
            getattr(process,b).outputCommands.append('keep *_simMuonGEMDigis_*_*')
            getattr(process,b).outputCommands.append('keep *_simMuonGEMCSCPadDigis_*_*')
            getattr(process,b).outputCommands.append('keep *_gemRecHits_*_*')

    return process
