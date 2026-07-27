#include <memory>
#include <vector>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <utility>

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
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/TrackReco/interface/DeDxHitInfo.h"
#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/Provenance/interface/ProductID.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "DataFormats/SiStripDetId/interface/SiStripDetId.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"

namespace {
  struct PairHash {
    size_t operator()(const std::pair<edm::ProductID, unsigned int>& p) const {
      // edm::ProductID has no std::hash specialization; hash its process/product indices directly.
      const size_t h1 = (static_cast<size_t>(p.first.processIndex()) << 16) | p.first.productIndex();
      return std::hash<size_t>()(h1) ^ (std::hash<unsigned int>()(p.second) << 1);
    }
  };
}  // namespace

class MuonDeDxTableProducer : public edm::global::EDProducer<> {
public:
  explicit MuonDeDxTableProducer(const edm::ParameterSet&);
  ~MuonDeDxTableProducer() override = default;
  static void fillDescriptions(edm::ConfigurationDescriptions&);

private:
  void produce(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

  // Duplicated from IsHitNearEdge in IsoTrackDeDxTableProducer.cc, with the
  // hand-rolled TID/TEC ring bit masks replaced by TrackerTopology accessors.
  static bool IsHitNearEdge(int subdet, const TrackerTopology& tTopo, DetId detId, float localX, float localY) {
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
      unsigned int ring = tTopo.tidRing(detId);
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
        edm::LogWarning("MuonDeDxTableProducer") << "No ring found for TID, check for error";
      }
    } else if (subdet == 5) {
      if (fabs(localX) > 4.65) found = true;
      if (fabs(localY) < 0.25 || fabs(localY) > 9.2) found = true;
    } else if (subdet == 6) {
      unsigned int ring = tTopo.tecRing(detId);
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
        edm::LogWarning("MuonDeDxTableProducer") << "No ring found for TEC, check for error";
      }
    } else {
      edm::LogWarning("MuonDeDxTableProducer") << "No subdetector found in IsHitNearEdge, check for error";
    }
    return found;
  }

  // Duplicated verbatim from IsoTrackDeDxTableProducer::getPFIso() (itself duplicated
  // from MuonExtendedTableProducer::getPFIso()) to avoid a cross-producer dependency.
  static float getPFIso(const pat::Muon& muon) {
    return (muon.pfIsolationR04().sumChargedHadronPt +
            std::max(0., muon.pfIsolationR04().sumNeutralHadronEt + muon.pfIsolationR04().sumPhotonEt -
                             0.5 * muon.pfIsolationR04().sumPUPt)) /
           muon.pt();
  }

  // Duplicated verbatim from IsoTrackDeDxTableProducer::passesGoodMuon().
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
  const edm::EDGetTokenT<std::vector<pat::Muon>>          muonsToken_;
  const edm::EDGetTokenT<std::vector<pat::IsolatedTrack>> isoTracksToken_;
  const edm::EDGetTokenT<reco::DeDxHitInfoAss>            dedxToken_;
  const edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> trackerTopoToken_;
};

MuonDeDxTableProducer::MuonDeDxTableProducer(const edm::ParameterSet& iConfig)
    : name_(iConfig.getParameter<std::string>("name")),
      muonsToken_(consumes<std::vector<pat::Muon>>(iConfig.getParameter<edm::InputTag>("muons"))),
      isoTracksToken_(consumes<std::vector<pat::IsolatedTrack>>(
          iConfig.getParameter<edm::InputTag>("isolatedTracks"))),
      dedxToken_(consumes<reco::DeDxHitInfoAss>(iConfig.getParameter<edm::InputTag>("dedx"))),
      trackerTopoToken_(esConsumes()) {
  produces<nanoaod::FlatTable>(name_);
  produces<nanoaod::FlatTable>(name_ + "DeDxHits");
}

void MuonDeDxTableProducer::produce(edm::StreamID,
                                     edm::Event& iEvent,
                                     const edm::EventSetup& iSetup) const {
  edm::Handle<std::vector<pat::Muon>> muonsH;
  iEvent.getByToken(muonsToken_, muonsH);
  edm::Handle<std::vector<pat::IsolatedTrack>> isoTracksH;
  iEvent.getByToken(isoTracksToken_, isoTracksH);
  edm::Handle<reco::DeDxHitInfoAss> dedxH;
  iEvent.getByToken(dedxToken_, dedxH);

  const TrackerTopology& tTopo = iSetup.getData(trackerTopoToken_);

  // Map packedCandidate identity (ProductID, key) -> isolatedTracks index,
  // mirroring the identity IsolatedTrackCleaner itself matches on.
  std::unordered_map<std::pair<edm::ProductID, unsigned int>, size_t, PairHash> candToIsoIdx;
  candToIsoIdx.reserve(isoTracksH->size());
  for (size_t j = 0; j < isoTracksH->size(); ++j) {
    const auto& ref = (*isoTracksH)[j].packedCandRef();
    if (ref.isNonnull()) {
      candToIsoIdx[{ref.id(), ref.key()}] = j;
    }
  }

  const size_t nMuons = muonsH->size();

  // Per-muon extension columns
  std::vector<int>  nDeDxHits(nMuons, 0);
  std::vector<int>  deDxHitFirstIdx(nMuons, -1);
  std::vector<int>  nPixelDeDxHits(nMuons, 0);
  std::vector<bool> hasNearEdge(nMuons, false);
  std::vector<int>  isoTrackIdx(nMuons, -1);

  // Per-hit columns (accumulated across all muons)
  std::vector<float> hit_dEdx;
  std::vector<int>   hit_layerIndex;
  std::vector<int>   hit_detId;

  for (size_t i = 0; i < nMuons; ++i) {
    const pat::Muon& mu = (*muonsH)[i];

    const float pfIso = getPFIso(mu);
    if (!passesGoodMuon(mu, pfIso)) continue;

    size_t matchedIdx = std::numeric_limits<size_t>::max();
    for (size_t k = 0; k < mu.numberOfSourceCandidatePtrs(); ++k) {
      const reco::CandidatePtr ptr = mu.sourceCandidatePtr(k);
      if (ptr.isNull()) continue;
      auto it = candToIsoIdx.find({ptr.id(), ptr.key()});
      if (it != candToIsoIdx.end()) {
        matchedIdx = it->second;
        break;
      }
    }

    if (matchedIdx == std::numeric_limits<size_t>::max()) {
      edm::LogWarning("MuonDeDxTableProducer")
          << "GoodMuon[" << i << "] (pt=" << mu.pt() << ", eta=" << mu.eta()
          << ") had no matching isolatedTracks entry — emitting defaults";
      continue;
    }

    isoTrackIdx[i] = static_cast<int>(matchedIdx);

    const reco::DeDxHitInfoRef dedxref =
        (*dedxH)[edm::Ref<std::vector<pat::IsolatedTrack>>(isoTracksH, matchedIdx)];
    if (dedxref.isNull()) continue;

    const reco::DeDxHitInfo& dedx = *dedxref;
    const int nHits = static_cast<int>(dedx.size());

    deDxHitFirstIdx[i] = static_cast<int>(hit_dEdx.size());
    nDeDxHits[i]       = nHits;

    int nPixel = 0;
    bool nearEdge = false;

    for (size_t h = 0; h < dedx.size(); ++h) {
      const DetId dedxId = dedx.detId(h);
      const float localX = dedx.pos(h).x();  // transient — not stored
      const float localY = dedx.pos(h).y();  // transient — not stored
      const int   subdet = dedxId.subdetId();

      // Charge conversion matching legacy ECPTreeMaker exactly
      float charge;
      if (subdet < 3) {  // pixel (PXB=1, PXF=2)
        charge = 3.61e-06f * dedx.charge(h);
        ++nPixel;
      } else {  // strip
        charge = 3.61e-06f * 265.f * dedx.charge(h);
      }
      hit_dEdx.push_back(charge / dedx.pathlength(h));

      // Layer index
      int layer = -1;
      if (subdet < 3) {
        if (subdet == PixelSubdetector::PixelBarrel) layer = tTopo.pxbLayer(dedxId);
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
      if (!nearEdge && IsHitNearEdge(subdet, tTopo, dedxId, localX, localY)) nearEdge = true;
    }

    nPixelDeDxHits[i] = nPixel;
    hasNearEdge[i]    = nearEdge;
  }

  // Build and put Muon extension table
  auto muTab = std::make_unique<nanoaod::FlatTable>(nMuons, name_, false, /*extension=*/true);
  muTab->addColumn<int>("nDeDxHits", nDeDxHits, "number of dE/dx hits");
  muTab->addColumn<int>("deDxHitFirstIdx", deDxHitFirstIdx, "first row in MuonDeDxHits (-1 if none)");
  muTab->addColumn<int>("nPixelDeDxHits", nPixelDeDxHits, "number of pixel dE/dx hits");
  muTab->addColumn<bool>("hasTrackerHitNearEdge", hasNearEdge, "any dE/dx hit near a sensor edge");
  muTab->addColumn<int>("isoTrackIdx", isoTrackIdx, "matched isolatedTracks index, -1 if none (debug)");

  // Build and put per-hit table
  const size_t nHitsTotal = hit_dEdx.size();
  auto hitTab = std::make_unique<nanoaod::FlatTable>(nHitsTotal, name_ + "DeDxHits", false, false);
  hitTab->addColumn<float>("hit_dEdx", hit_dEdx, "dE/dx per hit: charge/pathlength [MeV/cm]");
  hitTab->addColumn<int>("hit_layerIndex", hit_layerIndex, "pixel layer / strip layer or wheel");
  hitTab->addColumn<int>("hit_detId", hit_detId, "raw DetId (read back as uint32)");

  iEvent.put(std::move(muTab), name_);
  iEvent.put(std::move(hitTab), name_ + "DeDxHits");
}

void MuonDeDxTableProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::string>("name", "Muon")->setComment("name of the Muon FlatTable being extended (also prefixes the hit table)");
  desc.add<edm::InputTag>("muons", edm::InputTag("linkedObjects", "muons"))->setComment("PAT muon collection (row identity/order — same source as the Muon main table)");
  desc.add<edm::InputTag>("isolatedTracks", edm::InputTag("isolatedTracks"))->setComment("full (uncleaned) isolated track collection used for packedCandidate identity matching");
  desc.add<edm::InputTag>("dedx", edm::InputTag("isolatedTracks"))->setComment("DeDxHitInfo association product (same label as isolatedTracks)");
  descriptions.add("MuonDeDxTable", desc);
}

DEFINE_FWK_MODULE(MuonDeDxTableProducer);
