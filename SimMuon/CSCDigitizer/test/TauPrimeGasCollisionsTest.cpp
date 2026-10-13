#include "CLHEP/Random/JamesRandom.h"
#include "SimMuon/CSCDigitizer/interface/CSCGasCollisions.h"
#include "SimMuon/CSCDigitizer/src/CSCCrossGap.h"
#include "FWCore/Utilities/interface/Exception.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

class TauPrimeGasCollisionsTest {
public:
  static void require(bool ok, const char *message) {
    if (!ok)
      throw std::runtime_error(message);
  }

  static edm::ParameterSet config(double mass, double charge, bool dump = false) {
    edm::ParameterSet pset;
    pset.addParameter<double>("TauPrimeMassGeV", mass);
    pset.addParameter<double>("TauPrimeChargeMagnitude", charge);
    pset.addParameter<bool>("TauPrimeDigiValidation", false);
    pset.addUntrackedParameter<bool>("dumpGasCollisions", dump);
    return pset;
  }

  static PSimHit hit(int pdg = 17) {
    return PSimHit(LocalPoint(0., 0., -0.25), LocalPoint(0., 0., 0.25), 100., 0., 0.000005,
                   pdg, 1, 1, 0., 0., 0);
  }

  static void run() {
    CSCGasCollisions gas(config(100., 1.));
    std::vector<float> values(CSCGasCollisions::N_ENERGY);
    const float top = gas.theGammaBins.back();
    for (float gamma : {top, top + 1.f}) {
      gas.fillCollisionsForThisGamma(gamma, values, true);
      for (int i = 0; i < CSCGasCollisions::N_ENERGY; ++i)
        require(values[i] == gas.theCollisionTable[i * CSCGasCollisions::N_GAMMA +
                                                   CSCGasCollisions::N_GAMMA - 1], "exotic upper column");
      gas.fillCollisionsForThisGamma(gamma, values, false);
      for (int i = 0; i < CSCGasCollisions::N_ENERGY; ++i)
        require(values[i] == gas.theCollisionTable[i * CSCGasCollisions::N_GAMMA], "ordinary upper column changed");
    }
    gas.fillCollisionsForThisGamma(std::nextafter(top, -INFINITY), values, true);
    for (int i = 0; i < CSCGasCollisions::N_ENERGY; ++i)
      require(std::abs(values[i] - gas.theCollisionTable[i * CSCGasCollisions::N_GAMMA +
                                                       CSCGasCollisions::N_GAMMA - 1]) < 1.e-4, "upper-bin continuity");
    require(gas.isTauPrime(hit(17)) && gas.isTauPrime(hit(-17)) && !gas.isTauPrime(hit(13)), "PDG scope");
    CSCGasCollisions unit(config(100., 1., true));
    CSCGasCollisions third(config(100., 1. / 3., true));
    CLHEP::HepJamesRandom unitEngine(321), thirdEngine(321);
    std::vector<LocalPoint> positions;
    std::vector<int> electrons;
    unit.simulate(hit(), positions, electrons, &unitEngine);
    third.simulate(hit(), positions, electrons, &thirdEngine);
    require(std::abs(third.theCrossGap->stepLengths().front() /
                     unit.theCrossGap->stepLengths().front() - 9.) < 1.e-12, "reciprocal charge-squared spacing");
    require(std::abs(unit.theCrossGap->gamma() - std::sqrt(2.)) < 1.e-12, "runtime mass not used");

    bool testedNonIonizingPredecessor = false;
    for (long seed = 1; seed < 1000 && !testedNonIonizingPredecessor; ++seed) {
      CLHEP::HepJamesRandom actual(seed), reference(seed);
      gas.simulate(hit(), positions, electrons, &actual);
      if (positions.empty())
        continue;
      gas.fillCollisionsForThisGamma(gas.theCrossGap->logGamma(), values, true);
      const double anmin = std::exp(values.back());
      const double anmax = std::exp(values.front());
      const double rate = anmax - anmin;
      double distance = 0.;
      bool skippedIonization = false;
      for (int attempt = 0; attempt < CSCGasCollisions::MAX_STEPS; ++attempt) {
        double step = gas.generateStep(rate, &reference);
        if (distance + step > gas.theCrossGap->length())
          break;
        double energy = gas.generateEnergyLoss(rate, anmin, anmax, values, &reference);
        if (energy > gas.deCut)
          continue;
        distance += step;
        if (energy <= gas.eion) {
          skippedIonization = true;
          continue;
        }
        if (skippedIonization) {
          const LocalPoint expected = hit().entryPoint() + distance * gas.theCrossGap->unitVector();
          require((positions.front() - expected).mag() < 1.e-6, "non-ionizing steps omitted from coordinates");
          testedNonIonizingPredecessor = true;
        }
        break;
      }
    }
    require(testedNonIonizingPredecessor, "no coordinate regression case exercised");
    CLHEP::HepJamesRandom positive(17), negative(17);
    std::vector<LocalPoint> positivePositions;
    std::vector<int> positiveElectrons;
    gas.simulate(hit(17), positivePositions, positiveElectrons, &positive);
    gas.simulate(hit(-17), positions, electrons, &negative);
    require(positiveElectrons == electrons && positivePositions.size() == positions.size(), "particle signs differ");
    for (size_t i = 0; i < positions.size(); ++i)
      require((positivePositions[i] - positions[i]).mag() == 0., "particle sign positions differ");
    for (double charge : {0., -1., std::numeric_limits<double>::infinity(), 1.e-200, 1.e200}) {
      bool rejected = false;
      try {
        CSCGasCollisions invalid(config(100., charge));
      } catch (const cms::Exception &) {
        rejected = true;
      }
      require(rejected, "invalid charge accepted");
    }
    std::cout << "TauPrimeGasCollisionsTest passed" << std::endl;
  }
};

int main() { TauPrimeGasCollisionsTest::run(); }
