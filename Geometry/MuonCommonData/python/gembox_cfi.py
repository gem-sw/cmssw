import FWCore.ParameterSet.Config as cms

XMLIdealGeometryESSource = cms.ESSource("XMLIdealGeometryESSource",
    geomXMLFiles = cms.vstring('Geometry/CMSCommonData/data/materials.xml',
        'Geometry/CMSCommonData/data/rotations.xml',
        'Geometry/MuonCommonData/data/cosmic1/cms.xml',
        #'Geometry/MuonCommonData/data/cosmic1/muonBase.xml', # Phase-2 Muon
        #'Geometry/MuonCommonData/data/cosmic1/cmsMuon.xml',        
        #'Geometry/MuonCommonData/data/cosmic1/mf.xml',        
        'Geometry/MuonCommonData/data/PhaseII/gemf.xml',
        'Geometry/MuonCommonData/data/cosmic1/gem11.xml',
        'Geometry/MuonCommonData/data/cosmic1/muonNumbering.xml',
        'Geometry/MuonCommonData/data/cosmic1/muonSens.xml',
        'Geometry/MuonCommonData/data/cosmic1/muonProdCuts.xml',
        'Geometry/MuonCommonData/data/cosmic1/GEMSpecsFilter.xml',   # Phase-2 Muon
        'Geometry/MuonCommonData/data/cosmic1/GEMSpecs.xml',
        'Geometry/MuonCommonData/data/cosmic1/gembox.xml',
        ## these are added on the fly
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c1_r1.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c1_r2.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c1_r3.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c1_r4.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c1_r5.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c2_r1.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c2_r2.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c2_r3.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c2_r4.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c2_r5.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c3_r1.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c3_r2.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c3_r3.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c3_r4.xml',
        ## 'Geometry/MuonCommonData/data/cosmic1/gem11L_c3_r5.xml',

        ),
    rootNodeName = cms.string('cms:OCMS')
)

