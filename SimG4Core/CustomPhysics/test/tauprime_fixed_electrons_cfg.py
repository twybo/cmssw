from SimG4Core.CustomPhysics.TauPrimeUnified_cff import make_process

# Edit these values here; optional command-line arguments override them.
process = make_process(
    massMode="fixed",
    counting="electrons",
    mass=1000.0,  # Nominal mass in GeV; mHatMin is max(50 GeV, mass).
    charge=1.0,  # Transport charge magnitude in units of e.
    maxEvents=10,
    seed=12345,
    outputFile="output.root",
)
