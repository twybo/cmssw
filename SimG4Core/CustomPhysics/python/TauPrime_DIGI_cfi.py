from math import isfinite

import FWCore.ParameterSet.Config as cms


def customiseTauPrimeDigi(process: cms.Process, massGeV: float, chargeMagnitude: float,
                         validate: bool = True) -> cms.Process:
    charge_squared = chargeMagnitude * chargeMagnitude
    if any(not isfinite(value) or value <= 0
           for value in (massGeV, chargeMagnitude, charge_squared)):
        raise ValueError("Tau-prime DIGI mass, charge and squared charge must be finite and positive")
    labels = ("simMuonCSCDigis", "simMuonDTDigis", "simMuonRPCDigis")
    if any(not hasattr(process, label) for label in labels):
        raise ValueError("Tau-prime DIGI customization requires CSC, DT and RPC digitizers")
    if process.simMuonRPCDigis.digiModel.value() != "RPCSimAsymmetricCls":
        raise ValueError("Tau-prime DIGI supports the Run-3 RPCSimAsymmetricCls model")
    if process.simMuonDTDigis.onlyMuHits.value():
        raise ValueError("Tau-prime DIGI requires simMuonDTDigis.onlyMuHits=False")
    if hasattr(process, "g4SimHits"):
        physics = process.g4SimHits.Physics
        if (not hasattr(physics, "TauPrimeMassGeV") or
                not hasattr(physics, "TauPrimeChargeMagnitude") or
                physics.TauPrimeMassGeV.value() != massGeV or
                physics.TauPrimeChargeMagnitude.value() != chargeMagnitude):
            raise ValueError("Tau-prime SIM and DIGI mass and charge must agree")
    for parameters in (process.simMuonCSCDigis, process.simMuonDTDigis,
                       process.simMuonRPCDigis.digiModelConfig):
        parameters.TauPrimeMassGeV = cms.double(massGeV)
        parameters.TauPrimeChargeMagnitude = cms.double(chargeMagnitude)
        parameters.TauPrimeDigiValidation = cms.bool(validate)
    return process


def customise(process: cms.Process) -> cms.Process:
    return customiseTauPrimeDigi(process, massGeV=100.0, chargeMagnitude=1.0)
