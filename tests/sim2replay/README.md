# sim2replay contract test

Two levels:

1. **`make_fake_T.C`** — synthetic contract test (no Geant4 needed): feeds a
   hand-built truth tree into the installed `prad2ana_sim2replay`.
2. **`run_e2e.sh`** — full end-to-end: runs real `prad2sim` prad2 jobs
   (elastic + møller at 3.5 GeV), converts with `sim2replay`, and validates
   the recon tree (entries, resolved `cl_center`, HyCal↔GEM matches).

Validates the **simulation → replayed-data** interface end to end, by feeding
truth trees into the real, installed `prad2ana_sim2replay` and checking it
emits a valid `recon` tree.

This is the contract prad2sim's ROOT output (`output/<name>_<run>.root`, tree
`T`) must satisfy. See [`../../plans/03-data-interface.md`](../../plans/03-data-interface.md).

## What `sim2replay` reads (the contract)

Tree **`T`**, branches:

| Branch | Type | Meaning |
|---|---|---|
| `VD.N` | `int` | virtual-detector hits (plane at the HyCal front) |
| `VD.X/Y/Z` | `double[VD.N]` | hit position, lab frame, mm |
| `VD.P` | `double[VD.N]` | momentum mag, used as cluster **energy** (MeV) |
| `GEM.N` | `int` | GEM hits |
| `GEM.DID` | `int[GEM.N]` | GEM detector id 0..3 |
| `GEM.X/Y/Z` | `double[GEM.N]` | entry position, mm |
| `GEM.Xout/Yout/Zout` | `double[GEM.N]` | **exit** position, mm (averaged with entry → hit) |
| `GEM.Edep` | `double[GEM.N]` | energy deposit |

`sim2replay` smears VD hits into HyCal clusters (forcing `cl_z = 6225` mm),
resolves `cl_center` against `hycal_map.json`, builds GEM hits from the
in/out average, runs `analysis::MatchingTools`, and writes the `recon` tree
documented in `prad2evviewer/docs/REPLAYED_DATA.md`.

## Run

```bash
cd tests/sim2replay
mkdir -p ep ee
root -b -q 'make_fake_T.C("ep/ep.root",1)'   # elastic-ep-like: 1 cluster/event
root -b -q 'make_fake_T.C("ee/ee.root",2)'   # Moller-ee-like:  2 clusters/event
prad2ana_sim2replay ep ee 1e6 1e6 -o sim_recon.root
root -b -q -e 'TFile f("sim_recon.root"); auto*t=(TTree*)f.Get("recon"); \
  printf("recon entries=%lld branches=%d\n",t->GetEntries(),t->GetListOfBranches()->GetSize()); \
  t->Scan("n_clusters:cl_energy:cl_center:n_gem_hits","","",5);'
```

`prad2ana_sim2replay` comes from the `prad2` toolkit (`~/Apps/prad2/bin`,
`source ~/Apps/prad2/bin/prad2_setup.sh`). It self-resolves its database from
`../share/prad2evviewer/database` (override with `PRAD2_DATABASE_DIR`).

## Expected

A `recon` tree (~`N_ep + N_ee` entries) with `cl_energy ≈ VD.P` (smeared),
`cl_center` = real PrimEx module IDs, and `n_gem_hits = 4 × n_clusters`.

## Caveats this test surfaces

- `sim2replay` hardcodes **`beamE = 3500` MeV**; its cuts (`VD.P > beamE/300`,
  `total > 0.5·beamE`) reject sub-GeV events. prad/drad at 1.1 GeV need this
  made configurable upstream.
- `cl_z`/`gem_z` are **overridden** by `sim2replay` constants — only `x,y`,
  `DID`, and `VD.P` from the simulation are used.
- Once prad2sim can run with Geant4, replace the synthetic generator with a
  real short `prad2sim` run and re-run the same `sim2replay` + check.
