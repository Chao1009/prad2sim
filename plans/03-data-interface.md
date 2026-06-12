# Data interface: matching `REPLAYED_DATA.md`

Reference: `../prad2evviewer/docs/REPLAYED_DATA.md`. Goal: simulated data is
consumed by the `prad2*` ecosystem (viewer, analysis) **identically to real
replayed data**.

## 0. Key finding — the bridge already exists (and is validated)

The `prad2` toolkit ships **`prad2ana_sim2replay`** (`~/Apps/prad2/bin`, source
`prad2evviewer/analysis/tools/sim2replay.cpp`). It reads simulation truth ROOT
files and writes the **`recon`** tree from `REPLAYED_DATA.md`:

```
sim2replay <dir of input_ep.root> <dir of input_ee.root> <ep_lumi nb^-1> <ee_lumi nb^-1> [-o out.root]
           → recon tree (default sim_recon.root)
```

It combines an elastic-ep and a Møller-ee sample by luminosity, smears truth
into HyCal clusters, builds GEM hits, runs the real `analysis::MatchingTools`,
and fills `prad2::ReconEventData`. **This is exactly the Phase-A "truth → recon"
step — already implemented.** prad2sim's job is therefore *not* to write a recon
tree itself, but to **emit truth files in the format `sim2replay` reads.**

**Validated end to end on 2026-06-12** (no Geant4 needed): a synthetic `T` tree
with the contract below was fed to the installed `sim2replay`, which produced a
`recon` tree (entries = N_ep+N_ee, 53 branches) with correct `cl_energy`,
`cl_center` (real PrimEx module IDs from `hycal_map.json`), and
`n_gem_hits = 4 × n_clusters`. Reproduce with [`../tests/sim2replay`](../tests/sim2replay/).

## 1. The contract prad2sim must satisfy

`sim2replay` opens a `TChain("T")` and `SetBranchAddress` on:

| Branch | Type | Role in sim2replay |
|---|---|---|
| `VD.N` | `int` | virtual-detector hits (a plane at the **HyCal front**) |
| `VD.X`, `VD.Y`, `VD.Z` | `double[VD.N]` | hit position, lab frame, mm |
| `VD.P` | `double[VD.N]` | momentum mag → used as cluster **energy** (MeV) |
| `GEM.N` | `int` | GEM hits |
| `GEM.DID` | `int[GEM.N]` | GEM detector id 0..3 |
| `GEM.X/Y/Z` | `double[GEM.N]` | entry position, mm |
| `GEM.Xout/Yout/Zout` | `double[GEM.N]` | **exit** position, mm (avg with entry → hit x,y) |
| `GEM.Edep` | `double[GEM.N]` | energy deposit (noise threshold) |

Only `VD.{X,Y,P}`, `GEM.{DID, X, Y, Xout, Yout, Edep}` actually drive the output;
`sim2replay` overrides z with its own `hycal_z = 6225` / `gem_z` constants.
Positions are HyCal/GEM-centered lab-frame mm (beam-aligned).

### Status in prad2sim (after this work)
- `StandardDetectorSD` now registers `.DID` and `.Xout/.Yout/.Zout`
  (`src/StandardDetectorSD.cc`, `include/StandardDetectorSD.hh`) — the branches
  `sim2replay` needs but the baseline lacked. **Needs a Geant4 build to compile.**
- **Still TODO** (Geant4 geometry work, can't be tested without G4):
  1. **A `VD` plane at the HyCal front**, enabled. Today `DefinePRadVolumes`
     puts a virtual detector 60 mm downstream of the target (a scattering check)
     and `sensitive_detectors.virtual` defaults to `false`. The data interface
     needs a virtual plane spanning the HyCal face at `crystal_surface`, with
     `VD` as its SD abbrev, recording `VD.X/Y/Z/P`.
  2. **GEM `DID` value 0..3 per plane.** The branch exists but its value comes
     from `StandardHit::GetDetectorID()`, which the tracking SD never sets (only
     `CalorimeterSD` does). Assign GEM copy numbers 0..3 in the geometry and set
     the hit's detector id from the copy number, so the 4-GEM matching works.

## 2. Two fidelity levels (decision: both, Phase A first)

### Phase A — `sim2replay` (truth-smeared recon). DONE on the prad2 side.
Wire prad2sim to emit `T` with a HyCal-front `VD` plane + GEM in/out + DID, then
run `sim2replay`. Remaining work is the two Geant4 geometry TODOs above.

### Phase B — full reconstruction via the `prad2det` library
For trigger/recon realism, link **`prad2det`** (static lib in
`prad2evviewer/prad2det/`, namespaces `fdec`/`gem`/`prad2`; survey in
`plans/`-referenced notes) and reconstruct from real detector energies:
- HyCal: feed per-module energies (`HC.ModuleEdep`, already produced by
  `CalorimeterSD`) into `fdec::HyCalCluster` (island algorithm) instead of using
  truth VD hits → realistic clusters, splitting, leakage.
- GEM: `gem::GemCluster::CartesianReconstruct` on digitized strips.
- Match with `analysis::MatchingTools`; write `prad2::ReconEventData` via
  `prad2::SetReconWriteBranches`.
- Optionally also write the raw **`events`** tree (`prad2::RawEventData`,
  FADC250 waveforms + GEM strips) for the real `prad2ana_replay_recon` path.

Build integration (confirmed direction — link, don't re-declare):
`add_subdirectory(prad2evviewer/prad2det)` + `target_link_libraries(prad2sim
PRIVATE prad2det)`, or link the installed `~/Apps/prad2/lib/libprad2det.a` with
`-I ~/Apps/prad2/include/prad2det`. `EventData.h` needs no ROOT; `EventData_io.h`
does.

## 3. ID encodings (already handled by `sim2replay`/`prad2det`)
PbGlass 1..1156, PbWO4 1001..2152, Veto 3001..3004, LMS 3100..3103;
`module_type` 0..4. `findModuleID()` resolves `cl_center` from `hycal_map.json`;
prad2sim just supplies lab-frame x,y. **The sim HyCal geometry must share the
`hycal_map.json` module positions** so truth x,y land on the right modules — the
one place the two sides must agree (validate the sim crystal layout against
`~/Apps/prad2/share/prad2evviewer/database/hycal_map.json`).

## 4. Caveats found in `sim2replay` (upstream, flag to collaborators)
- **`beamE` hardcoded to 3500 MeV**; cuts `VD.P > beamE/300` and
  `total > 0.5·beamE` reject sub-GeV events → prad/drad at 1.1 GeV need `beamE`
  configurable. (X17/PRad-II beam is multi-GeV, hence the constant.)
- Event-interleave guard `if (i + 1 % 4 == 0 ...)` parses as `i + ((1%4)==0)` =
  `i` — the intended "every 4th event is ep" mixing doesn't happen. Cosmetic for
  our purposes but worth a PR upstream.
- `GEM.DID` is read but several sim forks (e.g. X17) never wrote the branch, so
  their GEM hits collapse to det 0 — prad2sim should write a correct `.DID`.

## 5. E2E result (2026-06-12) and remaining items

**Validated** (`tests/sim2replay/run_e2e.sh`): prad2 config (elastic + møller,
3.5 GeV, 400+400 events) → installed `sim2replay` → recon tree with 673
entries, `total_energy` ≈ beam, **all `cl_center` resolved** (the prad2 sim
HyCal is generated from `hycal_map.json` via
`database/make_hycal_table_from_map.py`), 316 HyCal↔GEM matched pairs.
Matching is radius-limited (matched-pair mean r = 117 mm): `sim2replay`
labels clusters `cl_z = 6225` mm while the VD plane sits at 5907 mm, so the
GEM projection leaves the 10 mm window beyond r ≈ 200 mm.

**Upstream `sim2replay` suggestions (prad2evviewer):**
1. Use `sim->VD_z[j]` for `cl_z` (1 line) — removes the radial matching limit.
2. Make `beamE` (hardcoded 3500 MeV) configurable — blocks 1.1 GeV prad/drad.
3. Fix the ep/ee interleave `if (i + 1 % 4 == 0 ...)` precedence bug.

**Remaining unknowns (non-blocking):**
1. Does `prad2evviewer` require `scalers`/`epics`/`runinfo` trees to *exist* to
   open a file, or tolerate their absence? (sim files won't have real ones.)
2. For Phase B: whether `prad2ana_replay_recon` will consume a sim-produced
   `events` ROOT tree directly, or needs EVIO.
