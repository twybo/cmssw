#ifndef SimG4Core_CustomPhysics_TauPrimePairProductionModel_h
#define SimG4Core_CustomPhysics_TauPrimePairProductionModel_h

#include "G4hPairProductionModel.hh"

class TauPrimePairProductionModel final : public G4hPairProductionModel {
public:
  explicit TauPrimePairProductionModel(const G4ParticleDefinition* particle);

  void Initialise(const G4ParticleDefinition* particle, const G4DataVector& cuts) override;
};

#endif
