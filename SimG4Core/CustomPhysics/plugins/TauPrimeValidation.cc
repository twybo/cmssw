#include "SimG4Core/Watcher/interface/SimWatcher.h"
#include "SimG4Core/Watcher/interface/SimWatcherFactory.h"
#include "SimG4Core/Notification/interface/Observer.h"
#include "SimG4Core/Notification/interface/BeginOfRun.h"
#include "SimG4Core/Notification/interface/BeginOfTrack.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4DynamicParticle.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4VProcess.hh"
#include "G4Track.hh"
#include "G4MuIonisation.hh"
#include "G4MuMultipleScattering.hh"
#include <CLHEP/Units/SystemOfUnits.h>

#include <array>
#include <cmath>
#include <typeinfo>

class TauPrimeValidation : public SimWatcher,
                           public Observer<const BeginOfRun*>,
                           public Observer<const BeginOfTrack*> {
public:
  explicit TauPrimeValidation(const edm::ParameterSet& config) {
    const auto parameters = config.getParameter<edm::ParameterSet>("TauPrimeValidation");
    massGeV_ = parameters.getParameter<double>("massGeV");
    chargeMagnitude_ = parameters.getParameter<double>("chargeMagnitude");
    setMT(true);
  }

  void update(const BeginOfRun*) override {
    reportedPrimary_.fill(false);
    for (int sign : {1, -1}) {
      const auto* particle = G4ParticleTable::GetParticleTable()->FindParticle(sign * 17);
      if (!particle) {
        throw cms::Exception("TauPrimeValidation") << "Missing PDG " << sign * 17;
      }
      edm::LogVerbatim("TauPrimeValidation")
          << "TAUPRIME_DEFINITION pdg=" << particle->GetPDGEncoding()
          << " massGeV=" << particle->GetPDGMass() / CLHEP::GeV
          << " chargeInE=" << particle->GetPDGCharge() / CLHEP::eplus
          << " particleType=" << particle->GetParticleType()
          << " twiceSpin=" << particle->GetPDGiSpin()
          << " lepton=" << particle->GetLeptonNumber() << " baryon=" << particle->GetBaryonNumber()
          << " stable=" << particle->GetPDGStable() << " antiPDG=" << particle->GetAntiPDGEncoding()
          << " widthGeV=" << particle->GetPDGWidth() / CLHEP::GeV
          << " lifetime=" << particle->GetPDGLifeTime();
      if (std::abs(particle->GetPDGMass() / CLHEP::GeV - massGeV_) > 1.e-10 * massGeV_ ||
          std::abs(particle->GetPDGCharge() / CLHEP::eplus + sign * chargeMagnitude_) > 1.e-12 ||
          particle->GetPDGiSpin() != 1 || particle->GetLeptonNumber() != sign ||
          particle->GetBaryonNumber() != 0 || particle->GetParticleType() != "custom" || !particle->GetPDGStable() ||
          particle->GetPDGWidth() != 0. || particle->GetPDGLifeTime() != -1. ||
          particle->GetAntiPDGEncoding() != -sign * 17) {
        throw cms::Exception("TauPrimeValidation") << "Incorrect particle properties for PDG " << sign * 17;
      }

      const auto* manager = particle->GetProcessManager();
      if (!manager) {
        throw cms::Exception("TauPrimeValidation") << "No process manager for PDG " << sign * 17;
      }
      const auto* processes = manager->GetProcessList();
      int ionisation = 0;
      int scattering = 0;
      bool transportation = false;
      for (int index = 0; index < manager->GetProcessListLength(); ++index) {
        const auto* process = (*processes)[index];
        edm::LogVerbatim("TauPrimeValidation")
            << "TAUPRIME_PROCESS pdg=" << sign * 17 << " name=" << process->GetProcessName()
            << " type=" << process->GetProcessType() << " subtype=" << process->GetProcessSubType();
        const bool isIonisation = typeid(*process) == typeid(G4MuIonisation);
        const bool isScattering = typeid(*process) == typeid(G4MuMultipleScattering);
        ionisation += isIonisation;
        scattering += isScattering;
        transportation |= process->GetProcessType() == fTransportation;
        if (process->GetProcessType() != fTransportation && !isIonisation && !isScattering) {
          throw cms::Exception("TauPrimeValidation") << "Unexpected muon-variant process: "
                                                     << process->GetProcessName();
        }
      }
      if (ionisation != 1 || scattering != 1 || !transportation) {
        throw cms::Exception("TauPrimeValidation")
            << "Incorrect muon-variant process list for PDG " << sign * 17
            << ": G4MuIonisation=" << ionisation << " G4MuMultipleScattering=" << scattering
            << " transportation=" << transportation;
      }
    }
  }

  void update(const BeginOfTrack* notification) override {
    const auto* track = (*notification)();
    const int pdg = track->GetDefinition()->GetPDGEncoding();
    if (std::abs(pdg) != 17 || track->GetParentID() != 0) {
      return;
    }
    const auto* dynamic = track->GetDynamicParticle();
    if (std::abs(dynamic->GetMass() / CLHEP::GeV - massGeV_) > 1.e-8 * massGeV_ ||
        std::abs(dynamic->GetCharge() / CLHEP::eplus + (pdg > 0 ? 1 : -1) * chargeMagnitude_) > 1.e-12) {
      throw cms::Exception("TauPrimeValidation") << "Incorrect dynamical primary properties for PDG " << pdg;
    }
    const unsigned int index = pdg > 0 ? 0 : 1;
    if (!reportedPrimary_[index]) {
      edm::LogVerbatim("TauPrimeValidation")
          << "TAUPRIME_PRIMARY pdg=" << pdg << " massGeV=" << dynamic->GetMass() / CLHEP::GeV
          << " chargeInE=" << dynamic->GetCharge() / CLHEP::eplus
          << " momentumGeV=" << track->GetMomentum().mag() / CLHEP::GeV
          << " energyGeV=" << track->GetTotalEnergy() / CLHEP::GeV;
      reportedPrimary_[index] = true;
    }
  }

private:
  double massGeV_;
  double chargeMagnitude_;
  std::array<bool, 2> reportedPrimary_{};
};

DEFINE_SIMWATCHER(TauPrimeValidation);
