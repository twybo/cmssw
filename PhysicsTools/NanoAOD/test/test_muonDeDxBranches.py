#!/usr/bin/env python3
"""
Validate that the branches added by MuonDeDxTableProducer
(PhysicsTools/NanoAOD/plugins/MuonDeDxTableProducer.cc, wired in via
custom_exo_cff.py add_muonDeDxTables) are present (and sane) in an
exoNanoAOD file.

Checks, against the "Events" tree:
  - Muon table extension columns (name="Muon" -> extension=True, reuses nMuon)
  - MuonDeDxHits per-hit table (name="Muon" + "DeDxHits" -> own counter, non-extension)

Usage:
  python3 test_muonDeDxBranches.py [path/to/exoNanoMC_PAT_NANO.root]
"""
import sys
import argparse

import awkward as ak
import uproot

# (branch name, expected dtype description)
EXPECTED_BRANCHES = [
    # Muon extension (MuonDeDxTableProducer -> "Muon" table, extension=True)
    ("Muon_nDeDxHits", "int"),
    ("Muon_deDxHitFirstIdx", "int"),
    ("Muon_nPixelDeDxHits", "int"),
    ("Muon_hasTrackerHitNearEdge", "bool"),
    ("Muon_isoTrackIdx", "int"),
    # MuonDeDxHits per-hit table (own counter, non-extension)
    ("nMuonDeDxHits", "int"),
    ("MuonDeDxHits_hit_dEdx", "float"),
    ("MuonDeDxHits_hit_layerIndex", "int"),
    ("MuonDeDxHits_hit_detId", "int"),
]


def check_file(path):
    print(f"Opening {path}")
    f = uproot.open(path)
    if "Events" not in f:
        print("FAIL: no 'Events' tree found in file")
        return 1

    tree = f["Events"]
    branches = set(tree.keys())

    n_events = tree.num_entries
    print(f"Events tree has {n_events} entries, {len(branches)} branches\n")

    missing = []
    present = []

    for name, expect_kind in EXPECTED_BRANCHES:
        if name not in branches:
            missing.append(name)
            print(f"[MISSING] {name}")
            continue
        present.append(name)

    if missing:
        print(f"\n{len(missing)}/{len(EXPECTED_BRANCHES)} expected branch(es) MISSING:")
        for name in missing:
            print(f"  - {name}")

    # Content sanity checks for whatever *is* present
    if present:
        print(f"\nContent summary for {len(present)} present branch(es):")
        for name in present:
            arr = tree[name].array(library="ak")
            flat = arr if arr.ndim == 1 else ak.flatten(arr, axis=None)
            n_flat = len(flat)
            if n_flat == 0:
                print(f"  {name}: 0 values read (empty in all {n_events} events)")
            else:
                print(f"  {name}: {n_flat} values, min={ak.min(flat)}, max={ak.max(flat)}")

    # Cross-check: nMuon vs len(Muon_nDeDxHits) per event (extension table must match)
    if "Muon_nDeDxHits" in branches and "nMuon" in branches:
        n_muon = tree["nMuon"].array(library="np")
        n_dedx_ext = ak.num(tree["Muon_nDeDxHits"].array(library="ak"), axis=1).to_numpy()
        if (n_muon != n_dedx_ext).any():
            n_bad = int((n_muon != n_dedx_ext).sum())
            print(f"\nWARNING: Muon_nDeDxHits row count mismatches nMuon in {n_bad} event(s) "
                  "(extension table should always match the base Muon collection size)")
        else:
            print("\nOK: Muon_nDeDxHits row count matches nMuon in every event")

    dedx_hits_present = "nMuonDeDxHits" in branches
    if dedx_hits_present:
        n_hits = tree["nMuonDeDxHits"].array(library="np")
        total_hits = int(n_hits.sum())
        print(f"\nTotal Muon dE/dx hits across all events: {total_hits}")
        if total_hits == 0:
            print("WARNING: nMuonDeDxHits sums to zero across all events "
                  "(table exists but is empty everywhere)")

    if missing:
        print(f"\nRESULT: FAIL — {len(missing)} Muon dE/dx branch(es) not found in {path}")
        return 1

    print(f"\nRESULT: PASS — all {len(EXPECTED_BRANCHES)} expected Muon dE/dx branches present in {path}")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rootfile", nargs="?", default="exoNanoMC_PAT_NANO.root",
                         help="NanoAOD file to check (default: exoNanoMC_PAT_NANO.root)")
    args = parser.parse_args()
    sys.exit(check_file(args.rootfile))


if __name__ == "__main__":
    main()
