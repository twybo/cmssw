"""Build runnable Run-3 tau-prime GEN/SIM/DIGI processes."""

import FWCore.ParameterSet.Config as cms

from SimMuon.Configuration.customiseTauPrime import customise


def make_process(
    massMode: str = "fixed",
    counting: str = "clusters",
    *,
    mass: float = 1000.0,
    charge: float = 1.0,
    maxEvents: int = 10,
    seed: int = 12345,
    outputFile: str = "output.root",
) -> cms.Process:
    import runpy
    import subprocess
    import tempfile
    from pathlib import Path

    if type(maxEvents) is not int or maxEvents <= 0:
        raise ValueError("Event count must be a positive integer")
    if type(seed) is not int or not 1 <= seed <= 899999000:
        raise ValueError("Base random seed must be an integer in [1, 899999000]")
    with tempfile.TemporaryDirectory(prefix="tauprime-config-") as directory:
        config = str(Path(directory) / "generated.py")
        subprocess.run(
            [
                "cmsDriver.py",
                "Configuration/GenProduction/python/ZprimeToECPs_fragment_cff.py",
                "--mc",
                "--conditions",
                "auto:phase1_2024_realistic",
                "--era",
                "Run3_2024",
                "--geometry",
                "DB:Extended",
                "--step",
                "GEN,SIM,DIGI:pdigi_valid",
                "--beamspot",
                "DBrealistic",
                "--pileup",
                "NoPileUp",
                "--nThreads",
                "1",
                "--nStreams",
                "1",
                "--eventcontent",
                "FEVTDEBUG",
                "--datatier",
                "GEN-SIM-DIGI",
                "--processName",
                "TAUPRIME",
                "--number",
                str(maxEvents),
                "--fileout",
                "file:" + outputFile.removeprefix("file:"),
                "--python_filename",
                config,
                "--no_exec",
            ],
            check=True,
        )
        process = runpy.run_path(config)["process"]
    process = customise(process, mass, charge, massMode, counting)
    for index, name in enumerate(
        sorted(process.RandomNumberGeneratorService.parameterNames_())
    ):
        pset = getattr(process.RandomNumberGeneratorService, name)
        if isinstance(pset, cms.PSet) and hasattr(pset, "initialSeed"):
            pset.initialSeed = cms.untracked.uint32(seed + index)
    process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1)
    process.MessageLogger.cerr.default.limit = cms.untracked.int32(0)
    for category in ("TauPrimeValidation", "TauPrimeDigi", "SimG4CoreApplication"):
        setattr(
            process.MessageLogger.cerr,
            category,
            cms.untracked.PSet(limit=cms.untracked.int32(-1)),
        )
    for output in process.outputModules_().values():
        output.outputCommands.extend(
            [
                "keep *_generator_*_*",
                "keep *_genParticles_*_*",
                "keep *_genFilterEfficiencyProducer_*_*",
                "keep *_g4SimHits_*_*",
                "keep *_simMuonCSCDigis_*_*",
                "keep *_simMuonDTDigis_*_*",
                "keep *_simMuonRPCDigis_*_*",
            ]
        )
    return process
