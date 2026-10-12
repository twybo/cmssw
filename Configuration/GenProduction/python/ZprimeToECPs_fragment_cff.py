import FWCore.ParameterSet.Config as cms

from Configuration.Generator.Pythia8CommonSettings_cfi import pythia8CommonSettingsBlock
from Configuration.Generator.MCTunesRun3ECM13p6TeV.PythiaCP5Settings_cfi import pythia8CP5SettingsBlock

generator = cms.EDFilter(
    "Pythia8ConcurrentGeneratorFilter",
    pythiaPylistVerbosity=cms.untracked.int32(0),
    pythiaHepMCVerbosity=cms.untracked.bool(False),
    maxEventsToPrint=cms.untracked.int32(0),
    addHepMCProduct=cms.bool(True),
    comEnergy=cms.double(13600.0),
    PythiaParameters=cms.PSet(
        pythia8CommonSettingsBlock,
        pythia8CP5SettingsBlock,
        processParameters=cms.vstring(
            "NewGaugeBoson:ffbar2gmZZprime = on",
            "Zprime:gmZmode = 5",
            "32:m0 = 91.2",
            "32:onMode = off",
            "32:onIfAny = 17",
            "Zprime:coup2gen4 = on",
            "Zprime:universality = off",
            "Zprime:vtauPrime = 0.92",
            "Zprime:atauPrime = 0",
            "17:m0 = 100.0",
            "17:chargeType = -3",
            "17:mayDecay = off",
            "PhaseSpace:mHatMin = 200.0",
        ),
        parameterSets=cms.vstring("pythia8CommonSettings", "pythia8CP5Settings", "processParameters"),
    ),
)

ecpFilter = cms.EDFilter(
    "MCSingleParticleFilter",
    ParticleID=cms.untracked.vint32(17, -17),
    MinPt=cms.untracked.vdouble(0.0, 0.0),
    MinEta=cms.untracked.vdouble(-2.6, -2.6),
    MaxEta=cms.untracked.vdouble(2.6, 2.6),
    Status=cms.untracked.vint32(1, 1),
)

ProductionFilterSequence = cms.Sequence(generator * ecpFilter)
