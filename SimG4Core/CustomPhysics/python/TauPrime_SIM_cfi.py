from math import isfinite

import FWCore.ParameterSet.Config as cms


def customiseTauPrime(
    process: cms.Process,
    massGeV: float,
    chargeMagnitude: float,
    validate: bool = True,
) -> cms.Process:
    if any(
        not isfinite(value) or value <= 0
        for value in (massGeV, chargeMagnitude)
    ):
        raise ValueError(
            "Tau-prime mass and charge magnitude must be finite and positive"
        )
    if not hasattr(process, "g4SimHits"):
        raise ValueError("Tau-prime customization requires the SIM step (g4SimHits)")
    physics = process.g4SimHits.Physics
    standard_type = (
        physics.TauPrimeStandardPhysicsType.value()
        if hasattr(physics, "TauPrimeStandardPhysicsType")
        else physics.type.value()
    )
    if standard_type != "SimG4Core/Physics/FTFP_BERT_EMM":
        raise ValueError(
            f"Tau-prime requires FTFP_BERT_EMM; got {standard_type}"
        )

    process.load("SimG4Core.CustomPhysics.CustomPhysics_cfi")
    setup = process.customPhysicsSetup.clone(
        particlesDef=cms.FileInPath(
            "SimG4Core/CustomPhysics/data/particles_tauprime_empty.txt"
        ),
        rhadronPhysics=cms.bool(False),
        TauPrimeMassGeV=cms.double(massGeV),
        TauPrimeChargeMagnitude=cms.double(chargeMagnitude),
        TauPrimeStandardPhysicsType=cms.string(standard_type),
    )
    process.g4SimHits.Physics = physics.clone(**setup.parameters_())
    process.g4SimHits.Physics.type = cms.string("SimG4Core/Physics/CustomPhysics")
    process.g4SimHits.Physics.ExoticaPhysicsSS = cms.untracked.bool(False)
    process.g4SimHits.Physics.DBrem = cms.untracked.bool(False)

    watchers = [
        watcher
        for watcher in process.g4SimHits.Watchers
        if watcher.type.value() != "TauPrimeValidation"
    ]
    if validate:
        watchers.append(
            cms.PSet(
                type=cms.string("TauPrimeValidation"),
                TauPrimeValidation=cms.PSet(
                    massGeV=cms.double(massGeV),
                    chargeMagnitude=cms.double(chargeMagnitude),
                ),
            )
        )
    process.g4SimHits.Watchers = cms.VPSet(*watchers)
    return process


def customise(process: cms.Process) -> cms.Process:
    return customiseTauPrime(process, massGeV=100.0, chargeMagnitude=1.0)
