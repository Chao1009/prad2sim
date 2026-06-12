# Implementation roadmap

Ordered so each phase is independently buildable/testable and leaves the package
working. Phases 0–2 are pure refactors (no behaviour change); 3–4 add the new
configs and data interface.

## Phase 0 — toolchain & scaffolding
- [ ] Bump CMake to ≥3.16, target-based linking; replace `file(GLOB)` with an
      explicit source list.
- [ ] Guard Geant4 11.x / Qt6 API (e.g. `G4VisAttributes::GetInvisible()`); pick
      target G4/ROOT versions (confirm with user).
- [ ] Make EVIO/PRadAnalyzer digitization and `CADMesh` **optional** CMake
      components; core sim builds without them.
- [ ] Add a `plans/`-driven `CONTRIBUTING`/style note (naming, units, where
      constants live). Smoke-build all current configs as a baseline.

## Phase 1 — structural refactor (no physics change) — DONE 2026-06-12
- [x] Extract `DefineMaterials()` → `MaterialBuilder` (+ PRad2/X17 materials).
- [x] `DetectorModule` interface; geometry split into Target/Beamline/
      VacuumSystem/GEM/HeBag/Scintillator/HyCal/VirtualDet/CadInserts modules
      under `src/detector/`; `DetectorConstruction` is a thin per-config
      assembler. (Recoil lives inside `DRadTargetModule` — it is physically
      mounted in the target gas volume.)
- [x] Regression: fixed-seed prad/drad/test **bit-identical** before/after
      (`tests/regression/`, tree-stats diff). One deliberate change:
      `CalorimeterSD` now takes the crystal-surface z from the geometry
      instead of a hardcoded PRad-I constant — fixes the PbWO4 attenuation
      depth for drad (HC energies change there) and makes prad2/x17 possible.
- [x] In/out positions on `StandardHit` (session 1).
- [ ] Remove the `gRootTree` global; vector branches vs fixed arrays —
      deferred (cosmetic; no behavior impact).

## Phase 2 — generators
- [ ] `GeneratorFactory` keyed off config; tidy file-gun readers.
- [ ] Document each `evgen/` program's inputs/outputs in `evgen/README`.

## Phase 3 — new configs — DONE 2026-06-12 (except X17 signal generator)
- [x] `config/prad2.json`: two GEM stations (PRad-II window stack, 3 mm drift
      SD, DID 0..3 verified), 4-paddle scintillator + SciVD planes, CAD
      housing/window assembly (config-driven STL list), HyCal **generated
      from the prad2 `hycal_map.json`** (`database/make_hycal_table_from_map.py`
      — the legacy table had the lead-glass ring transposed vs the map).
- [x] `config/x17.json` (based on prad2.json): target −455 cm, GEMs at
      +6688/7088 mm, HyCal at +7506.464 mm, 12 m world, kX17 vacuum (He bag,
      thick/hole windows), beam-pipe shielding, X17 adapter STLs at zero
      offset, LH2/Ta target options.
- [x] Smoke tests: both run clean; overlap check shows only pre-existing
      legacy beam-hole slivers (present in prad too) and upstream CAD
      contact overlaps (µm–0.7 mm, inherited from the STL assembly).
- [ ] X17→e+e- signal generator (event-file path) — still open.

## Phase 4 — data interface (the headline goal)

**Reframed after discovery (2026-06-12):** `prad2ana_sim2replay` already turns
sim truth into the `recon` tree, and the contract is validated end to end (see
[03-data-interface.md](03-data-interface.md) + [`../tests/sim2replay`](../tests/sim2replay/)).
So Phase A is mostly about making prad2sim *emit the truth format* `sim2replay`
reads, not writing a recon writer.

- [x] Survey `prad2det` (clusterer + `EventData`/`EventData_io` structs) and the
      `sim2replay` contract; confirm `add_subdirectory(prad2evviewer/prad2det)` /
      installed `libprad2det.a` is the link target.
- [x] Validate the `T` → `sim2replay` → `recon` chain with a synthetic tree
      (`tests/sim2replay/`).
- [x] Add `.DID` + `.Xout/.Yout/.Zout` branches to `StandardDetectorSD`
      (the branches `sim2replay` needs). *(written; compile on a Geant4 box)*
- [x] **Geant4 geometry** — DONE 2026-06-12:
  - [x] Config-driven `VD` plane (`virtual_det` section): prad2/x17 place the
        annulus just behind the last GEM station (the PRadSim_PRad2
        convention its sim2replay was written against).
  - [x] GEM detector IDs 0..3 — already produced by the copy-number scheme
        once two PRad-II stations exist; verified in output.
- [x] Sim HyCal built from `hycal_map.json` for prad2/x17 → **all `cl_center`
      resolve (0 unresolved in E2E)**.
- [x] **E2E validated** (`tests/sim2replay/run_e2e.sh`): prad2 elastic+møller
      at 3.5 GeV → `sim2replay` → recon tree (673/800 entries, total_energy
      ≈ beam, 316 HyCal↔GEM matched pairs).
- [ ] Upstream `sim2replay` improvements (prad2evviewer repo):
  1. `beamE` hardcoded 3500 MeV → make configurable (blocks 1.1 GeV
     prad/drad use).
  2. Use `VD.Z` from the tree instead of forcing `cl_z = 6225` — removes the
     radial projection error that limits HyCal↔GEM matching to r ≲ 200 mm
     (matched-pair mean radius 117 mm in the E2E run).
  3. The ep/ee interleave condition `if (i + 1 % 4 == 0 ...)` mis-parses
     (operator precedence) and never mixes as intended.
- [ ] **Phase B** (fidelity): link `prad2det`, reconstruct from real
      `HC.ModuleEdep` via `fdec::HyCalCluster` + `gem::GemCluster`, write
      `recon` (and optionally raw `events`) via `prad2::SetReconWriteBranches`.
- [ ] Keep the old EVIO/PRadAnalyzer `digitization/` path until Phase B
      validates, then retire.

## Phase 5 — polish
- [ ] README + per-module docs; constants audit; CI smoke build if available.
- [ ] Optional: split `libprad2sim` core lib + thin `main` for reuse/tests.

## Cross-cutting checks
- Fixed-seed determinism for regression diffs.
- No silent truncation: if any cap (cluster/hit/track count) is hit, log it.
- Keep the truth `T` tree available behind `output.truth` for acceptance studies.

## Environment note (this machine) — updated 2026-06-12
ROOT 6.28, the `prad2` toolkit, **and now Geant4 11.4.0** (`~/Apps/geant4`, built
from source) are all installed. prad2sim **builds and runs** here — see
[05-build-notes.md](05-build-notes.md). So the whole pipeline is buildable and
testable locally; no more blind Geant4 edits.

## Done this session
- [x] Geant4 11.4.0 installed (`~/Apps/build_geant4.sh`).
- [x] prad2sim ported to G4 11.4 (`GetInvisible`, `std::hash`, `constexpr`) and
      **builds + runs** (500/200-event jobs, exit 0, `T` tree written).
- [x] Phase-0 CMake: min 3.16, optional `digitization` (`BUILD_DIGITIZATION`).
- [x] SD emits `GEM.Xout/Yout/Zout`; confirmed `GEM.DID` (from
      `TrackingDetectorSD`) is single + correct after reverting a duplicate.
- [x] `T` → `sim2replay` → `recon` contract proven (`tests/sim2replay`).

## Immediate next steps
1. **HyCal-front `VD` plane** (enabled) writing `VD.X/Y/Z/P`, sized/aligned to the
   HyCal face; assign GEM `DID` 0..3 for the PRad-II layout. Build+run loop is
   ~1 min now, so iterate with overlap checking.
2. Align the sim HyCal transverse frame to `hycal_map.json`; make `sim2replay`
   `beamE` configurable (or add a high-energy config) → run a real prad2sim job
   end to end through `sim2replay` and open in `prad2evviewer`.
3. Then the structural refactor (Phases 1–2: `DetectorModule`/`MaterialFactory`/
   SD unification) and the new `prad2`/`x17` configs (Phase 3), all now
   compile-testable locally.
