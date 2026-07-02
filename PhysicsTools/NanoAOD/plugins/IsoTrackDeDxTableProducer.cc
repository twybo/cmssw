#include <memory>
#include <vector>
#include <cmath>
#include <limits>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "DataFormats/PatCandidates/interface/IsolatedTrack.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/TrackReco/interface/DeDxHitInfo.h"
#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "DataFormats/Math/interface/angle.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "DataFormats/SiStripDetId/interface/SiStripDetId.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"
#include "DataFormats/Math/interface/deltaR.h"

class IsoTrackDeDxTableProducer : public edm::global::EDProducer<> {
public:
  explicit IsoTrackDeDxTableProducer(const edm::ParameterSet&);
  ~IsoTrackDeDxTableProducer() override = default;
  static void fillDescriptions(edm::ConfigurationDescriptions&);

private:
  void produce(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

  // Stub — maintainer replaces with real geometry-based edge check.
  // Only dependency is <cmath> (fabs). No EventSetup needed.

  static bool IsHitNearEdge(int subdet, uint32_t detId, float localX, float localY) {
    bool found = false;
    if (subdet == 1) {
      if (localX > 0.79 || localX < -0.8) found = true;
      if (fabs(localY) > 3.2) found = true;
    } else if (subdet == 2) {
      if (fabs(localX) > 0.8) found = true;
      if (fabs(localY) > 3.2) found = true;
    } else if (subdet == 3) {
      if (fabs(localX) > 3.05) found = true;
      if (fabs(localY) > 5.6) found = true;
    } else if (subdet == 4) {
      unsigned int ring = (detId >> 9) & 0x3;
      if (ring == 1) {
        if (fabs(localY) < 5.5) found = true;
        if ((localX < 0 && (localY - 7.41f * localX - 28.23f) > 0) ||
            (localX > 0 && (localY + 7.41f * localX - 28.23f) > 0))
          found = true;
      } else if (ring == 2) {
        if (fabs(localY) < 4.35) found = true;
        if ((localX < 0 && (localY - 7.63f * localX - 37.2f) > 0) ||
            (localX > 0 && (localY + 7.47f * localX - 36.49f) > 0))
          found = true;
      } else if (ring == 3) {
        if (localY < -5.5 || localY > 5.45) found = true;
        if ((localX < 0 && (localY + 12.43f * localX + 44.6f) < 0) ||
            (localX > 0 && (localY - 12.51f * localX + 44.9f) < 0))
          found = true;
      } else {
        std::cout << "No ring found for TID, check for error\n";
      }
    } else if (subdet == 5) {
      if (fabs(localX) > 4.65) found = true;
      if (fabs(localY) < 0.25 || fabs(localY) > 9.2) found = true;
    } else if (subdet == 6) {
      unsigned int ring = (detId >> 5) & 0x7;
      if (ring == 1) {
        if (fabs(localY) > 4.2) found = true;
        if ((localX < 0 && (localY - 7.49f * localX - 27.65f) > 0) ||
            (localX > 0 && (localY + 7.51f * localX - 27.68f) > 0))
          found = true;
      } else if (ring == 2) {
        if (fabs(localY) > 4.35) found = true;
        if ((localX < 0 && (localY - 7.32f * localX - 35.7f) > 0) ||
            (localX > 0 && (localY + 7.45f * localX - 36.4f) > 0))
          found = true;
      } else if (ring == 3) {
        if (localY < -5.45 || localY > 5.5) found = true;
        if ((localX < 0 && (localY - 12.62f * localX - 45.24f) > 0) ||
            (localX > 0 && (localY + 12.46f * localX - 44.67f) > 0))
          found = true;
      } else if (ring == 4) {
        if (fabs(localY) > 5.7) found = true;
        if ((localX < 0 && (localY - 17.53f * localX - 56.39f) > 0) ||
            (localX > 0 && (localY + 17.38f * localX - 55.8f) > 0))
          found = true;
      } else if (ring == 5) {
        if (fabs(localY) > 7.3 || (localY > -0.82f && localY < -0.7f) ||
            (localY > -1.1f && localY < -1.0f))
          found = true;
        if ((localX < 0 && (localY - 12.45f * localX - 67.1f) > 0) ||
            (localX > 0 && (localY + 12.65f * localX - 68.18f) > 0))
          found = true;
      } else if (ring == 6) {
        if (fabs(localY) > 9.1 || (localY > -0.82f && localY < -0.75f) ||
            (localY > -0.52f && localY < -0.35f))
          found = true;
        if ((localX < 0 && (localY - 17.41f * localX - 81.56f) > 0) ||
            (localX > 0 && (localY + 17.48f * localX - 81.81f) > 0))
          found = true;
      } else if (ring == 7) {
        if (fabs(localY) > 10.15 || (localY > 0.4f && localY < 0.54f) ||
            (localY > 0.68f && localY < 0.84f))
          found = true;
        if ((localX < 0 && (localY + 24.53f * localX + 97.35f) < 0) ||
            (localX > 0 && (localY - 24.88f * localX - 98.68f) < 0))
          found = true;
      } else {
        std::cout << "No ring found for TEC, check for error\n";
      }
    } else {
      std::cout << "No subdetector found in IsHitNearEdge, check for error\n";
    }
    return found;
  }

  // Duplicated verbatim from MuonExtendedTableProducer::getPFIso() to avoid a
  // cross-producer data-product / scheduling-order dependency on PATmuonExtendedTable.
  static float getPFIso(const pat::Muon& muon) {
    return (muon.pfIsolationR04().sumChargedHadronPt +
            std::max(0., muon.pfIsolationR04().sumNeutralHadronEt + muon.pfIsolationR04().sumPhotonEt -
                             0.5 * muon.pfIsolationR04().sumPUPt)) /
           muon.pt();
  }

  // Encodes the analysis-level "GoodMuon" selection (eta, loose ID, PF iso, global,
  // muon timing / 1-over-beta consistent with a MIP-like candidate).
  static bool passesGoodMuon(const pat::Muon& mu, float pfIso) {
    constexpr float kEtaMin = -1.5f, kEtaMax = 1.5f;
    constexpr float kPFIsoMin = 0.f, kPFIsoMax = 0.15f;
    constexpr float kTimeMin = 0.f, kTimeMax = 25.f;
    constexpr float kTimeErrMin = 0.f, kTimeErrMax = 4.f;
    constexpr float kNDofMin = 8.f;
    constexpr float kInverseBetaMin = 1.0f;

    if (mu.eta() < kEtaMin || mu.eta() > kEtaMax) return false;
    if (!mu.isLooseMuon()) return false;
    if (pfIso < kPFIsoMin || pfIso > kPFIsoMax) return false;
    if (!mu.isGlobalMuon()) return false;
    if (mu.time().timeAtIpInOut < kTimeMin || mu.time().timeAtIpInOut > kTimeMax) return false;
    if (mu.time().timeAtIpInOutErr < kTimeErrMin || mu.time().timeAtIpInOutErr > kTimeErrMax) return false;
    if (mu.time().nDof < kNDofMin) return false;
    if (mu.inverseBeta() < kInverseBetaMin) return false;
    return true;
  }

  const std::string name_;
  const edm::EDGetTokenT<std::vector<pat::IsolatedTrack>> finalTracksToken_;
  const edm::EDGetTokenT<std::vector<pat::IsolatedTrack>> tracksToken_;
  const edm::EDGetTokenT<reco::DeDxHitInfoAss>            dedxToken_;
  const edm::EDGetTokenT<std::vector<pat::Muon>>          muonsToken_;
  const edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> trackerTopoToken_;
};

IsoTrackDeDxTableProducer::IsoTrackDeDxTableProducer(const edm::ParameterSet& iConfig)
    : name_(iConfig.getParameter<std::string>("name")),
      finalTracksToken_(consumes<std::vector<pat::IsolatedTrack>>(
          iConfig.getParameter<edm::InputTag>("finalIsolatedTracks"))),
      tracksToken_(consumes<std::vector<pat::IsolatedTrack>>(
          iConfig.getParameter<edm::InputTag>("isolatedTracks"))),
      dedxToken_(consumes<reco::DeDxHitInfoAss>(
          iConfig.getParameter<edm::InputTag>("dedx"))),
      muonsToken_(consumes<std::vector<pat::Muon>>(
          iConfig.getParameter<edm::InputTag>("muons"))),
      trackerTopoToken_(esConsumes()) {
  produces<nanoaod::FlatTable>(name_);
  produces<nanoaod::FlatTable>(name_ + "DeDxHits");
}

void IsoTrackDeDxTableProducer::produce(edm::StreamID,
                                        edm::Event& iEvent,
                                        const edm::EventSetup& iSetup) const {
  edm::Handle<std::vector<pat::IsolatedTrack>> finalH, origH;
  iEvent.getByToken(finalTracksToken_, finalH);
  iEvent.getByToken(tracksToken_,      origH);
  edm::Handle<reco::DeDxHitInfoAss> dedxH;
  iEvent.getByToken(dedxToken_, dedxH);
  edm::Handle<std::vector<pat::Muon>> muonsH;
  iEvent.getByToken(muonsToken_, muonsH);

  const TrackerTopology& tTopo = iSetup.getData(trackerTopoToken_);

  const size_t nFinal = finalH->size();

  // Per-track extension columns
  std::vector<int>   nDeDxHits(nFinal, 0);
  std::vector<int>   deDxHitFirstIdx(nFinal, -1);
  std::vector<int>   nPixelDeDxHits(nFinal, 0);
  std::vector<float> alphaMax(nFinal, 0.f);
  std::vector<bool>  hasNearEdge(nFinal, false);
  std::vector<bool>  isGlobalMuonMatched(nFinal, false);
  std::vector<bool>  hasGoodMuonMatch(nFinal, false);

  // Per-hit columns (accumulated across all tracks)
  std::vector<float> hit_dEdx;
  std::vector<int>   hit_layerIndex;
  std::vector<int>   hit_detId;

  // exact bit-identical comparison — valid because IsolatedTrackCleaner value-copies
  auto sameTrack = [](const pat::IsolatedTrack& a, const pat::IsolatedTrack& b) {
    return a.pt()     == b.pt()     && a.eta()    == b.eta()    && a.phi()    == b.phi() &&
           a.dxy()    == b.dxy()    && a.dz()     == b.dz()     && a.charge() == b.charge();
  };

  size_t o = 0;
  for (size_t f = 0; f < nFinal; ++f) {
    const auto& ft = (*finalH)[f];

    // Advance original cursor to the matching element (order-preserving subsequence)
    while (o < origH->size() && !sameTrack((*origH)[o], ft)) ++o;

    if (o >= origH->size()) {
      edm::LogWarning("IsoTrackDeDxTableProducer")
          << "finalIsolatedTracks[" << f << "] had no match in isolatedTracks — emitting defaults";
    }

    // Fetch dedx ref via the original isolatedTracks key
    reco::DeDxHitInfoRef dedxref;
    if (o < origH->size()) {
      dedxref = (*dedxH)[edm::Ref<std::vector<pat::IsolatedTrack>>(origH, o)];
    }

    // --- alphaMax: max 3D opening angle vs tracks pT>=35 and SA muons pT>35 ---
    float aMax = 0.f;
    for (const auto& t2 : *origH) {
      if (t2.pt() >= 35.f)
        aMax = std::max(aMax, (float)angle(ft.px(), ft.py(), ft.pz(),
                                           t2.px(), t2.py(), t2.pz()));
    }
    for (const auto& mu : *muonsH) {
      if (mu.pt() > 35.f && mu.isStandAloneMuon())
        aMax = std::max(aMax, (float)angle(ft.px(), ft.py(), ft.pz(),
                                           mu.px(), mu.py(), mu.pz()));
    }
    alphaMax[f] = aMax;

    // --- Nearest global muon match (unbounded search, threshold applied after) ---
    const pat::Muon* bestMuon = nullptr;
    float bestDR = std::numeric_limits<float>::max();
    for (const auto& mu : *muonsH) {
      if (!mu.isGlobalMuon()) continue;
      const float dR = reco::deltaR(ft.eta(), ft.phi(), mu.eta(), mu.phi());
      if (dR < bestDR) {
        bestDR   = dR;
        bestMuon = &mu;
      }
    }
    if (bestMuon != nullptr && bestDR < 0.05f) {
      isGlobalMuonMatched[f] = true;
      const float pfIso = getPFIso(*bestMuon);
      hasGoodMuonMatch[f] = passesGoodMuon(*bestMuon, pfIso);
    }

    // --- Per-hit loop (only for tracks matched to a GoodMuon) ---
    if (dedxref.isNonnull() && hasGoodMuonMatch[f]) {
      const reco::DeDxHitInfo& dedx = *dedxref;
      const int nHits = static_cast<int>(dedx.size());

      deDxHitFirstIdx[f] = static_cast<int>(hit_dEdx.size());
      nDeDxHits[f]       = nHits;

      int nPixel = 0;
      bool nearEdge = false;

      for (size_t i = 0; i < dedx.size(); ++i) {
        const DetId  dedxId  = dedx.detId(i);
        const float  localX  = dedx.pos(i).x();  // transient — not stored
        const float  localY  = dedx.pos(i).y();  // transient — not stored
        const int    subdet  = dedxId.subdetId();

        // Charge conversion matching legacy ECPTreeMaker exactly
        float charge;
        if (subdet < 3) {          // pixel (PXB=1, PXF=2)
          charge = 3.61e-06f * dedx.charge(i);
          ++nPixel;
        } else {                   // strip
          charge = 3.61e-06f * 265.f * dedx.charge(i);
        }
        hit_dEdx.push_back(charge / dedx.pathlength(i));

        // Layer index
        int layer = -1;
        if (subdet < 3) {
          if      (subdet == PixelSubdetector::PixelBarrel) layer = tTopo.pxbLayer(dedxId);
          else if (subdet == PixelSubdetector::PixelEndcap) layer = tTopo.pxfDisk(dedxId);
        } else {
          switch (SiStripDetId(dedxId).subDetector()) {
            case SiStripDetId::TIB: layer = tTopo.tibLayer(dedxId); break;
            case SiStripDetId::TID: layer = tTopo.tidWheel(dedxId); break;
            case SiStripDetId::TOB: layer = tTopo.tobLayer(dedxId); break;
            case SiStripDetId::TEC: layer = tTopo.tecWheel(dedxId); break;
            default: layer = -1;
          }
        }
        hit_layerIndex.push_back(layer);
        hit_detId.push_back(static_cast<int>(dedxId.rawId()));

        // Edge flag: short-circuit once true
        if (!nearEdge && IsHitNearEdge(subdet, dedxId.rawId(), localX, localY))
          nearEdge = true;
      }

      nPixelDeDxHits[f] = nPixel;
      hasNearEdge[f]     = nearEdge;
    }
    // else: nDeDxHits=0, deDxHitFirstIdx=-1, hasNearEdge=false — already initialised

    if (o < origH->size()) ++o;
  }

  // Build and put IsoTrack extension table
  auto trkTab = std::make_unique<nanoaod::FlatTable>(nFinal, name_, false, /*extension=*/true);
  trkTab->addColumn<int>  ("nDeDxHits",            nDeDxHits,       "number of dE/dx hits");
  trkTab->addColumn<int>  ("deDxHitFirstIdx",      deDxHitFirstIdx, "first row in IsoTrackDeDxHits (-1 if none)");
  trkTab->addColumn<int>  ("nPixelDeDxHits",       nPixelDeDxHits,  "number of pixel dE/dx hits");
  trkTab->addColumn<float>("alphaMax",             alphaMax,        "max 3D opening angle vs tracks/SA muons pT>35 [rad]");
  trkTab->addColumn<bool> ("hasTrackerHitNearEdge",hasNearEdge,     "any dE/dx hit near a sensor edge");
  trkTab->addColumn<bool> ("isGlobalMuonMatched",  isGlobalMuonMatched, "nearest global muon within dR<0.05");
  trkTab->addColumn<bool> ("hasGoodMuonMatch",     hasGoodMuonMatch,    "dR-matched global muon passes GoodMuon selection (gates per-hit dE/dx storage)");

  // Build and put per-hit table
  const size_t nHitsTotal = hit_dEdx.size();
  auto hitTab = std::make_unique<nanoaod::FlatTable>(nHitsTotal, name_ + "DeDxHits", false, false);
  hitTab->addColumn<float>("hit_dEdx",       hit_dEdx,       "dE/dx per hit: charge/pathlength [MeV/cm]");
  hitTab->addColumn<int>  ("hit_layerIndex", hit_layerIndex, "pixel layer / strip layer or wheel");
  hitTab->addColumn<int>  ("hit_detId",      hit_detId,      "raw DetId (read back as uint32)");

  iEvent.put(std::move(trkTab), name_);
  iEvent.put(std::move(hitTab), name_ + "DeDxHits");
}

void IsoTrackDeDxTableProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::string>("name", "IsoTrack")->setComment("name of the IsoTrack FlatTable (also prefixes the hit table)");
  desc.add<edm::InputTag>("finalIsolatedTracks", edm::InputTag("finalIsolatedTracks"))->setComment("cleaned isolated track collection (row identity/order)");
  desc.add<edm::InputTag>("isolatedTracks",      edm::InputTag("isolatedTracks"))     ->setComment("full isolated track collection (association key + alphaMax)");
  desc.add<edm::InputTag>("dedx",                edm::InputTag("isolatedTracks"))     ->setComment("DeDxHitInfo association product (same label as isolatedTracks)");
  desc.add<edm::InputTag>("muons",               edm::InputTag("linkedObjects", "muons"))->setComment("PAT muon collection for alphaMax SA-muon term");
  descriptions.add("IsoTrackDeDxTable", desc);
}

DEFINE_FWK_MODULE(IsoTrackDeDxTableProducer);
