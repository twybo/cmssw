// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************

#include "SimG4Core/CustomPhysics/interface/TauPrimePairProductionModel.h"

#include "G4AutoLock.hh"
#include "G4ElementDataRegistry.hh"
#include "G4EmParameters.hh"
#include "G4Log.hh"
#include "G4ParticleDefinition.hh"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {
  G4Mutex tauPrimePairTableMutex = G4MUTEX_INITIALIZER;
}

TauPrimePairProductionModel::TauPrimePairProductionModel(const G4ParticleDefinition* particle)
    : G4hPairProductionModel(particle) {}

void TauPrimePairProductionModel::Initialise(const G4ParticleDefinition* p, const G4DataVector& cuts) {
  // Adapt only G4MuPairProductionModel::Initialise from Geant4 v11.2.2:
  // its private dataName stays empty for PDG 17 and can alias unnamed tables.
  SetParticle(p);

  if (nullptr == fParticleChange) {
    fParticleChange = GetParticleChangeForLoss();
    if (0 == nbine) {
      emin = std::max(lowestKinEnergy, LowEnergyLimit());
      emax = std::max(HighEnergyLimit(), emin * 2);
      nbine = std::size_t(nYBinPerDecade * std::log10(emax / emin));
      if (nbine < 3) {
        nbine = 3;
      }
      ymin = G4Log(minPairEnergy / emin);
      dy = -ymin / G4double(nbiny);
    }
  }

  if (lowestKinEnergy >= HighEnergyLimit() || p != particle) {
    return;
  }

  std::ostringstream name;
  name << "tauPrimeEEPairProd:" << p->GetPDGEncoding() << ':' << std::hexfloat << p->GetPDGMass() << ':'
       << p->GetPDGCharge() << ':' << emin << ':' << emax;
  auto* registry = G4ElementDataRegistry::Instance();
  G4AutoLock lock(&tauPrimePairTableMutex);
  fElementData = registry->GetElementDataByName(name.str());
  if (nullptr == fElementData) {
    fElementData = new G4ElementData(NZDATPAIR);
    fElementData->SetName(name.str());
    G4bool useDataFile = G4EmParameters::Instance()->RetrieveMuDataFromFile();
    if (useDataFile) {
      useDataFile = RetrieveTables();
    }
    if (!useDataFile) {
      MakeSamplingTables();
    }
    if (fTableToFile) {
      StoreTables();
    }
  }
  lock.unlock();

  if (IsMaster()) {
    InitialiseElementSelectors(p, cuts);
  }
}
