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

## Phase 1 — structural refactor (no physics change)
- [ ] Extract `DefineMaterials()` → `MaterialFactory` (+ PRad2/X17 materials).
- [ ] Introduce the `DetectorModule` interface and split shared builders into
      `TargetModule`, `VacuumSystemModule`, `GEMModule`, `ScintillatorModule`,
      `HyCalModule`, `RecoilModule`. `DetectorConstruction` becomes a thin
      assembler driven by a config→module table.
- [ ] Unify `StandardHit` to carry in/out positions + momenta; collapse SD
      boilerplate into a shared base; evaluate vector branches vs fixed arrays.
- [ ] Remove the `gRootTree` global; pass the output writer through
      `ActionInitialization`.
- [ ] Regression: same config → same `T` tree contents as before the refactor
      (compare a fixed-seed run bit-for-bit where possible).

## Phase 2 — generators
- [ ] `GeneratorFactory` keyed off config; tidy file-gun readers.
- [ ] Document each `evgen/` program's inputs/outputs in `evgen/README`.

## Phase 3 — new configs
- [ ] `config/prad2.json` + module variants (3 mm GEM drift, 4-plane
      scintillator, CAD housing import, PRad-II HyCal map alignment).
- [ ] `config/x17.json` + X17 vacuum window/adapter (config-driven STL),
      beam-pipe shielding, X17→e+e- generator (event-file path first).
- [ ] Per-config smoke tests: builds, no overlaps, SDs attached, N events run,
      expected branches present (validation matrix in
      [02-configurations.md](02-configurations.md)).

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
- [ ] **Geant4 geometry (needs a G4 build):**
  - [ ] Add an enabled **`VD` plane at the HyCal front** (abbrev `VD`,
        `VD.X/Y/Z/P`); retire/rename the 60-mm scattering-check plane.
  - [ ] Assign GEM copy numbers 0..3 and set the hit detector id from them so
        `GEM.DID` is meaningful for 4-GEM matching.
- [ ] Verify the sim HyCal crystal layout matches `hycal_map.json` x,y so
      `cl_center` resolves correctly; run a short real `prad2sim` job through
      `sim2replay` and open the result in `prad2evviewer`.
- [ ] Get `beamE` (and its cuts) made configurable in `sim2replay` for the
      1.1 GeV prad/drad configs (upstream PR), or pre-scale.
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
