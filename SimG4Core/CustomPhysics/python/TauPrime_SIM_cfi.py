import re
import FWCore.ParameterSet.Config as cms

def customise(process):

    # cmsDriver's fragment dump only inlines actual EDFilter/EDProducer/Sequence
    # objects, not plain top-level PSets, so tauPrimeGenInfo (defined in the GEN
    # fragment) does not survive into the generated GEN,SIM config. Read the
    # already-resolved values back out of the baked Pythia8 parameter strings
    # on process.generator instead, which does survive the dump.
    TAUPRIME_MASS = None
    TAUPRIME_CHARGETYPE = None
    for line in process.generator.PythiaParameters.processParameters:
        m = re.match(r'^\s*17:m0\s*=\s*([\d.eE+-]+)', line)
        if m:
            TAUPRIME_MASS = float(m.group(1))
        m = re.match(r'^\s*17:chargeType\s*=\s*(-?\d+)', line)
        if m:
            TAUPRIME_CHARGETYPE = int(m.group(1))

    if TAUPRIME_MASS is None or TAUPRIME_CHARGETYPE is None:
        raise ValueError("TauPrime_SIM_cfi.customise: could not find 17:m0/17:chargeType "
                          "in process.generator.PythiaParameters.processParameters")

    process.load("SimG4Core.CustomPhysics.CustomPhysics_cfi")
    process.customPhysicsSetup.TauPrimeMass = TAUPRIME_MASS
    process.customPhysicsSetup.TauPrimeChargeType = TAUPRIME_CHARGETYPE

    if hasattr(process, 'g4SimHits'):
        process.g4SimHits.Physics.type = cms.string('SimG4Core/Physics/CustomPhysics')
        process.g4SimHits.Physics.ExoticaPhysicsSS = cms.untracked.bool(False)
        # add custom options
        process.g4SimHits.Physics = cms.PSet(
            process.g4SimHits.Physics, #keep all default value and add others
            process.customPhysicsSetup
        )

    return process
