"""Shared Run-3 tau-prime GEN/SIM/DIGI configuration."""
from math import isfinite

import FWCore.ParameterSet.Config as cms


def customise(process: cms.Process, mass: float = 1000.0, charge: float = 1.0,
              massMode: str = "fixed", counting: str = "clusters") -> cms.Process:
    from SimG4Core.CustomPhysics.TauPrime_SIM_cfi import customiseTauPrime
    from SimG4Core.CustomPhysics.TauPrime_DIGI_cfi import customiseTauPrimeDigi

    if massMode not in {"fixed", "sampled"} or counting not in {"clusters", "electrons"}:
        raise ValueError("Unsupported tau-prime mode")
    if any(not isfinite(x) or x <= 0 for x in (mass, charge)):
        raise ValueError("Mass and charge magnitude must be finite and positive")
    process = customiseTauPrime(process, mass, charge, mHatMinGeV=max(50.0, mass), validate=True)
    keys = {"17:mWidth", "17:isResonance", "17:doForceWidth"}
    parameters = [line for line in process.generator.PythiaParameters.processParameters
                  if line.split("=", 1)[0].strip() not in keys]
    if massMode == "fixed":
        parameters.extend(("17:mWidth = 0", "17:isResonance = on", "17:doForceWidth = on"))
    process.generator.PythiaParameters.processParameters = cms.vstring(*parameters)
    return customiseTauPrimeDigi(process, mass, charge, validate=True,
                                dtCountingMode=counting, rpcCountingMode=counting,
                                exoticSamplingLimit=400)


def make_process(massMode: str = "fixed", counting: str = "clusters", *,
                 mass: float = 1000.0, charge: float = 1.0, maxEvents: int = 10,
                 seed: int = 12345, outputFile: str = "output.root") -> cms.Process:
    import runpy
    import subprocess
    import tempfile
    from pathlib import Path
    from FWCore.ParameterSet.VarParsing import VarParsing

    options = VarParsing("analysis")
    options.register("mass", mass, options.multiplicity.singleton, options.varType.float, "Nominal mass in GeV")
    options.register("charge", charge, options.multiplicity.singleton, options.varType.float, "Transport charge magnitude in e")
    options.register("seed", seed, options.multiplicity.singleton, options.varType.int, "Base random seed")
    options.maxEvents = maxEvents
    options.outputFile = outputFile
    options.parseArguments()
    if options.maxEvents <= 0 or not 1 <= options.seed <= 900000000 - 1000:
        raise ValueError("Positive event count and base seed in [1, 899999000] required")
    with tempfile.TemporaryDirectory(prefix="tauprime-config-") as directory:
        config = str(Path(directory) / "generated.py")
        subprocess.run([
            "cmsDriver.py", "Configuration/GenProduction/python/ZprimeToECPs_fragment_cff.py",
            "--mc", "--conditions", "150X_mcRun3_2024_realistic_v3", "--era", "Run3_2024",
            "--geometry", "DB:Extended", "--step", "GEN,SIM,DIGI:pdigi_valid",
            "--beamspot", "DBrealistic", "--pileup", "NoPileUp", "--nThreads", "1", "--nStreams", "1",
            "--eventcontent", "FEVTDEBUG", "--datatier", "GEN-SIM-DIGI", "--processName", "TAUPRIME",
            "--number", str(options.maxEvents), "--fileout", "file:" + options.outputFile.removeprefix("file:"),
            "--python_filename", config, "--no_exec"], check=True)
        process = runpy.run_path(config)["process"]
    process = customise(process, options.mass, options.charge, massMode, counting)
    for index, name in enumerate(sorted(process.RandomNumberGeneratorService.parameterNames_())):
        pset = getattr(process.RandomNumberGeneratorService, name)
        if isinstance(pset, cms.PSet) and hasattr(pset, "initialSeed"):
            pset.initialSeed = cms.untracked.uint32(options.seed + index)
    process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1)
    process.MessageLogger.cerr.default.limit = cms.untracked.int32(0)
    for category in ("TauPrimeValidation", "TauPrimeDigi", "SimG4CoreApplication"):
        setattr(process.MessageLogger.cerr, category, cms.untracked.PSet(limit=cms.untracked.int32(-1)))
    for output in process.outputModules_().values():
        output.outputCommands.extend([
            "keep *_generator_*_*", "keep *_genParticles_*_*", "keep *_genFilterEfficiencyProducer_*_*",
            "keep *_g4SimHits_*_*", "keep *_simMuonCSCDigis_*_*", "keep *_simMuonDTDigis_*_*",
            "keep *_simMuonRPCDigis_*_*"])
    return process
