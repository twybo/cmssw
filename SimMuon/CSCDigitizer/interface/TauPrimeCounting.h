#ifndef SimMuon_CSCDigitizer_TauPrimeCounting_h
#define SimMuon_CSCDigitizer_TauPrimeCounting_h

#include <vector>

namespace tauprime {
  struct Counts {
    int electrons = 0;
    int survivingElectrons = 0;
    int clusters = 0;
    int survivingClusters = 0;
  };

  template <typename Draw>
  Counts countSurvivors(const std::vector<int> &electrons, bool clusterMode, Draw draw) {
    Counts result;
    for (int count : electrons) {
      result.electrons += count;
      ++result.clusters;
      int survivors = 0;
      if (clusterMode) {
        if (draw() < 0.667)
          survivors = count;
      } else {
        for (int i = 0; i < count; ++i)
          if (draw() < 0.667)
            ++survivors;
      }
      result.survivingElectrons += survivors;
      result.survivingClusters += survivors > 0;
    }
    return result;
  }
}

#endif
