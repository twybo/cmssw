from SimMuon.Configuration.tauPrimeProcess import make_process

# Edit the run settings here.
process = make_process(
    massMode="fixed",
    counting="electrons",
    mass=1000.0,  # Nominal mass in GeV; mHatMin is max(50 GeV, mass).
    charge=1.0,  # Transport charge magnitude in units of e.
    maxEvents=10,
    seed=12345,
    outputFile="output.root",
)
