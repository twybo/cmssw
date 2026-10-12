from math import isfinite

import FWCore.ParameterSet.Config as cms


def customiseTauPrime(process: cms.Process, massGeV: float, chargeMagnitude: float,
                     mHatMinGeV: float = 200.0, validate: bool = True) -> cms.Process:
    if any(not isfinite(value) or value <= 0
           for value in (massGeV, chargeMagnitude, mHatMinGeV)):
        raise ValueError("Tau-prime mass, charge magnitude and mHatMin must be finite and positive")
    if not hasattr(process, "g4SimHits"):
        raise ValueError("Tau-prime customization requires the SIM step (g4SimHits)")
    physics = process.g4SimHits.Physics
    standard_type = (physics.TauPrimeStandardPhysicsType.value()
                     if hasattr(physics, "TauPrimeStandardPhysicsType") else physics.type.value())
    if standard_type not in {"SimG4Core/Physics/FTFP_BERT_EMH", "SimG4Core/Physics/FTFP_BERT_EMM"}:
        raise ValueError(f"Unsupported ordinary-particle physics list: {standard_type}")

    if hasattr(process, "generator"):
        parameters = process.generator.PythiaParameters.processParameters
        keys = {"17:m0", "17:chargeType", "17:mayDecay", "PhaseSpace:mHatMin"}
        retained = [line for line in parameters if line.split("=", 1)[0].strip() not in keys]
        process.generator.PythiaParameters.processParameters = cms.vstring(
            *retained,
            f"17:m0 = {massGeV:.17g}",
            "17:chargeType = -3",
            "17:mayDecay = off",
            f"PhaseSpace:mHatMin = {mHatMinGeV:.17g}",
        )

    process.load("SimG4Core.CustomPhysics.CustomPhysics_cfi")
    setup = process.customPhysicsSetup.clone(
        particlesDef=cms.FileInPath("SimG4Core/CustomPhysics/data/particles_tauprime_empty.txt"),
        rhadronPhysics=cms.bool(False),
        TauPrimeMassGeV=cms.double(massGeV),
        TauPrimeChargeMagnitude=cms.double(chargeMagnitude),
        TauPrimeStandardPhysicsType=cms.string(standard_type),
    )
    process.g4SimHits.Physics = physics.clone(**setup.parameters_())
    process.g4SimHits.Physics.type = cms.string("SimG4Core/Physics/CustomPhysics")
    process.g4SimHits.Physics.ExoticaPhysicsSS = cms.untracked.bool(False)
    process.g4SimHits.Physics.DBrem = cms.untracked.bool(False)

    watchers = [watcher for watcher in process.g4SimHits.Watchers
                if watcher.type.value() != "TauPrimeValidation"]
    if validate:
        watchers.append(cms.PSet(
            type=cms.string("TauPrimeValidation"),
            TauPrimeValidation=cms.PSet(
                massGeV=cms.double(massGeV),
                chargeMagnitude=cms.double(chargeMagnitude),
            ),
        ))
    process.g4SimHits.Watchers = cms.VPSet(*watchers)
    return process


def customise(process: cms.Process) -> cms.Process:
    return customiseTauPrime(process, massGeV=100.0, chargeMagnitude=1.0)
