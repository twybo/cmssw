"""Shared Run-3 tau-prime GEN/SIM/DIGI configuration."""

from math import isfinite

import FWCore.ParameterSet.Config as cms


def customise(
    process: cms.Process,
    mass: float = 1000.0,
    charge: float = 1.0,
    massMode: str = "fixed",
    counting: str = "clusters",
) -> cms.Process:
    from SimG4Core.CustomPhysics.TauPrime_SIM_cfi import customiseTauPrime
    from SimMuon.Configuration.customiseTauPrimeDigi import customiseTauPrimeDigi

    if massMode not in {"fixed", "sampled"} or counting not in {
        "clusters",
        "electrons",
    }:
        raise ValueError("Unsupported tau-prime mode")
    if any(not isfinite(x) or x <= 0 for x in (mass, charge)):
        raise ValueError("Mass and charge magnitude must be finite and positive")
    process = customiseTauPrime(process, mass, charge, validate=True)
    keys = {
        "17:m0",
        "17:chargeType",
        "17:mayDecay",
        "PhaseSpace:mHatMin",
        "17:mWidth",
        "17:isResonance",
        "17:doForceWidth",
    }
    parameters = [
        line
        for line in process.generator.PythiaParameters.processParameters
        if line.split("=", 1)[0].strip() not in keys
    ]
    parameters.extend(
        (
            f"17:m0 = {mass:.17g}",
            "17:chargeType = -3",
            "17:mayDecay = off",
            f"PhaseSpace:mHatMin = {max(50.0, mass):.17g}",
        )
    )
    if massMode == "fixed":
        parameters.extend(
            ("17:mWidth = 0", "17:isResonance = on", "17:doForceWidth = on")
        )
    process.generator.PythiaParameters.processParameters = cms.vstring(*parameters)
    return customiseTauPrimeDigi(
        process,
        mass,
        charge,
        validate=True,
        dtCountingMode=counting,
        rpcCountingMode=counting,
        exoticSamplingLimit=400,
    )
